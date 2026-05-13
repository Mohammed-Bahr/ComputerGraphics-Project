/*
 * ============================================================================
 * Computer Graphics Term Project - Person 3 Implementation
 * Linux / GNOME Port using GTK3 + Cairo
 * ============================================================================
 *
 * This program is a complete computer graphics drawing application that
 * demonstrates multiple line, circle, ellipse, and clipping algorithms.
 * It is a Linux port of a Windows Win32 application (main.cpp).
 *
 * Key libraries used:
 *   - GTK3:  Cross-platform GUI toolkit (window, menu, events)
 *   - Cairo: 2D graphics library (pixel-level drawing)
 *
 * Implemented Algorithms:
 *   1. DDA Line             (Digital Differential Analyzer)
 *   2. Midpoint Line        (Bresenham's line algorithm)
 *   3. Modified Midpoint Circle (faster Bresenham variant using 2nd-order diffs)
 *   4. Direct Circle        (x² + y² = R², explicit equation)
 *   5. Bresenham Circle     (Midpoint circle algorithm)
 *   6. Polar Circle         (using sin/cos parametric form)
 *   7. Iterative Polar Circle (rotation matrix, no trig per pixel)
 *   8. Midpoint Ellipse     (4-way symmetry, two-region algorithm)
 *   9. Fill Circle with Lines (radial lines from center)
 *  10. Rectangular Point Clipping
 *  11. Rectangular Line Clipping   (Cohen-Sutherland)
 *  12. Rectangular Polygon Clipping (Sutherland-Hodgman)
 *  13. Circular Point Clipping
 *  14. Circular Line Clipping
 *
 * Build command:
 *   g++ LinuxMain.cpp -o cg_project $(pkg-config --cflags --libs gtk+-3.0) -lm -std=c++11
 *
 * Architecture overview:
 *   - ALL rendering happens inside the "draw" signal callback (on_draw).
 *   - Mouse clicks update application state and store Shape objects,
 *     then call gtk_widget_queue_draw() to trigger a full redraw.
 *   - No direct drawing ever happens outside on_draw.
 *
 * Mouse interaction per mode:
 *   - Lines / Circles (except ellipse): 2 clicks (start/end or center/radius)
 *   - Ellipse: 3 clicks (center, b-radius, a-radius)
 *   - Fill / Point Clip / Circle Point Clip: 1 click on existing shape
 *   - Polygon Clip: left-clicks to add vertices, right-click to finalize
 *   - Circle Line Clip: 2 clicks (second must be inside an existing circle)
 * ============================================================================
 */

// ============================================================================
// INCLUDES
// ============================================================================
#include <gtk/gtk.h>      // GTK3: widgets, window, menus, events, Cairo context
#include <cmath>          // sqrt, pow, sin, cos, atan2, round, abs
#include <vector>         // std::vector: dynamic array for storing shapes/points
#include <iostream>       // std::cout: console output for debugging / user feedback
#include <algorithm>      // std::max, std::min
#include <cstdio>         // std::fopen, fwrite, fread, fclose for save/load

using namespace std;

// ============================================================================
// STRUCT: Color
// PURPOSE:  Stores an RGB color in Cairo's 0.0–1.0 floating-point range.
// NOTE:     Windows uses COLORREF (0–255 range). Cairo uses 0.0–1.0 doubles.
// ============================================================================
struct Color {
    double r, g, b;  // Red, Green, Blue components (each 0.0 = none, 1.0 = full)
};

// ============================================================================
// STRUCT: Point2D
// PURPOSE:  Replaces Windows' POINT type (used for polygon clipping).
// NOTE:     Windows POINT has LONG x,y; we use int for simplicity.
// ============================================================================
struct Point2D {
    int x, y;
    // Constructor with defaults (0,0) so we can write Point2D() or Point2D(a,b)
    Point2D(int a = 0, int b = 0) : x(a), y(b) {}
};

// ============================================================================
// ENUM: DrawMode
// PURPOSE:  Identifies which drawing/clipping algorithm the user selected.
//           Each mode corresponds to a menu item and determines how mouse
//           clicks are interpreted and which algorithm renders the shape.
// ============================================================================
enum DrawMode {
    MODE_NONE,                      // No mode selected — ignore clicks
    // --- Line algorithms ---
    MODE_DDA_LINE,                  // Digital Differential Analyzer
    MODE_MIDPOINT_LINE,             // Bresenham's integer midpoint line
    // --- Circle algorithms ---
    MODE_MODIFIED_MIDPOINT_CIRCLE,  // Faster Bresenham (2nd-order differences)
    MODE_DIRECT_CIRCLE,             // Direct from equation x²+y²=R²
    MODE_BRESENHAM_CIRCLE,          // Standard midpoint/Bresenham circle
    MODE_POLAR_CIRCLE,              // Parametric: x=R·cosθ, y=R·sinθ
    MODE_ITERATIVE_POLAR_CIRCLE,    // Rotation matrix (no trig per pixel)
    // --- Ellipse algorithm ---
    MODE_MIDPOINT_ELLIPSE,          // Midpoint ellipse (4-way symmetry)
    // --- Fill algorithm ---
    MODE_FILL_CIRCLE_WITH_LINES,    // Fill a circle quadrant with radial lines
    // --- Clipping algorithms ---
    MODE_CLIP_POINT,                // Test if a point is inside rectangular clip window
    MODE_CLIP_LINE,                 // Cohen-Sutherland line clipping against rectangle
    MODE_CLIP_POLYGON,              // Sutherland-Hodgman polygon clipping against rectangle
    MODE_CLIP_CIRCLE_POINT,         // Test if a point is inside a drawn circle
    MODE_CLIP_CIRCLE_LINE,          // Clip a line to the boundary of a circle
};

// ============================================================================
// STRUCT: Shape
// PURPOSE:  Stores one drawn shape so it can be redrawn whenever the window
//           needs refreshing (resize, expose, clear+redraw).
// NOTE:     The meaning of (x1,y1)-(x2,y2) varies by mode:
//           - Lines:    (x1,y1) = start, (x2,y2) = end
//           - Circles:  (x1,y1) = center, (x2,y2) = point on circumference
//           - Ellipse:  (x1,y1) = center, (x2,y2) = used to derive radii
//           - Clipping: stores the point(s) or click location
// ============================================================================
struct Shape {
    DrawMode mode;    // Which algorithm to use when rendering
    Color    color;   // Color this shape was drawn with
    int x1, y1;       // First coordinate pair (see note above)
    int x2, y2;       // Second coordinate pair (see note above)
};

// ============================================================================
// GLOBAL STATE
// ============================================================================
// Why globals? This is a single-window application ported from Windows.
// The original Win32 code used globals extensively, and GTK signal callbacks
// have a fixed signature with limited user-data. Using globals keeps the
// port simple and avoids wrapping everything in a class.

Color    g_backgroundColor = {0.0, 0.0, 0.0};   // Default background: black
Color    g_drawColor       = {1.0, 1.0, 1.0};   // Default drawing color: white
DrawMode g_currentMode     = MODE_NONE;          // Currently active drawing mode

// Multi-click state machine
// Some modes need 2 clicks (lines, circles), ellipse needs 3 clicks,
// other modes are single-click.
bool g_firstClick  = false;  // true after first click in a multi-click sequence
bool g_secondClick = false;  // true after second click (ellipse mode uses 3 clicks)
int  g_startX = 0, g_startY = 0;  // Coordinates from the first click
int  g_X1 = 0, g_Y1 = 0;         // Coordinates from the second click (ellipse)

// Shape storage (retained-mode rendering)
vector<Shape>   g_shapes;           // All drawn shapes — redrawn every frame
vector<Point2D> g_polygonPoints;    // Polygon vertices being collected (left-clicks)
vector<Point2D> g_polygonResult;    // Clipped polygon result (after Sutherland-Hodgman)
bool            g_polygonClipped = false;  // true once polygon is finalized via right-click

// Rectangular clipping window boundaries
// These define a yellow dashed rectangle used by MODE_CLIP_POINT, _LINE, _POLYGON.
int g_clipX1 = 150, g_clipY1 = 150;   // Top-left corner of clip window
int g_clipX2 = 600, g_clipY2 = 400;   // Bottom-right corner of clip window

// GTK widget pointers (needed globally for menu callbacks to trigger redraws)
GtkWidget* g_drawingArea = nullptr;  // The GtkDrawingArea where we render pixels
GtkWidget* g_mainWindow  = nullptr;  // The top-level application window

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Round: Rounds a double to the nearest integer.
 * Used throughout drawing algorithms to convert floating-point
 * coordinates to pixel positions.
 *
 * Math: (int)(x + 0.5) rounds to nearest integer.
 *   Example: Round(3.2) = 3, Round(3.7) = 4, Round(-0.3) = 0
 */
static inline int Round(double x) {
    return (int)(x + 0.5);
}

/**
 * swap_points: Swaps two coordinate pairs.
 * Used by DDA line to ensure we always draw from left-to-right
 * or top-to-bottom.
 *
 * Before: a,b = (x1,y1), c,d = (x2,y2)
 * After:  a,b = (x2,y2), c,d = (x1,y1)
 */
static void swap_points(int& a, int& b, int& c, int& d) {
    int t;
    t = a; a = c; c = t;  // Swap x1 ↔ x2
    t = b; b = d; d = t;  // Swap y1 ↔ y2
}

/**
 * SetPixel: Draws a single pixel on the Cairo surface.
 *
 * Cairo doesn't have a "set pixel" primitive, so we simulate one
 * by filling a 1×1 rectangle at the given position.
 *
 * Steps:
 *   1. Set the drawing color using cairo_set_source_rgb
 *   2. Create a 1×1 rectangle path at (x, y)
 *   3. Fill that rectangle with the current source color
 *
 * @param cr    Cairo context (the drawing surface)
 * @param x, y  Pixel coordinates
 * @param color Color to draw with
 */
static void SetPixel(cairo_t* cr, int x, int y, Color color) {
    cairo_set_source_rgb(cr, color.r, color.g, color.b);  // Set pen color
    cairo_rectangle(cr, x, y, 1.0, 1.0);                   // 1×1 pixel path
    cairo_fill(cr);                                         // Fill it
}

/**
 * CheckIFPointINCircle: Tests whether a point (x,y) lies inside
 * or on the circumference of a circle.
 *
 * Math: A point is inside a circle if its distance from the center
 * is ≤ the radius.
 *   distance = √[(x - xc)² + (y - yc)²]
 *   inside ↔ distance ≤ R
 *
 * @param R       Circle radius
 * @param xc, yc  Circle center
 * @param x, y    Point to test
 * @return true if the point is inside/on the circle
 */
static bool CheckIFPointINCircle(int R, int xc, int yc, int x, int y) {
    return sqrt(pow((double)(x - xc), 2) + pow((double)(y - yc), 2)) <= R;
}

/**
 * FindCircle: Searches all stored shapes for a circle that contains
 * the given point (x,y). Used by:
 *   - Fill Circle with Lines (click on a circle quadrant)
 *   - Circle Point Clipping   (click on a circle)
 *   - Circle Line Clipping    (second click on a circle)
 *
 * @param x, y   Click coordinates
 * @param xc, yc [out] Center of the found circle
 * @param R      [out] Radius of the found circle
 * @return true if a containing circle was found
 */
static bool FindCircle(int x, int y, int& xc, int& yc, int& R) {
    // Iterate through all stored shapes looking for circle types
    for (auto& s : g_shapes) {
        if (s.mode == MODE_DIRECT_CIRCLE ||
            s.mode == MODE_BRESENHAM_CIRCLE ||
            s.mode == MODE_MODIFIED_MIDPOINT_CIRCLE ||
            s.mode == MODE_POLAR_CIRCLE ||
            s.mode == MODE_ITERATIVE_POLAR_CIRCLE)
        {
            // Calculate radius: distance from center to the second click point
            // For circles, (x1,y1) is center, (x2,y2) is a point on circumference
            int dx = s.x2 - s.x1, dy = s.y2 - s.y1;
            R = (int)sqrt(dx * dx + dy * dy);
            xc = s.x1; yc = s.y1;

            if (CheckIFPointINCircle(R, xc, yc, x, y))
                return true;
        }
    }
    return false;
}

// ============================================================================
// SYMMETRY DRAWING FUNCTIONS
// ============================================================================
// Circle and ellipse algorithms take advantage of symmetry to reduce
// computation. Instead of computing every pixel on the full circumference,
// we compute pixels in one octant (1/8th of the circle) and reflect them.

/**
 * Draw8Points: Plots 8 symmetric points of a circle.
 *
 * Circle symmetry: A circle centered at (xc,yc) has 8-fold symmetry.
 * If (x, y) is on the circle, then all 8 sign combinations
 * (±x, ±y) and (±y, ±x) are also on the circle.
 *
 * Octants (numbered counterclockwise from 3 o'clock):
 *   Octant 0: ( x,  y)  →  0° to 45°
 *   Octant 1: ( y,  x)  → 45° to 90°
 *   Octant 2: (-y,  x)  → 90° to 135°
 *   Octant 3: (-x,  y)  → 135° to 180°
 *   Octant 4: (-x, -y)  → 180° to 225°
 *   Octant 5: (-y, -x)  → 225° to 270°
 *   Octant 6: ( y, -x)  → 270° to 315°
 *   Octant 7: ( x, -y)  → 315° to 360°
 *
 * @param cr        Cairo context
 * @param xc, yc    Circle center
 * @param x, y      Point in first octant
 * @param color     Drawing color
 */
static void Draw8Points(cairo_t* cr, int xc, int yc, int x, int y, Color color) {
    // Quadrant I   (x > 0, y > 0)
    SetPixel(cr, xc + x, yc + y, color);   // Octant 0: ( x,  y)
    SetPixel(cr, xc + y, yc + x, color);   // Octant 1: ( y,  x)

    // Quadrant II  (x < 0, y > 0)
    SetPixel(cr, xc - x, yc + y, color);   // Octant 3: (-x,  y)
    SetPixel(cr, xc - y, yc + x, color);   // Octant 2: (-y,  x)

    // Quadrant III (x < 0, y < 0)
    SetPixel(cr, xc - x, yc - y, color);   // Octant 4: (-x, -y)
    SetPixel(cr, xc - y, yc - x, color);   // Octant 5: (-y, -x)

    // Quadrant IV  (x > 0, y < 0)
    SetPixel(cr, xc + x, yc - y, color);   // Octant 7: ( x, -y)
    SetPixel(cr, xc + y, yc - x, color);   // Octant 6: ( y, -x)
}

/**
 * Draw4Points: Plots 4 symmetric points of an ellipse.
 *
 * Ellipse symmetry: An axis-aligned ellipse centered at (xc,yc)
 * has 4-fold symmetry. If (x, y) is on the ellipse, then all 4
 * sign combinations (±x, ±y) are also on the ellipse.
 *
 * (Unlike circles, ellipses are NOT 8-way symmetric because
 *  swapping x and y changes the ellipse unless a = b, i.e. it's a circle.)
 *
 * @param cr        Cairo context
 * @param xc, yc    Ellipse center
 * @param x, y      Point in first quadrant
 * @param color     Drawing color
 */
static void Draw4Points(cairo_t* cr, int xc, int yc, int x, int y, Color color) {
    SetPixel(cr, xc + x, yc + y, color);   // Quadrant I:  (+x, +y)
    SetPixel(cr, xc - x, yc + y, color);   // Quadrant II: (-x, +y)
    SetPixel(cr, xc + x, yc - y, color);   // Quadrant IV: (+x, -y)
    SetPixel(cr, xc - x, yc - y, color);   // Quadrant III:(-x, -y)
}

// ============================================================================
// ALGORITHM 1: DDA LINE (Digital Differential Analyzer)
// ============================================================================
/**
 * The DDA algorithm rasterizes a line by stepping along the dominant axis
 * (the axis with the larger range) and computing the corresponding position
 * on the other axis using floating-point arithmetic.
 *
 * How it works:
 *   1. Compute dx = x2 - x1, dy = y2 - y1
 *   2. If |dy| ≤ |dx| (gentle slope, |m| ≤ 1):
 *        - Step along X axis (x increments by 1 each iteration)
 *        - Compute y = y + m where m = dy/dx
 *        - Round y to nearest integer for pixel position
 *   3. If |dy| > |dx| (steep slope, |m| > 1):
 *        - Step along Y axis (y increments by 1)
 *        - Compute x = x + 1/m
 *        - Round x to nearest integer
 *   4. Always draw from left-to-right or top-to-bottom to avoid gaps
 *
 * Pros: Simple, handles all slopes
 * Cons: Uses floating-point arithmetic (slower than integer-only methods)
 *
 * @param cr        Cairo context
 * @param x1, y1    Start point
 * @param x2, y2    End point
 * @param c         Drawing color
 */
static void DrawLineDDA(cairo_t* cr,
                        int x1, int y1, int x2, int y2, Color c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Check if the line is more horizontal than vertical
    if (abs(dy) <= abs(dx)) {
        // --- Gentle slope case (|slope| ≤ 1) ---
        // Step along the X axis
        double m = (dx != 0) ? (double)dy / dx : 0.0;  // Slope

        // Ensure we draw left-to-right
        if (x1 > x2) swap_points(x1, y1, x2, y2);

        SetPixel(cr, x1, y1, c);  // Draw first pixel

        int    x = x1;
        double y = y1;

        while (x < x2) {
            x++;           // Step X by 1
            y += m;        // Advance Y by slope
            SetPixel(cr, x, Round(y), c);  // Round Y to nearest pixel
        }
    } else {
        // --- Steep slope case (|slope| > 1) ---
        // Step along the Y axis
        double mi = (dy != 0) ? (double)dx / dy : 0.0;  // Inverse slope (1/m)

        // Ensure we draw top-to-bottom
        if (y1 > y2) swap_points(x1, y1, x2, y2);

        SetPixel(cr, x1, y1, c);  // Draw first pixel

        int    y = y1;
        double x = x1;

        while (y < y2) {
            y++;           // Step Y by 1
            x += mi;       // Advance X by 1/m
            SetPixel(cr, Round(x), y, c);  // Round X to nearest pixel
        }
    }
}

// ============================================================================
// ALGORITHM 2: MIDPOINT LINE (Bresenham's Line Algorithm)
// ============================================================================
/**
 * Bresenham's line algorithm rasterizes a line using ONLY integer arithmetic.
 * It's faster than DDA because there are no floating-point operations.
 *
 * Core idea:
 *   At each step, we have a "decision parameter" d that tells us whether
 *   the midpoint between two candidate pixels is above or below the line.
 *   Based on d, we choose the pixel closer to the true line.
 *
 * For gentle slopes (|m| ≤ 1):
 *   - We always increment X
 *   - We decide whether to increment Y based on d
 *   - Decision parameter d starts at: 2*dy - dx (for standard Bresenham)
 *   - If d < 0: midpoint is below the line → choose E (no Y change)
 *   - If d ≥ 0: midpoint is above the line → choose NE (increment Y)
 *
 * For steep slopes (|m| > 1):
 *   - Same logic but roles of X and Y are swapped
 *
 * The code here uses an alternate formulation (dx - 2*dy) which is
 * mathematically equivalent but avoids some initial computations.
 *
 * @param cr        Cairo context
 * @param x1, y1    Start point
 * @param x2, y2    End point
 * @param c         Drawing color
 */
static void DrawLineMidpoint(cairo_t* cr,
                             int x1, int y1, int x2, int y2, Color c) {
    int dx = abs(x2 - x1);   // Total X distance (always positive)
    int dy = abs(y2 - y1);   // Total Y distance (always positive)

    // Direction of step: 1 = increasing, -1 = decreasing
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int x = x1, y = y1;     // Current pixel

    if (dx >= dy) {
        // --- Gentle slope: step along X axis ---
        // d = dx - 2*dy  (decision parameter, alternate formulation)
        // delta1 = 2*dx - 2*dy  (change when d < 0 → move diagonally)
        // delta2 = -2*dy         (change when d ≥ 0 → move horizontally)
        int d       = dx - 2 * dy;
        int delta1  = 2 * dx - 2 * dy;
        int delta2  = -2 * dy;

        SetPixel(cr, x, y, c);  // Plot first pixel

        while (x != x2) {
            if (d < 0) {
                // Midpoint below the line: choose diagonal step
                y += sy;
                d += delta1;
            } else {
                // Midpoint above the line: choose horizontal step
                d += delta2;
            }
            x += sx;                    // Always step in X direction
            SetPixel(cr, x, y, c);      // Plot the chosen pixel
        }
    } else {
        // --- Steep slope: step along Y axis ---
        // Same logic but X and Y roles swapped
        int d       = dy - 2 * dx;
        int delta1  = 2 * dy - 2 * dx;
        int delta2  = -2 * dx;

        SetPixel(cr, x, y, c);

        while (y != y2) {
            if (d < 0) {
                x += sx;
                d += delta1;
            } else {
                d += delta2;
            }
            y += sy;
            SetPixel(cr, x, y, c);
        }
    }
}

// ============================================================================
// ALGORITHM 3: MODIFIED MIDPOINT CIRCLE (Faster Bresenham Variant)
// ============================================================================
/**
 * This is an optimized version of the standard midpoint circle algorithm
 * that uses second-order differences (c1, c2) to improve performance.
 *
 * Standard midpoint circle uses: d = 1 - R, then updates d by 2x+3 or 2(x-y)+5
 * This variant pre-computes the increments (c1, c2) and updates them
 * incrementally, avoiding the per-pixel multiplication.
 *
 * Key optimization:
 *   c1 = 2*x + 3  (increment when choosing E)
 *   c2 = 2*(x-y) + 5  (increment when choosing SE)
 *   Both c1 and c2 are updated by simple addition each iteration
 *   instead of recomputing 2*x+3 or 2*(x-y)+5 from scratch.
 *
 * @param cr        Cairo context
 * @param xc, yc    Circle center
 * @param R         Circle radius
 * @param color     Drawing color
 */
static void DrawCircleModifiedMidpoint(cairo_t* cr,
                                       int xc, int yc, int R, Color color) {
    if (R <= 0) return;  // Invalid radius

    int x  = 0, y = R;      // Start at top of circle (0, R)
    int d  = 1 - R;          // Initial decision parameter
    int c1 = 3;              // Initial value of (2*x + 3)
    int c2 = 5 - 2 * R;      // Initial value of (2*(x-y) + 5)

    Draw8Points(cr, xc, yc, x, y, color);  // Plot initial 8 points

    while (x < y) {
        if (d < 0) {
            // Midpoint inside the circle → move RIGHT (E)
            d  += c1;
            c2 += 2;          // c2 increases by 2 when y doesn't change
        } else {
            // Midpoint outside the circle → move RIGHT and DOWN (SE)
            d  += c2;
            c2 += 4;          // c2 increases by 4 when y decrements
            y--;              // Move down
        }
        c1 += 2;              // c1 always increases by 2 per step
        x++;                  // Always move right
        Draw8Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 4: DIRECT CIRCLE (Explicit Equation)
// ============================================================================
/**
 * The direct method computes circle points using the explicit equation:
 *   y = ±√(R² - x²)
 *
 * We only need to iterate x from 0 to R/√2 (45° point, where x = y).
 * Beyond that, the slope exceeds 1 and stepping by x would leave gaps,
 * but since we use Draw8Points which swaps x and y for octants 1,3,5,7,
 * all points are covered.
 *
 * Pros: Conceptually simple
 * Cons: Uses sqrt() per pixel (slow), floating-point rounding errors
 *
 * @param cr        Cairo context
 * @param xc, yc    Circle center
 * @param R         Circle radius
 * @param color     Drawing color
 */
static void DrawCircleDirect(cairo_t* cr,
                             int xc, int yc, int R, Color color) {
    if (R <= 0) return;

    // Limit x to R/√2 ≈ R*0.707 (the 45° point where x = y)
    int limit = (int)(R / sqrt(2.0)) + 1;

    for (int x = 0; x <= limit; x++) {
        // Compute y = √(R² - x²)  (explicit circle equation)
        int y = (int)round(sqrt((double)R * R - (double)x * x));
        Draw8Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 5: BRESENHAM CIRCLE (Standard Midpoint Circle Algorithm)
// ============================================================================
/**
 * The standard midpoint circle algorithm uses ONLY integer arithmetic.
 *
 * Decision parameter: d = 1 - R
 *   - Starts at the top of the circle (0, R)
 *   - At each step, we test the midpoint between moving RIGHT (E)
 *     vs moving RIGHT+DOWN (SE)
 *
 * Update rules:
 *   If d < 0 (midpoint inside): d += 2*x + 3    (choose E)
 *   If d ≥ 0 (midpoint outside): d += 2*(x-y) + 5, y--  (choose SE)
 *   Always: x++
 *
 * This is simpler than the "Modified Midpoint" variant but computes
 * 2*x+3 and 2*(x-y)+5 from scratch each iteration (slightly slower).
 *
 * @param cr        Cairo context
 * @param xc, yc    Circle center
 * @param R         Circle radius
 * @param color     Drawing color
 */
static void DrawCircleBresenham(cairo_t* cr,
                                int xc, int yc, int R, Color color) {
    if (R <= 0) return;

    int x = 0, y = R;        // Start at (0, R) — top of circle
    int d = 1 - R;            // Initial decision parameter

    Draw8Points(cr, xc, yc, x, y, color);

    while (x < y) {
        if (d < 0) {
            // Midpoint inside: move RIGHT
            d += 2 * x + 3;
        } else {
            // Midpoint outside: move RIGHT and DOWN
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        Draw8Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 6: POLAR CIRCLE (Trigonometric Parametric Form)
// ============================================================================
/**
 * Uses the parametric equations of a circle:
 *   x(θ) = R · cos(θ)
 *   y(θ) = R · sin(θ)
 *
 * where θ goes from 0 to π/4 (45°). We use a small angular step
 * dθ = 1/R (empirically chosen for good pixel coverage).
 *
 * At each step:
 *   θ += dθ
 *   x = round(R · cos(θ))
 *   y = round(R · sin(θ))
 *
 * The Draw8Points call handles mirroring to all 8 octants.
 *
 * Pros: Conceptually clean, uses familiar sine/cosine
 * Cons: Expensive — calls cos() and sin() every pixel
 *
 * @param cr        Cairo context
 * @param xc, yc    Circle center
 * @param R         Circle radius
 * @param color     Drawing color
 */
static void DrawCirclePolar(cairo_t* cr,
                            int xc, int yc, int R, Color color) {
    if (R <= 0) return;

    int x = R, y = 0;              // Start at 0° (rightmost point)
    double theta = 0;              // Current angle
    double dtheta = 1.0 / R;       // Angular step size

    Draw8Points(cr, xc, yc, x, y, color);

    while (x > y) {
        theta += dtheta;                              // Increment angle
        x = Round(R * cos(theta));                    // x = R·cos(θ)
        y = Round(R * sin(theta));                    // y = R·sin(θ)
        Draw8Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 7: ITERATIVE POLAR CIRCLE (Rotation Matrix Method)
// ============================================================================
/**
 * Improves on the polar method by avoiding trigonometric function calls
 * (cos/sin) for every pixel. Instead, we use a rotation matrix to rotate
 * the previous point by dθ:
 *
 *   [x₁]   [ cos(dθ)  -sin(dθ) ] [x₀]
 *   [y₁] = [ sin(dθ)   cos(dθ) ] [y₀]
 *
 * Which expands to:
 *   x₁ = x₀ · cos(dθ) - y₀ · sin(dθ)
 *   y₁ = x₀ · sin(dθ) + y₀ · cos(dθ)
 *
 * cos(dθ) and sin(dθ) are computed ONCE at the start, then each step
 * is just 4 multiplications and 2 additions — much faster than cos/sin.
 *
 * @param cr        Cairo context
 * @param xc, yc    Circle center
 * @param R         Circle radius
 * @param color     Drawing color
 */
static void DrawCircleIterativePolar(cairo_t* cr,
                                     int xc, int yc, int R, Color color) {
    if (R <= 0) return;

    double x = R, y = 0;                     // Start at 0° (rightmost point)
    double dtheta = 1.0 / R;                 // Angular step
    double cdtheta = cos(dtheta);            // cos(Δθ) — precomputed once
    double sdtheta = sin(dtheta);            // sin(Δθ) — precomputed once

    Draw8Points(cr, xc, yc, (int)x, (int)y, color);

    while (x > y) {
        // Apply rotation matrix to the previous point
        double x1 = x * cdtheta - y * sdtheta;
        y = x * sdtheta + y * cdtheta;
        x = x1;

        Draw8Points(cr, xc, yc, Round(x), Round(y), color);
    }
}

// ============================================================================
// ALGORITHM 8: MIDPOINT ELLIPSE
// ============================================================================
/**
 * The midpoint ellipse algorithm is similar to the midpoint circle algorithm,
 * but uses 4-way symmetry instead of 8-way (since ellipses aren't 8-way
 * symmetric unless a = b).
 *
 * The ellipse equation:  b²x² + a²y² = a²b²
 *   where a = semi-major axis (X radius)
 *         b = semi-minor axis (Y radius)
 *
 * The algorithm splits the ellipse into TWO regions:
 *
 * Region 1:  slope < 1  (dx < dy)
 *   - Step along X axis
 *   - Decision parameter d₁ starts at:  b² - a²b + a²/4
 *   - Update based on whether midpoint is inside or outside
 *
 * Region 2:  slope > 1  (dx > dy)
 *   - Step along Y axis
 *   - Decision parameter d₂ starts at:
 *       b²(x + 0.5)² + a²(y - 1)² - a²b²
 *   - Update based on whether midpoint is inside or outside
 *
 * @param cr        Cairo context
 * @param xc, yc    Ellipse center
 * @param a         Semi-major axis (X radius)
 * @param b         Semi-minor axis (Y radius)
 * @param color     Drawing color
 */
static void DrawEllipseMidpoint(cairo_t* cr,
                                int xc, int yc, int a, int b, Color color) {
    if (a <= 0 || b <= 0) return;

    int x = 0, y = b;                            // Start at top of ellipse
    double d = 0.25 * a * a + b * b - a * a * b; // Initial decision param (Region 1)
    double dx = 2.0 * b * b * x;                 // 2·b²·x  (gradient in X)
    double dy = 2.0 * a * a * y;                 // 2·a²·y  (gradient in Y)

    Draw4Points(cr, xc, yc, x, y, color);

    // ---- Region 1: steep slope (step in X) ----
    while (dx < dy) {
        x++;
        dx += 2 * b * b;                         // Update gradient in X
        if (d < 0) {
            d += dx + b * b;                     // Midpoint inside: move E
        } else {
            dy -= 2 * a * a;                     // Update gradient in Y (decreasing)
            d += dx - dy + b * b;                // Midpoint outside: move SE
            y--;
        }
        Draw4Points(cr, xc, yc, x, y, color);
    }

    // ---- Region 2: gentle slope (step in Y) ----
    d = (double)b * b * (x + 0.5) * (x + 0.5)    // Recompute d for Region 2
      + (double)a * a * (y - 1) * (y - 1)
      - (double)a * a * b * b;
    Draw4Points(cr, xc, yc, x, y, color);

    while (y >= 0) {
        y--;
        dy -= 2 * a * a;                         // Update gradient in Y
        if (d > 0) {
            d += -dy + a * a;                    // Midpoint outside: move S
        } else {
            dx += 2 * b * b;                     // Update gradient in X
            d += dx - dy + a * a;                // Midpoint inside: move SE
            x++;
        }
        Draw4Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 9: FILL CIRCLE WITH LINES
// ============================================================================
/**
 * Fills a quadrant (quarter) of a circle with radial lines from center
 * to points along the circumference arc.
 *
 * Algorithm:
 *   1. The user clicks on a quadrant of an existing circle
 *   2. We determine which quadrant (1st, 2nd, 3rd, 4th) based on the
 *      angle from the circle center to the click point
 *   3. We draw lines from the center to every point on the arc spanning
 *      that quadrant, using the Midpoint Line algorithm
 *
 * Optimization: Instead of calling cos/sin for every line endpoint,
 * we use a rotation matrix (similar to Iterative Polar) to step along
 * the arc incrementally.
 *
 * @param cr            Cairo context
 * @param xc, yc        Circle center
 * @param R             Circle radius
 * @param thetaS        Start angle (radians)
 * @param thetaE        End angle (radians)
 * @param color         Drawing color
 */
static void FillCircleWithLines(cairo_t* cr, int xc, int yc, int R,
                                double thetaS, double thetaE, Color color) {
    if (R <= 0) return;

    // Angular step size (optimal for pixel coverage)
    double dtheta = 1.0 / R;

    // Precompute cos(Δθ) and sin(Δθ) for the rotation matrix
    double ct = cos(dtheta), st = sin(dtheta);

    // Initial point on the circle at angle thetaS
    double x_rel = R * cos(thetaS);
    double y_rel = R * sin(thetaS);

    // Number of radial lines to draw
    int steps = (int)((thetaE - thetaS) / dtheta);

    for (int i = 0; i <= steps; i++) {
        // Draw a line from center to the current point on the arc
        DrawLineMidpoint(cr, xc, yc,
                         xc + Round(x_rel), yc + Round(y_rel), color);

        // Rotate (x_rel, y_rel) by dθ using rotation matrix
        double next_x = x_rel * ct - y_rel * st;
        y_rel = x_rel * st + y_rel * ct;
        x_rel = next_x;
    }
}

// ============================================================================
// CLIPPING: CLIP WINDOW
// ============================================================================
/**
 * DrawClipWindow: Renders the rectangular clipping window as a yellow
 * dashed rectangle. This gives the user a visual reference for where
 * the clipping boundaries are.
 *
 * The clip window defaults to (150,150)–(600,400) and is used by:
 *   - Point Clipping
 *   - Line Clipping (Cohen-Sutherland)
 *   - Polygon Clipping (Sutherland-Hodgman)
 */
static void DrawClipWindow(cairo_t* cr) {
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);          // Yellow color
    const double dashes[] = {4.0, 4.0};                // Dash pattern: 4px on, 4px off
    cairo_set_dash(cr, dashes, 2, 0.0);                // Enable dashed lines
    cairo_set_line_width(cr, 1.0);                     // 1px wide
    cairo_rectangle(cr,
                    g_clipX1, g_clipY1,                         // Top-left
                    g_clipX2 - g_clipX1, g_clipY2 - g_clipY1);  // Width × Height
    cairo_stroke(cr);                                   // Draw the rectangle
    cairo_set_dash(cr, nullptr, 0, 0.0);                // Reset dash (solid lines again)
}

// ============================================================================
// CLIPPING ALGORITHM 10: RECTANGULAR POINT CLIPPING
// ============================================================================
/**
 * PointClipping: Tests whether a point lies inside the rectangular clip window.
 * If inside, draws a small 5×5 cross to mark it. If outside, does nothing.
 *
 * Test:  xl ≤ x ≤ xr  AND  yt ≤ y ≤ yb
 *
 * @param cr        Cairo context
 * @param x, y      Point to test
 * @param xl, yt    Top-left corner of clip window
 * @param xr, yb    Bottom-right corner of clip window
 * @param color     Drawing color
 */
static void PointClipping(cairo_t* cr, int x, int y,
                          int xl, int yt, int xr, int yb, Color color) {
    if (x >= xl && x <= xr && y >= yt && y <= yb) {
        // Point is inside: draw a small cross marker (5×5 pixel block)
        for (int dx = -2; dx <= 2; dx++)
            for (int dy = -2; dy <= 2; dy++)
                SetPixel(cr, x + dx, y + dy, color);
        cout << "  Point INSIDE - visible" << endl;
    } else {
        cout << "  Point OUTSIDE - clipped" << endl;
    }
}

// ============================================================================
// CLIPPING ALGORITHM 11: COHEN-SUTHERLAND LINE CLIPPING
// ============================================================================
/**
 * The Cohen-Sutherland algorithm clips a line segment to a rectangular window.
 *
 * Core concept — Outcodes:
 *   Each endpoint is assigned a 4-bit "outcode":
 *     bit 0 (left):   x < xl
 *     bit 1 (top):    y < yt
 *     bit 2 (right):  x > xr
 *     bit 3 (bottom): y > yb
 *
 * Algorithm:
 *   1. Compute outcodes for both endpoints (o1, o2)
 *   2. If o1 = o2 = 0: both inside → trivially accept (draw the line)
 *   3. If o1 & o2 ≠ 0: both on same side → trivially reject (discard)
 *   4. Otherwise: one endpoint is outside → intersect with one of the
 *      four clip boundaries and replace the outside endpoint
 *   5. Repeat until trivial accept or reject
 *
 * Intersection computation:
 *   Vertical edge (x = xl or x = xr):
 *     y = y₁ + (x - x₁) · (y₂ - y₁) / (x₂ - x₁)
 *   Horizontal edge (y = yt or y = yb):
 *     x = x₁ + (y - y₁) · (x₂ - x₁) / (y₂ - y₁)
 */

/**
 * OutCode: A 4-bit code representing a point's position relative to
 * the clip window. Uses a bit-field union for compact storage.
 *
 * Bits:
 *   left (bit 0):   1 if x < xl
 *   top (bit 1):    1 if y < yt
 *   right (bit 2):  1 if x > xr
 *   bottom (bit 3): 1 if y > yb
 */
union OutCode {
    unsigned All : 4;  // All 4 bits as a single value (for trivial acceptance/rejection)
    struct {
        unsigned left : 1, top : 1, right : 1, bottom : 1;
    };  // Individual bit access
};

/**
 * GetOutCode: Computes the outcode for a point (x,y) against
 * the clipping rectangle defined by (xl, yt, xr, yb).
 */
static OutCode GetOutCode(double x, double y,
                          int xl, int yt, int xr, int yb) {
    OutCode out;
    out.All = 0;
    if (x < xl)          out.left   = 1;   // Left of clip window
    else if (x > xr)     out.right  = 1;   // Right of clip window
    if (y < yt)          out.top    = 1;   // Above clip window
    else if (y > yb)     out.bottom = 1;   // Below clip window
    return out;
}

/**
 * VIntersect: Computes intersection of line with a VERTICAL edge (x = constant).
 * Uses parameric line equation: y = y₁ + (x - x₁) · (y₂ - y₁) / (x₂ - x₁)
 *
 * @param xs, ys   Line start
 * @param xe, ye   Line end
 * @param x        X-coordinate of the vertical edge
 * @param xi, yi   [out] Intersection point
 */
static void VIntersect(double xs, double ys, double xe, double ye,
                       int x, double* xi, double* yi) {
    *xi = x;
    *yi = ys + (x - xs) * (ye - ys) / (xe - xs);
}

/**
 * HIntersect: Computes intersection of line with a HORIZONTAL edge (y = constant).
 * Uses parameric line equation: x = x₁ + (y - y₁) · (x₂ - x₁) / (y₂ - y₁)
 *
 * @param xs, ys   Line start
 * @param xe, ye   Line end
 * @param y        Y-coordinate of the horizontal edge
 * @param xi, yi   [out] Intersection point
 */
static void HIntersect(double xs, double ys, double xe, double ye,
                       int y, double* xi, double* yi) {
    *yi = y;
    *xi = xs + (y - ys) * (xe - xs) / (ye - ys);
}

/**
 * CohenSuth: The main Cohen-Sutherland line clipping algorithm.
 * Clips line from (xs,ys) to (xe,ye) to the rectangular window.
 * Draws the clipped portion (if any) using Cairo line primitives.
 */
static void CohenSuth(cairo_t* cr,
                      int xs, int ys, int xe, int ye,
                      int xl, int yt, int xr, int yb, Color color) {
    double x1 = xs, y1 = ys, x2 = xe, y2 = ye;
    OutCode o1 = GetOutCode(x1, y1, xl, yt, xr, yb);
    OutCode o2 = GetOutCode(x2, y2, xl, yt, xr, yb);

    // Loop until we either accept or reject
    while ((o1.All || o2.All) && !(o1.All & o2.All)) {
        double xi, yi;

        // Pick the outside endpoint to clip
        if (o1.All) {
            // Clip endpoint 1 (it's outside)
            if (o1.left)        VIntersect(x1, y1, x2, y2, xl, &xi, &yi);
            else if (o1.top)    HIntersect(x1, y1, x2, y2, yt, &xi, &yi);
            else if (o1.right)  VIntersect(x1, y1, x2, y2, xr, &xi, &yi);
            else                HIntersect(x1, y1, x2, y2, yb, &xi, &yi);
            x1 = xi; y1 = yi;
            o1 = GetOutCode(x1, y1, xl, yt, xr, yb);
        } else {
            // Clip endpoint 2 (endpoint 1 is inside, endpoint 2 is outside)
            if (o2.left)        VIntersect(x1, y1, x2, y2, xl, &xi, &yi);
            else if (o2.top)    HIntersect(x1, y1, x2, y2, yt, &xi, &yi);
            else if (o2.right)  VIntersect(x1, y1, x2, y2, xr, &xi, &yi);
            else                HIntersect(x1, y1, x2, y2, yb, &xi, &yi);
            x2 = xi; y2 = yi;
            o2 = GetOutCode(x2, y2, xl, yt, xr, yb);
        }
    }

    // If both endpoints are inside (outcode = 0), draw the clipped line
    if (!o1.All && !o2.All) {
        cairo_set_source_rgb(cr, color.r, color.g, color.b);
        cairo_set_line_width(cr, 1.0);
        cairo_move_to(cr, Round(x1), Round(y1));
        cairo_line_to(cr, Round(x2), Round(y2));
        cairo_stroke(cr);
        cout << "  Line clipped and drawn." << endl;
    } else {
        cout << "  Line completely outside - clipped." << endl;
    }
}

// ============================================================================
// CLIPPING ALGORITHM 12: SUTHERLAND-HODGMAN POLYGON CLIPPING
// ============================================================================
/**
 * The Sutherland-Hodgman algorithm clips a polygon against each edge of
 * the clip window sequentially (left, top, right, bottom).
 *
 * Algorithm per edge:
 *   For each consecutive pair of vertices (v₁, v₂):
 *     - If v₁ is inside AND v₂ is inside:   output v₂
 *     - If v₁ is inside AND v₂ is outside:  output intersection
 *     - If v₁ is outside AND v₂ is inside:  output intersection, then v₂
 *     - If v₁ is outside AND v₂ is outside: output nothing
 *
 * The output of one edge becomes the input for the next edge, forming
 * a pipeline. After all 4 edges, we get the final clipped polygon.
 */

/**
 * Vertex: A vertex with floating-point coordinates (used during clipping
 * for precise intersection calculations).
 */
struct Vertex {
    double x, y;
    Vertex(double a = 0, double b = 0) : x(a), y(b) {}
};

typedef vector<Vertex> VList;         // List of vertices
typedef bool    (*IsInFn)(Vertex&, int);  // Function pointer: "is inside edge?"
typedef Vertex (*IsectFn)(Vertex&, Vertex&, int);  // Function ptr: "intersect with edge"

// --- Inside-test functions ---
// Each returns true if the vertex is on the "inside" side of the edge
static bool InLeft(Vertex& v, int e)   { return v.x >= e; }
static bool InRight(Vertex& v, int e)  { return v.x <= e; }
static bool InTop(Vertex& v, int e)    { return v.y >= e; }
static bool InBottom(Vertex& v, int e) { return v.y <= e; }

// --- Intersection functions ---
// Compute the intersection of line (v1, v2) with a vertical/horizontal edge
static Vertex VIsectPoly(Vertex& v1, Vertex& v2, int xedge) {
    Vertex res ;
    res.x = xedge ;
    res.y = v1.y + (xedge - v1.x) * (v2.y - v1.y) / (v2.x - v1.x);

    return res ;
}
static Vertex HIsectPoly(Vertex& v1, Vertex& v2, int yedge) {
    Vertex res;
    res.y = yedge;
    res.x = v1.x + (yedge - v1.y) * (v2.x - v1.x) / (v2.y - v1.y);
    return res;
}

/**
 * ClipEdge: Clips a polygon (vertex list) against ONE edge of the
 * clip window using the Sutherland-Hodgman rules.
 *
 * @param p      Input polygon vertices
 * @param edge   Edge coordinate (e.g., xl for left, yt for top)
 * @param In     Inside-test function
 * @param Isect  Intersection function
 * @return The clipped polygon vertices for this edge
 */
static VList ClipEdge(VList p, int edge, IsInFn In, IsectFn Isect) {
    VList out;
    if (p.empty()) return out;

    Vertex v1 = p.back();             // Start with last vertex
    bool v1in = In(v1, edge);          // Is it inside?

    for (auto& v2 : p) {
        bool v2in = In(v2, edge);      // Is current vertex inside?
        if (!v1in && v2in) {
            // v1 outside → v2 inside: output intersection, then v2
            out.push_back(Isect(v1, v2, edge));
            out.push_back(v2);
        } else if (v1in && v2in) {
            // Both inside: output v2
            out.push_back(v2);
        } else if (v1in) {
            // v1 inside → v2 outside: output intersection only
            out.push_back(Isect(v1, v2, edge));
        }
        // Both outside: output nothing
        v1 = v2;
        v1in = v2in;
    }
    return out;
}

/**
 * PolygonClip: Clips a polygon against all 4 edges of the rectangular
 * clip window by calling ClipEdge sequentially.
 *
 * The clipped result is drawn using Cairo line segments.
 *
 * @param cr        Cairo context
 * @param pts       Input polygon vertices (from user clicks)
 * @param xl, yt    Top-left of clip window
 * @param xr, yb    Bottom-right of clip window
 * @param color     Drawing color
 */
static void PolygonClip(cairo_t* cr, vector<Point2D>& pts,
                        int xl, int yt, int xr, int yb, Color color) {
    // Convert Point2D to Vertex list
    VList vl;
    for (auto& p : pts)
        vl.push_back(Vertex(p.x, p.y));

    // Pipeline: clip against left, top, right, bottom edges sequentially
    vl = ClipEdge(vl, xl, InLeft,   VIsectPoly);
    vl = ClipEdge(vl, yt, InTop,    HIsectPoly);
    vl = ClipEdge(vl, xr, InRight,  VIsectPoly);
    vl = ClipEdge(vl, yb, InBottom, HIsectPoly);

    if (vl.size() < 2) {
        cout << "  Polygon completely outside." << endl;
        return;
    }

    // Draw the clipped polygon as a series of line segments
    cairo_set_source_rgb(cr, color.r, color.g, color.b);
    cairo_set_line_width(cr, 1.0);
    Vertex v1 = vl.back();
    for (auto& v2 : vl) {
        cairo_move_to(cr, Round(v1.x), Round(v1.y));
        cairo_line_to(cr, Round(v2.x), Round(v2.y));
        cairo_stroke(cr);
        v1 = v2;
    }
    cout << "  Polygon clipped with " << vl.size() << " vertices." << endl;
}

// ============================================================================
// ============================================================================
// CLIPPING ALGORITHM 13: CIRCULAR POINT CLIPPING  (continued)
// ============================================================================
/**
 * PointCircleClip: Tests whether a point (x,y) is inside an existing
 * drawn circle. If it is, fills the entire circle's bounding box with
 * the drawing color.
 *
 * This is a visual "clipping" effect — only points inside the circle
 * are shown (by filling the circle region).
 *
 * @param cr        Cairo context
 * @param xc, yc    Circle center (from the existing circle shape)
 * @param x, y      Point to test
 * @param R         Circle radius
 * @param color     Drawing color
 */
static void PointCircleClip(cairo_t* cr, int xc, int yc,
                            int x, int y, int R, Color color) {
    if (CheckIFPointINCircle(R, xc, yc, x, y)) {
        // Draw a small cross marker at the test point
        for (int dx = -2; dx <= 2; dx++)
            for (int dy = -2; dy <= 2; dy++)
                SetPixel(cr, x + dx, y + dy, color);
    }
}

// ============================================================================
// CLIPPING ALGORITHM 14: CIRCULAR LINE CLIPPING
// ============================================================================
/**
 * LineCircleClip: Steps along a line pixel by pixel and only draws
 * pixels that lie inside a given circle. This produces a "line clipped
 * to circle" effect where the line is only visible within the circle.
 *
 * Algorithm:
 *   1. Parametrically step along the line from (x1,y1) to (x2,y2)
 *   2. At each step, test if the current pixel is inside the circle
 *   3. Draw the pixel only if it passes the inside test
 *
 * @param cr           Cairo context
 * @param xc, yc       Circle center
 * @param x1, y1       Line start
 * @param x2, y2       Line end
 * @param R            Circle radius
 * @param color        Drawing color
 */
static void LineCircleClip(cairo_t* cr, int xc, int yc,
                           int x1, int y1, int x2, int y2,
                           int R, Color color) {
    int dx = x2 - x1, dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));  // Number of steps = longer axis
    double xInc = (steps > 0) ? (double)dx / steps : 0;  // X increment per step
    double yInc = (steps > 0) ? (double)dy / steps : 0;  // Y increment per step
    double x = x1, y = y1;

    for (int i = 0; i <= steps; i++) {
        // Only draw pixels that fall inside the circle
        if (CheckIFPointINCircle(R, xc, yc, Round(x), Round(y))) {
            SetPixel(cr, Round(x), Round(y), color);
        }
        x += xInc;  // Advance along the line
        y += yInc;
    }
}

// ============================================================================
// SHAPE RENDERING DISPATCHER
// ============================================================================
/**
 * RenderShape: The central rendering dispatcher.
 *
 * Called by on_draw() for each Shape stored in g_shapes.
 * Based on the shape's mode, it calls the appropriate drawing function.
 *
 * This is the "switchboard" that connects stored shapes to drawing algorithms.
 *
 * @param cr   Cairo context (from the draw signal)
 * @param s    The shape to render
 */
static void RenderShape(cairo_t* cr, const Shape& s) {
    switch (s.mode) {
    // ====================================================================
    // LINE ALGORITHMS
    // ====================================================================
    case MODE_DDA_LINE:
        // First click = (x1,y1), second click = (x2,y2)
        DrawLineDDA(cr, s.x1, s.y1, s.x2, s.y2, s.color);
        break;

    case MODE_MIDPOINT_LINE:
        DrawLineMidpoint(cr, s.x1, s.y1, s.x2, s.y2, s.color);
        break;

    // ====================================================================
    // CIRCLE ALGORITHMS
    // ====================================================================
    // For all circle modes: (x1,y1) = center, (x2,y2) = point on circumference
    // Radius R = distance between center and circumference point
    case MODE_MODIFIED_MIDPOINT_CIRCLE:
    case MODE_DIRECT_CIRCLE:
    case MODE_BRESENHAM_CIRCLE:
    case MODE_POLAR_CIRCLE:
    case MODE_ITERATIVE_POLAR_CIRCLE: {
        // Compute radius from the two stored points
        int dx = s.x2 - s.x1, dy = s.y2 - s.y1;
        int R = (int)sqrt(dx * dx + dy * dy);
        if (R <= 0) break;  // Skip degenerate circles

        // Dispatch to the specific algorithm
        switch (s.mode) {
        case MODE_MODIFIED_MIDPOINT_CIRCLE:
            DrawCircleModifiedMidpoint(cr, s.x1, s.y1, R, s.color);
            break;
        case MODE_DIRECT_CIRCLE:
            DrawCircleDirect(cr, s.x1, s.y1, R, s.color);
            break;
        case MODE_BRESENHAM_CIRCLE:
            DrawCircleBresenham(cr, s.x1, s.y1, R, s.color);
            break;
        case MODE_POLAR_CIRCLE:
            DrawCirclePolar(cr, s.x1, s.y1, R, s.color);
            break;
        case MODE_ITERATIVE_POLAR_CIRCLE:
            DrawCircleIterativePolar(cr, s.x1, s.y1, R, s.color);
            break;
        default: break;
        }
        break;
    }

    // ====================================================================
    // ELLIPSE ALGORITHM
    // ====================================================================
    case MODE_MIDPOINT_ELLIPSE: {
        // (x1,y1) = center, radii a=|x2-x1|, b=|y2-y1|
        int a = abs(s.x2 - s.x1);
        int b = abs(s.y2 - s.y1);
        DrawEllipseMidpoint(cr, s.x1, s.y1, a, b, s.color);
        break;
    }

    // ====================================================================
    // FILL CIRCLE WITH LINES
    // ====================================================================
    case MODE_FILL_CIRCLE_WITH_LINES: {
        // (x1,y1) = circle center; (x2,y2) = click point for quadrant detection
        int dx = s.x2 - s.x1, dy = s.y2 - s.y1;
        int R = (int)sqrt(dx * dx + dy * dy);
        if (R <= 0) break;

        // Determine which quadrant the click fell in
        double angle = atan2((double)(s.y2 - s.y1), (double)(s.x2 - s.x1));
        const double PI = 3.14159265;
        double thetaS, thetaE;

        if (angle >= 0 && angle < PI / 2) {
            thetaS = 0; thetaE = PI / 2;         // 1st quadrant (0° to 90°)
        } else if (angle >= PI / 2 && angle < PI) {
            thetaS = PI / 2; thetaE = PI;         // 2nd quadrant (90° to 180°)
        } else if (angle >= -PI && angle < -PI / 2) {
            thetaS = PI; thetaE = 3 * PI / 2;     // 3rd quadrant (180° to 270°)
        } else {
            thetaS = 3 * PI / 2; thetaE = 2 * PI; // 4th quadrant (270° to 360°)
        }

        FillCircleWithLines(cr, s.x1, s.y1, R, thetaS, thetaE, s.color);
        break;
    }

    // ====================================================================
    // CLIPPING: POINT
    // ====================================================================
    case MODE_CLIP_POINT:
        // Draw the clip window and test the stored point
        DrawClipWindow(cr);
        PointClipping(cr, s.x1, s.y1,
                      g_clipX1, g_clipY1, g_clipX2, g_clipY2, s.color);
        break;

    // ====================================================================
    // CLIPPING: LINE (Cohen-Sutherland)
    // ====================================================================
    case MODE_CLIP_LINE:
        // Draw the clip window and clip the stored line to it
        DrawClipWindow(cr);
        CohenSuth(cr, s.x1, s.y1, s.x2, s.y2,
                  g_clipX1, g_clipY1, g_clipX2, g_clipY2, s.color);
        break;

    // ====================================================================
    // CLIPPING: CIRCULAR POINT
    // ====================================================================
    case MODE_CLIP_CIRCLE_POINT: {
        // (x1,y1) = click point, (x2,y2) = circle center
        // Find the circle to get its radius and test the click point
        int xc, yc, R;
        if (FindCircle(s.x1, s.y1, xc, yc, R)) {
            PointCircleClip(cr, xc, yc, s.x1, s.y1, R, s.color);
        }
        break;
    }

    // ====================================================================
    // CLIPPING: CIRCULAR LINE
    // ====================================================================
    case MODE_CLIP_CIRCLE_LINE: {
        // (x1,y1)-(x2,y2) = line to clip. Find a circle at (x2,y2).
        int xc, yc, R;
        if (FindCircle(s.x2, s.y2, xc, yc, R)) {
            LineCircleClip(cr, xc, yc, s.x1, s.y1, s.x2, s.y2, R, s.color);
        }
        break;
    }

    default:
        // MODE_NONE and any unhandled modes: do nothing
        break;
    }
}

// ============================================================================
// GTK SIGNAL: "draw"
// ============================================================================
/**
 * on_draw: The main rendering callback. Connected to the "draw" signal
 * of g_drawingArea. Called whenever the widget needs repainting.
 *
 * This is the ONLY place where Cairo drawing commands should be issued.
 * NEVER draw outside this callback.
 *
 * Rendering order:
 *   1. Fill entire widget with background color
 *   2. Render each shape from g_shapes via RenderShape()
 *   3. Draw clipping window overlay if in a clipping mode
 *   4. Draw polygon point markers if collecting polygon vertices
 *   5. Draw clipped polygon result if available
 *
 * @param widget   The drawing area widget (unused)
 * @param cr       Cairo context provided by GTK (automatically clipped)
 * @param data     User data (unused)
 * @return FALSE to let other handlers run (standard GTK convention)
 */
static gboolean on_draw(GtkWidget* /*widget*/, cairo_t* cr,
                        gpointer /*data*/) {
    // ---- Step 1: Paint background ----
    cairo_set_source_rgb(cr, g_backgroundColor.r,
                             g_backgroundColor.g,
                             g_backgroundColor.b);
    cairo_paint(cr);  // Fills entire clip region with source color

    // ---- Step 2: Redraw all stored shapes ----
    for (const auto& shape : g_shapes)
        RenderShape(cr, shape);

    // ---- Step 3: Draw clip window overlay for active clipping modes ----
    if (g_currentMode == MODE_CLIP_POINT ||
        g_currentMode == MODE_CLIP_LINE ||
        g_currentMode == MODE_CLIP_POLYGON) {
        DrawClipWindow(cr);
    }
    // Also show clip window if there are clipped polygon results
    if (g_polygonClipped) {
        DrawClipWindow(cr);
    }

    // ---- Step 4: Draw polygon point markers (while collecting) ----
    if (g_currentMode == MODE_CLIP_POLYGON && !g_polygonPoints.empty()) {
        cairo_set_source_rgb(cr, g_drawColor.r, g_drawColor.g, g_drawColor.b);
        for (auto& pt : g_polygonPoints) {
            // Draw a small cross marker at each collected point
            for (int d = -2; d <= 2; d++) {
                SetPixel(cr, pt.x + d, pt.y, g_drawColor);  // Horizontal bar
                SetPixel(cr, pt.x, pt.y + d, g_drawColor);  // Vertical bar
            }
        }
    }

    // ---- Step 5: Draw clipped polygon result (after right-click) ----
    if (g_polygonClipped && g_polygonResult.size() >= 2) {
        cairo_set_source_rgb(cr, g_drawColor.r, g_drawColor.g, g_drawColor.b);
        cairo_set_line_width(cr, 1.0);
        size_t n = g_polygonResult.size();
        for (size_t i = 0; i < n; i++) {
            size_t j = (i + 1) % n;  // Next vertex (wraps around)
            cairo_move_to(cr, g_polygonResult[i].x, g_polygonResult[i].y);
            cairo_line_to(cr, g_polygonResult[j].x, g_polygonResult[j].y);
            cairo_stroke(cr);
        }
    }

    return FALSE;  // FALSE = event handled, let GTK continue propagation
}

// ============================================================================
// GTK SIGNAL: "button-press-event"
// ============================================================================
/**
 * on_button_press: Handles mouse clicks on the drawing area.
 * Replaces the Windows WM_LBUTTONDOWN / WM_RBUTTONDOWN message handlers.
 *
 * IMPORTANT: This function NEVER draws directly. It only:
 *   1. Updates application state (stores shapes, coordinates, flags)
 *   2. Calls gtk_widget_queue_draw() to request a redraw
 *
 * The actual rendering happens in on_draw().
 *
 * Click handling per mode:
 *   - MODE_NONE:             Shows error message
 *   - MODE_CLIP_POINT:       Single click → stores point as shape
 *   - MODE_CLIP_POLYGON:     Left-click adds vertex; Right-click clips + stores
 *   - MODE_CLIP_CIRCLE_POINT: Single click → finds circle, stores clip result
 *   - MODE_MIDPOINT_ELLIPSE: 3 clicks → center, b-radius, a-radius
 *   - MODE_FILL_CIRCLE_WITH_LINES: Single click → finds circle, stores fill
 *   - All other modes:       2 clicks → first stores start, second stores shape
 *
 * @param widget   The widget that received the event (drawing area)
 * @param event    Event details (button number, coordinates, etc.)
 * @param data     User data (unused)
 * @return TRUE if the event was handled, FALSE otherwise
 */
static gboolean on_button_press(GtkWidget* widget,
                                GdkEventButton* event,
                                gpointer /*data*/) {
    // Get click coordinates (relative to the drawing area widget)
    int x = (int)event->x;
    int y = (int)event->y;

    // ====================================================================
    // RIGHT-CLICK HANDLER
    // ====================================================================
    // Used to finalize polygon clipping (finish collecting vertices)
    if (event->button == GDK_BUTTON_SECONDARY) {
        // Only process if in polygon clip mode and we have enough points
        if (g_currentMode == MODE_CLIP_POLYGON && g_polygonPoints.size() >= 3) {
            // Step 1: Store the original (unclipped) polygon edges as line shapes
            // This preserves the user's original polygon for redrawing
            size_t n = g_polygonPoints.size();
            for (size_t i = 0; i < n; i++) {
                Shape edge;
                edge.mode  = MODE_DDA_LINE;
                edge.color = g_drawColor;
                edge.x1    = g_polygonPoints[i].x;
                edge.y1    = g_polygonPoints[i].y;
                edge.x2    = g_polygonPoints[(i + 1) % n].x;
                edge.y2    = g_polygonPoints[(i + 1) % n].y;
                g_shapes.push_back(edge);
            }

            // Step 2: Run Sutherland-Hodgman algorithm to compute clipped vertices
            vector<Point2D> clipped;
            {
                // Convert int points to Vertex (double) for precise math
                vector<Vertex> vl;
                for (auto& p : g_polygonPoints)
                    vl.push_back(Vertex(p.x, p.y));

                // Pipeline: clip against all 4 edges
                vl = ClipEdge(vl, g_clipX1, InLeft,   VIsectPoly);
                vl = ClipEdge(vl, g_clipY1, InTop,    HIsectPoly);
                vl = ClipEdge(vl, g_clipX2, InRight,  VIsectPoly);
                vl = ClipEdge(vl, g_clipY2, InBottom, HIsectPoly);

                // Convert back to integer points
                for (auto& v : vl)
                    clipped.push_back(Point2D(Round(v.x), Round(v.y)));

                cout << "  Polygon clipped with " << vl.size() << " vertices." << endl;
            }

            // Step 3: Store the clipped result for redrawing
            if (clipped.size() >= 2) {
                g_polygonResult = clipped;
                g_polygonClipped = true;
            }

            // Step 4: Clear temporary vertex storage and request redraw
            g_polygonPoints.clear();
            gtk_widget_queue_draw(widget);
        }
        return TRUE;  // Right-click always handled (prevents context menus)
    }

    // ====================================================================
    // LEFT-CLICK HANDLER
    // ====================================================================
    if (event->button != GDK_BUTTON_PRIMARY) return FALSE;

    // If no mode is selected, show a helpful message
    if (g_currentMode == MODE_NONE) {
        cout << "Please select a drawing mode from the menu first." << endl;
        return TRUE;
    }

    // ---- Mode: Point Clipping (single click) ----
    if (g_currentMode == MODE_CLIP_POINT) {
        // Store the click point as a shape with the clipping mode
        Shape s;
        s.mode  = MODE_CLIP_POINT;
        s.color = g_drawColor;
        s.x1    = x; s.y1 = y;   // The point to test
        s.x2    = 0; s.y2 = 0;   // Unused
        g_shapes.push_back(s);
        gtk_widget_queue_draw(widget);
        return TRUE;
    }

    // ---- Mode: Polygon Clipping (collect vertices) ----
    if (g_currentMode == MODE_CLIP_POLYGON) {
        g_polygonPoints.push_back(Point2D(x, y));
        gtk_widget_queue_draw(widget);
        cout << "  Point added (" << x << "," << y << ")" << endl;
        return TRUE;
    }

    // ---- Mode: Circular Point Clipping (single click on circle) ----
    if (g_currentMode == MODE_CLIP_CIRCLE_POINT) {
        int xc, yc, R;
        if (FindCircle(x, y, xc, yc, R)) {
            // Store the clip result as a shape
            Shape s;
            s.mode  = MODE_CLIP_CIRCLE_POINT;
            s.color = g_drawColor;
            s.x1    = x;        // Store click point x
            s.y1    = y;        // Store click point y
            s.x2    = xc;       // Store circle center
            s.y2    = yc;
            g_shapes.push_back(s);
            cout << "  Point inside circle - clipping applied." << endl;
        } else {
            cout << "  No circle found at that point." << endl;
        }
        gtk_widget_queue_draw(widget);
        return TRUE;
    }

    // ---- Mode: Midpoint Ellipse (3 clicks) ----
    if (g_currentMode == MODE_MIDPOINT_ELLIPSE) {
        if (!g_firstClick) {
            // Click 1: Store center point
            g_startX = x; g_startY = y;
            g_firstClick = true;
            cout << "  Ellipse center: (" << x << ", " << y << ")" << endl;
        } else if (!g_secondClick) {
            // Click 2: Store b-radius point (first radius)
            g_X1 = x; g_Y1 = y;
            g_secondClick = true;
            cout << "  First radius point: (" << x << ", " << y << ")" << endl;
        } else {
            // Click 3: Compute both radii and draw the ellipse
            // b = distance from center to 2nd click
            int b = (int)sqrt(pow(g_X1 - g_startX, 2)
                            + pow(g_Y1 - g_startY, 2));
            // a = distance from center to 3rd click
            int a = (int)sqrt(pow(x - g_startX, 2)
                            + pow(y - g_startY, 2));

            Shape s;
            s.mode  = MODE_MIDPOINT_ELLIPSE;
            s.color = g_drawColor;
            s.x1    = g_startX;       // Center X
            s.y1    = g_startY;       // Center Y
            s.x2    = g_startX + a;   // Store a as offset
            s.y2    = g_startY + b;   // Store b as offset
            g_shapes.push_back(s);

            cout << "  Ellipse drawn: a=" << a << ", b=" << b << endl;
            g_firstClick = false;
            g_secondClick = false;
            gtk_widget_queue_draw(widget);
        }
        return TRUE;
    }

    // ---- Mode: Fill Circle with Lines (single click on circle quadrant) ----
    if (g_currentMode == MODE_FILL_CIRCLE_WITH_LINES) {
        int xc, yc, R;
        if (!FindCircle(x, y, xc, yc, R)) {
            cout << "  No circle found at that point." << endl;
            return TRUE;
        }

        // Store the fill operation as a shape
        Shape s;
        s.mode  = MODE_FILL_CIRCLE_WITH_LINES;
        s.color = g_drawColor;
        s.x1    = xc;       // Circle center (found by FindCircle)
        s.y1    = yc;
        s.x2    = x;        // Click point (for quadrant detection)
        s.y2    = y;
        g_shapes.push_back(s);

        gtk_widget_queue_draw(widget);

        // Report which quadrant was filled
        double angle = atan2((double)(y - yc), (double)(x - xc));
        const double PI = 3.14159265;
        string Q;
        if (angle >= 0 && angle < PI / 2)         Q = "1st";
        else if (angle >= PI / 2 && angle < PI)   Q = "2nd";
        else if (angle >= -PI && angle < -PI / 2) Q = "3rd";
        else                                       Q = "4th";
        cout << "  Filling " << Q << " quarter of the circle." << endl;
        return TRUE;
    }

    // ---- Two-click modes (Lines, Circles, Line Clipping, etc.) ----
    if (!g_firstClick) {
        // FIRST CLICK: Store the start point
        g_startX    = x;
        g_startY    = y;
        g_firstClick = true;
        cout << "  First point: (" << x << ", " << y << ")" << endl;
    } else {
        // SECOND CLICK: Create and store the shape, then queue redraw
        g_firstClick = false;
        cout << "  Second point: (" << x << ", " << y << ")" << endl;

        // Create a shape with the current mode and both click points
        Shape s;
        s.mode  = g_currentMode;
        s.color = g_drawColor;
        s.x1    = g_startX;
        s.y1    = g_startY;
        s.x2    = x;
        s.y2    = y;
        g_shapes.push_back(s);

        // Console feedback about what was drawn
        switch (g_currentMode) {
        case MODE_DDA_LINE:
            cout << "  -> DDA Line drawn." << endl;
            break;
        case MODE_MIDPOINT_LINE:
            cout << "  -> Midpoint Line drawn." << endl;
            break;
        case MODE_MODIFIED_MIDPOINT_CIRCLE: {
            int R = (int)sqrt(pow(x - g_startX, 2) + pow(y - g_startY, 2));
            cout << "  -> Modified Midpoint Circle R=" << R << endl;
            break;
        }
        case MODE_DIRECT_CIRCLE: {
            int R = (int)sqrt(pow(x - g_startX, 2) + pow(y - g_startY, 2));
            cout << "  -> Direct Circle R=" << R << endl;
            break;
        }
        case MODE_BRESENHAM_CIRCLE: {
            int R = (int)sqrt(pow(x - g_startX, 2) + pow(y - g_startY, 2));
            cout << "  -> Bresenham Circle R=" << R << endl;
            break;
        }
        case MODE_POLAR_CIRCLE: {
            int R = (int)sqrt(pow(x - g_startX, 2) + pow(y - g_startY, 2));
            cout << "  -> Polar Circle R=" << R << endl;
            break;
        }
        case MODE_ITERATIVE_POLAR_CIRCLE: {
            int R = (int)sqrt(pow(x - g_startX, 2) + pow(y - g_startY, 2));
            cout << "  -> Iterative Polar Circle R=" << R << endl;
            break;
        }
        case MODE_CLIP_LINE:
            cout << "  -> Line clipped to rectangle." << endl;
            break;
        case MODE_CLIP_CIRCLE_LINE: {
            int xc, yc, R;
            if (FindCircle(x, y, xc, yc, R)) {
                cout << "  -> Line clipped to circle." << endl;
            } else {
                // Invalid: second click wasn't on a circle → undo
                cout << "  No circle at the second point." << endl;
                g_shapes.pop_back();  // Remove the invalid shape
            }
            break;
        }
        default:
            break;
        }

        // Request a full redraw (on_draw will call RenderShape for all shapes)
        gtk_widget_queue_draw(widget);
    }

    return TRUE;  // Event was handled
}

// ============================================================================
// FILE SAVE / LOAD
// ============================================================================
static void SaveToFile(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        cout << "  Error: Cannot open file for writing." << endl;
        return;
    }
    int n = (int)g_shapes.size();
    fwrite(&n, sizeof(int), 1, f);
    for (auto& s : g_shapes) {
        int modeVal = (int)s.mode;
        fwrite(&modeVal, sizeof(int), 1, f);
        fwrite(&s.color, sizeof(Color), 1, f);
        fwrite(&s.x1, sizeof(int), 1, f);
        fwrite(&s.y1, sizeof(int), 1, f);
        fwrite(&s.x2, sizeof(int), 1, f);
        fwrite(&s.y2, sizeof(int), 1, f);
    }
    fclose(f);
    cout << "  Saved " << n << " shapes to '" << filename << "'" << endl;
}

static void LoadFromFile(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        cout << "  Error: Cannot open file for reading." << endl;
        return;
    }
    int n = 0;
    fread(&n, sizeof(int), 1, f);
    if (n < 0 || n > 100000) {
        cout << "  Error: Invalid file format." << endl;
        fclose(f);
        return;
    }
    g_shapes.clear();
    g_polygonPoints.clear();
    g_polygonResult.clear();
    g_polygonClipped = false;
    g_firstClick = false;
    g_secondClick = false;
    for (int i = 0; i < n; i++) {
        Shape s;
        int modeVal;
        fread(&modeVal, sizeof(int), 1, f);
        s.mode = (DrawMode)modeVal;
        fread(&s.color, sizeof(Color), 1, f);
        fread(&s.x1, sizeof(int), 1, f);
        fread(&s.y1, sizeof(int), 1, f);
        fread(&s.x2, sizeof(int), 1, f);
        fread(&s.y2, sizeof(int), 1, f);
        g_shapes.push_back(s);
    }
    fclose(f);
    gtk_widget_queue_draw(g_drawingArea);
    cout << "  Loaded " << n << " shapes from '" << filename << "'" << endl;
}

// ============================================================================
// MENU CALLBACKS
// ============================================================================
// Each callback corresponds to a menu item. They set the current mode or
// modify application state, then request a redraw if needed.

// ---- File > Clear Screen ----
static void on_file_clear(GtkMenuItem* /*item*/, gpointer /*data*/) {
    g_shapes.clear();             // Remove all drawn shapes
    g_polygonPoints.clear();      // Clear polygon vertices being collected
    g_polygonResult.clear();      // Clear clipped polygon result
    g_polygonClipped = false;     // Reset clipped flag
    g_firstClick = false;         // Reset click state
    g_secondClick = false;
    gtk_widget_queue_draw(g_drawingArea);  // Redraw (now empty)
    cout << "Screen cleared." << endl;
}

// ---- Clear Button Callback ----
// static void on_clear_button(GtkButton*, gpointer) {
//     g_shapes.clear();
//     g_polygonPoints.clear();
//     g_polygonResult.clear();
//     g_polygonClipped = false;
//     g_firstClick = false;
//     g_secondClick = false;
//     gtk_widget_queue_draw(g_drawingArea);
//     cout << "Screen cleared." << endl;
// }

// ---- File > Save to File ----
static void on_file_save(GtkMenuItem*, gpointer) {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Save to File", GTK_WINDOW(g_mainWindow),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(
        GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        SaveToFile(filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// ---- File > Load from File ----
static void on_file_load(GtkMenuItem*, gpointer) {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Load from File", GTK_WINDOW(g_mainWindow),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        LoadFromFile(filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// ---- Preferences > Background: Black ----
static void on_bg_black(GtkMenuItem* /*item*/, gpointer /*data*/) {
    g_backgroundColor = {0.0, 0.0, 0.0};
    gtk_widget_queue_draw(g_drawingArea);
    cout << "Background color changed to Black." << endl;
}

// ---- Preferences > Background: White ----
static void on_bg_white(GtkMenuItem* /*item*/, gpointer /*data*/) {
    g_backgroundColor = {1.0, 1.0, 1.0};
    gtk_widget_queue_draw(g_drawingArea);
    cout << "Background color changed to White." << endl;
}

// ---- Preferences > Choose Drawing Color... ----
/**
 * Opens a GTK color chooser dialog. When the user picks a color,
 * it updates g_drawColor. All subsequent shapes will use this color.
 */
static void on_choose_color(GtkMenuItem* /*item*/, gpointer /*data*/) {
    // Create a color chooser dialog
    GtkWidget* dialog = gtk_color_chooser_dialog_new(
        "Choose Drawing Color", GTK_WINDOW(g_mainWindow));

    // Set initial color to current draw color
    GdkRGBA rgba = { g_drawColor.r, g_drawColor.g, g_drawColor.b, 1.0 };
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);

    // Show dialog and wait for response
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);
        g_drawColor = { rgba.red, rgba.green, rgba.blue };
        cout << "Drawing color changed to RGB("
             << (int)(rgba.red   * 255) << ", "
             << (int)(rgba.green * 255) << ", "
             << (int)(rgba.blue  * 255) << ")" << endl;
    }
    gtk_widget_destroy(dialog);  // Clean up
}

// ---- Preferences > Mouse Cursor ----
/**
 * set_cursor_type: Changes the mouse cursor icon for the drawing area.
 * Uses GTK's built-in cursor types.
 *
 * Available cursors:
 *   GDK_LEFT_PTR  → Standard arrow pointer
 *   GDK_CROSSHAIR → Crosshair (+)
 *   GDK_HAND2     → Hand (pointing finger)
 */
static void set_cursor_type(GdkCursorType type, const char* name) {
    GdkWindow*  win  = gtk_widget_get_window(g_drawingArea);
    GdkDisplay* disp = gdk_display_get_default();
    GdkCursor*  cursor = gdk_cursor_new_for_display(disp, type);
    gdk_window_set_cursor(win, cursor);
    g_object_unref(cursor);   // Release cursor reference
    cout << "Mouse cursor changed to " << name << "." << endl;
}

static void on_cursor_arrow(GtkMenuItem*, gpointer) {
    set_cursor_type(GDK_LEFT_PTR, "Arrow");
}
static void on_cursor_cross(GtkMenuItem*, gpointer) {
    set_cursor_type(GDK_CROSSHAIR, "Crosshair");
}
static void on_cursor_hand(GtkMenuItem*, gpointer) {
    set_cursor_type(GDK_HAND2, "Hand");
}

// ---- Lines Menu ----
static void on_lines_dda(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_DDA_LINE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[DDA Line Mode] Click two points to draw a line." << endl;
}

static void on_lines_midpoint(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_MIDPOINT_LINE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Midpoint Line Mode] Click two points to draw a line." << endl;
}

// ---- Circles Menu ----
static void on_circles_modified_midpoint(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_MODIFIED_MIDPOINT_CIRCLE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Modified Midpoint Circle] Click center, then a radius point." << endl;
}

static void on_circles_direct(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_DIRECT_CIRCLE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Direct Circle] Click center, then a radius point." << endl;
}

static void on_circles_bresenham(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_BRESENHAM_CIRCLE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Bresenham Circle] Click center, then a radius point." << endl;
}

static void on_circles_polar(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_POLAR_CIRCLE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Polar Circle] Click center, then a radius point." << endl;
}

static void on_circles_iterative_polar(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_ITERATIVE_POLAR_CIRCLE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Iterative Polar Circle] Click center, then a radius point." << endl;
}

// ---- Ellipses Menu ----
static void on_ellipse_midpoint(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_MIDPOINT_ELLIPSE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Midpoint Ellipse] Click center, then two radius points." << endl;
}

// ---- Filling Menu ----
static void on_fill_circles_lines(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_FILL_CIRCLE_WITH_LINES;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Fill Circle with Lines] Click on a quadrant of an existing circle." << endl;
}

// ---- Clipping Menu ----
static void on_clip_point(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_CLIP_POINT;
    g_firstClick  = false;
    g_secondClick = false;
    g_polygonPoints.clear();
    gtk_widget_queue_draw(g_drawingArea);  // Show clip window overlay
    cout << "\n[Point Clipping] Click a point to test against the clip window." << endl;
}

static void on_clip_line(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_CLIP_LINE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Line Clipping] Click two points for line clipped to clip window." << endl;
}

static void on_clip_polygon(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_CLIP_POLYGON;
    g_firstClick  = false;
    g_secondClick = false;
    g_polygonPoints.clear();
    gtk_widget_queue_draw(g_drawingArea);  // Show clip window overlay
    cout << "\n[Polygon Clipping] Left-click to add points, right-click to finish." << endl;
}

static void on_clip_circle_point(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_CLIP_CIRCLE_POINT;
    g_firstClick  = false;
    g_secondClick = false;
    g_polygonPoints.clear();
    gtk_widget_queue_draw(g_drawingArea);
    cout << "\n[Circle Point Clipping] Click on an existing circle to fill it." << endl;
}

static void on_clip_circle_line(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_CLIP_CIRCLE_LINE;
    g_firstClick  = false;
    g_secondClick = false;
    cout << "\n[Circle Line Clipping] Click two points. The second click must be on an existing circle." << endl;
}

// ============================================================================
// MENU BUILDER
// ============================================================================
/**
 * build_menu_bar: Constructs the full menu bar with all menus and items.
 *
 * Menu hierarchy:
 *   File          → Clear Screen
 *   Preferences   → Background: Black, Background: White,
 *                   Choose Drawing Color..., Separator,
 *                   Mouse Cursor → Arrow, Crosshair, Hand
 *   Lines         → DDA, Midpoint (Bresenham)
 *   Circles       → Modified Midpoint, Direct Equation,
 *                   Midpoint (Bresenham), Polar Circle,
 *                   Iterative Polar Circle
 *   Ellipses      → Midpoint Ellipse
 *   Filling       → Fill Circles with Lines
 *   Clipping      → Rectangular Point Clipping,
 *                   Rectangular Line Clipping,
 *                   Rectangular Polygon Clipping,
 *                   Circular Point Clipping,
 *                   Circular Line Clipping
 *
 * Each item is connected to its corresponding callback via g_signal_connect.
 *
 * @return A GtkWidget* pointing to the menu bar
 */
static GtkWidget* build_menu_bar() {
    GtkWidget* bar = gtk_menu_bar_new();

    // ====================================================================
    // FILE MENU
    // ====================================================================
    GtkWidget* file_menu = gtk_menu_new();
    GtkWidget* file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

        GtkWidget* clear_item = gtk_menu_item_new_with_label("Clear Screen");
        g_signal_connect(clear_item, "activate",
                         G_CALLBACK(on_file_clear), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), clear_item);

        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu),
                              gtk_separator_menu_item_new());

        GtkWidget* save_item = gtk_menu_item_new_with_label("Save to File...");
        g_signal_connect(save_item, "activate",
                         G_CALLBACK(on_file_save), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);

        GtkWidget* load_item = gtk_menu_item_new_with_label("Load from File...");
        g_signal_connect(load_item, "activate",
                         G_CALLBACK(on_file_load), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), load_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), file_item);

    // ====================================================================
    // PREFERENCES MENU
    // ====================================================================
    GtkWidget* pref_menu = gtk_menu_new();
    GtkWidget* pref_item = gtk_menu_item_new_with_label("Preferences");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pref_item), pref_menu);

        GtkWidget* bg_black_item = gtk_menu_item_new_with_label("Background: Black");
        g_signal_connect(bg_black_item, "activate",
                         G_CALLBACK(on_bg_black), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu), bg_black_item);

        GtkWidget* bg_white_item = gtk_menu_item_new_with_label("Background: White");
        g_signal_connect(bg_white_item, "activate",
                         G_CALLBACK(on_bg_white), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu), bg_white_item);

        GtkWidget* color_item = gtk_menu_item_new_with_label("Choose Drawing Color...");
        g_signal_connect(color_item, "activate",
                         G_CALLBACK(on_choose_color), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu), color_item);

        // Separator line
        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu),
                              gtk_separator_menu_item_new());

        // Cursor sub-menu (nested inside Preferences)
        GtkWidget* cursor_menu = gtk_menu_new();
        GtkWidget* cursor_item = gtk_menu_item_new_with_label("Mouse Cursor");
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(cursor_item), cursor_menu);

            GtkWidget* arrow_item = gtk_menu_item_new_with_label("Arrow");
            g_signal_connect(arrow_item, "activate",
                             G_CALLBACK(on_cursor_arrow), nullptr);
            gtk_menu_shell_append(GTK_MENU_SHELL(cursor_menu), arrow_item);

            GtkWidget* cross_item = gtk_menu_item_new_with_label("Crosshair");
            g_signal_connect(cross_item, "activate",
                             G_CALLBACK(on_cursor_cross), nullptr);
            gtk_menu_shell_append(GTK_MENU_SHELL(cursor_menu), cross_item);

            GtkWidget* hand_item = gtk_menu_item_new_with_label("Hand");
            g_signal_connect(hand_item, "activate",
                             G_CALLBACK(on_cursor_hand), nullptr);
            gtk_menu_shell_append(GTK_MENU_SHELL(cursor_menu), hand_item);

        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu), cursor_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), pref_item);

    // ====================================================================
    // LINES MENU
    // ====================================================================
    GtkWidget* lines_menu = gtk_menu_new();
    GtkWidget* lines_item = gtk_menu_item_new_with_label("Lines");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(lines_item), lines_menu);

        GtkWidget* dda_item = gtk_menu_item_new_with_label("DDA");
        g_signal_connect(dda_item, "activate",
                         G_CALLBACK(on_lines_dda), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(lines_menu), dda_item);

        GtkWidget* mp_item = gtk_menu_item_new_with_label("Midpoint (Bresenham)");
        g_signal_connect(mp_item, "activate",
                         G_CALLBACK(on_lines_midpoint), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(lines_menu), mp_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), lines_item);

    // ====================================================================
    // CIRCLES MENU
    // ====================================================================
    GtkWidget* circles_menu = gtk_menu_new();
    GtkWidget* circles_item = gtk_menu_item_new_with_label("Circles");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(circles_item), circles_menu);

        GtkWidget* mm_item = gtk_menu_item_new_with_label("Modified Midpoint");
        g_signal_connect(mm_item, "activate",
                         G_CALLBACK(on_circles_modified_midpoint), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(circles_menu), mm_item);

        GtkWidget* direct_item = gtk_menu_item_new_with_label("Direct Equation");
        g_signal_connect(direct_item, "activate",
                         G_CALLBACK(on_circles_direct), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(circles_menu), direct_item);

        GtkWidget* bres_item = gtk_menu_item_new_with_label("Midpoint (Bresenham)");
        g_signal_connect(bres_item, "activate",
                         G_CALLBACK(on_circles_bresenham), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(circles_menu), bres_item);

        GtkWidget* polar_item = gtk_menu_item_new_with_label("Polar Circle");
        g_signal_connect(polar_item, "activate",
                         G_CALLBACK(on_circles_polar), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(circles_menu), polar_item);

        GtkWidget* ipolar_item = gtk_menu_item_new_with_label("Iterative Polar Circle");
        g_signal_connect(ipolar_item, "activate",
                         G_CALLBACK(on_circles_iterative_polar), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(circles_menu), ipolar_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), circles_item);

    // ====================================================================
    // ELLIPSES MENU
    // ====================================================================
    GtkWidget* ellipse_menu = gtk_menu_new();
    GtkWidget* ellipse_item = gtk_menu_item_new_with_label("Ellipses");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(ellipse_item), ellipse_menu);

        GtkWidget* ell_mid_item = gtk_menu_item_new_with_label("Midpoint Ellipse");
        g_signal_connect(ell_mid_item, "activate",
                         G_CALLBACK(on_ellipse_midpoint), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(ellipse_menu), ell_mid_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), ellipse_item);

    // ====================================================================
    // FILLING MENU
    // ====================================================================
    GtkWidget* fill_menu = gtk_menu_new();
    GtkWidget* fill_item = gtk_menu_item_new_with_label("Filling");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fill_item), fill_menu);

        GtkWidget* fill_circ_item = gtk_menu_item_new_with_label("Fill Circles with Lines");
        g_signal_connect(fill_circ_item, "activate",
                         G_CALLBACK(on_fill_circles_lines), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(fill_menu), fill_circ_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), fill_item);

    // ====================================================================
    // CLIPPING MENU
    // ====================================================================
    GtkWidget* clip_menu = gtk_menu_new();
    GtkWidget* clip_item = gtk_menu_item_new_with_label("Clipping");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(clip_item), clip_menu);

        GtkWidget* cp_item = gtk_menu_item_new_with_label("Rectangular Point Clipping");
        g_signal_connect(cp_item, "activate",
                         G_CALLBACK(on_clip_point), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(clip_menu), cp_item);

        GtkWidget* cl_item = gtk_menu_item_new_with_label("Rectangular Line Clipping");
        g_signal_connect(cl_item, "activate",
                         G_CALLBACK(on_clip_line), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(clip_menu), cl_item);

        GtkWidget* cpoly_item = gtk_menu_item_new_with_label("Rectangular Polygon Clipping");
        g_signal_connect(cpoly_item, "activate",
                         G_CALLBACK(on_clip_polygon), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(clip_menu), cpoly_item);

        GtkWidget* ccp_item = gtk_menu_item_new_with_label("Circular Point Clipping");
        g_signal_connect(ccp_item, "activate",
                         G_CALLBACK(on_clip_circle_point), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(clip_menu), ccp_item);

        GtkWidget* ccl_item = gtk_menu_item_new_with_label("Circular Line Clipping");
        g_signal_connect(ccl_item, "activate",
                         G_CALLBACK(on_clip_circle_line), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(clip_menu), ccl_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), clip_item);

    return bar;  // Return the fully constructed menu bar
}

// ============================================================================
// MAIN FUNCTION (Application Entry Point)
// ============================================================================
/**
 * main: Application entry point.
 *
 * Steps:
 *   1. Initialize GTK (parses command-line flags GTK understands)
 *   2. Create the main window
 *   3. Create a vertical box layout (vbox) to hold: menu bar + drawing area
 *   4. Build the menu bar and add it to the top
 *   5. Create the drawing area (GtkDrawingArea) — this is where we render
 *   6. Enable button-press events on the drawing area
 *   7. Connect the "draw" signal to on_draw (main rendering callback)
 *   8. Connect the "button-press-event" signal to on_button_press
 *   9. Show all widgets and enter the GTK main event loop
 *   10. Return when gtk_main_quit() is called (window closed)
 *
 * @param argc  Command-line argument count
 * @param argv  Command-line argument values
 * @return 0 on successful exit
 */
int main(int argc, char* argv[]) {
    // Step 1: Initialize GTK (must be called before any GTK functions)
    gtk_init(&argc, &argv);

    // Step 2: Create the main application window
    g_mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(g_mainWindow),
                         "Computer Graphics Project - Person 3");
    gtk_window_set_default_size(GTK_WINDOW(g_mainWindow), 800, 600);

    // Connect "destroy" signal so the application quits when window is closed
    g_signal_connect(g_mainWindow, "destroy",
                     G_CALLBACK(gtk_main_quit), nullptr);

    // Step 3: Create a vertical box to organize widgets top-to-bottom
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(g_mainWindow), vbox);

    // Step 4: Add the menu bar at the top of the vbox
    GtkWidget* menu_bar = build_menu_bar();
    // Parameters: container, child, expand, fill, padding
    // expand=FALSE → menu bar takes only its minimum height
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    // Step 5: Add a Clear button below the menu bar
    GtkWidget* clear_button = gtk_button_new_with_label("Clear");
    // g_signal_connect(clear_button, "clicked",
    //                  G_CALLBACK(on_clear_button), nullptr);
    // gtk_box_pack_start(GTK_BOX(vbox), clear_button, FALSE, FALSE, 2);

    // Step 6: Create the drawing area (fills remaining space in vbox)
    g_drawingArea = gtk_drawing_area_new();
    gtk_widget_set_hexpand(g_drawingArea, TRUE);   // Expand horizontally
    gtk_widget_set_vexpand(g_drawingArea, TRUE);   // Expand vertically
    // expand=TRUE, fill=TRUE → drawing area fills all remaining space
    gtk_box_pack_start(GTK_BOX(vbox), g_drawingArea, TRUE, TRUE, 0);

    // Step 6: Enable mouse click events on the drawing area
    // Without this, button-press-event won't fire
    gtk_widget_add_events(g_drawingArea,
                          GDK_BUTTON_PRESS_MASK);

    // Step 7: Connect the "draw" signal (replaces WM_PAINT in Windows)
    // This is called whenever the widget needs to repaint itself
    g_signal_connect(g_drawingArea, "draw",
                     G_CALLBACK(on_draw), nullptr);

    // Step 8: Connect mouse click handler
    g_signal_connect(g_drawingArea, "button-press-event",
                     G_CALLBACK(on_button_press), nullptr);

    // Step 9: Print instructions and start the application
    cout << "=== Computer Graphics Project - Person 3 ===" << endl;
    cout << "Select a drawing mode from the menu, then click points." << endl;
    cout << endl;

    // Show all widgets (maps the window and makes it visible)
    gtk_widget_show_all(g_mainWindow);

    // Enter the GTK main event loop
    // This blocks until gtk_main_quit() is called (e.g., window closed)
    gtk_main();

    return 0;  // Exit successfully
}
