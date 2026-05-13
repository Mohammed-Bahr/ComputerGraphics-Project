/*
 * Computer Graphics Term Project - Person 3 Implementation
 * Linux/GNOME Port using GTK3 + Cairo
 *
 * Full implementation including:
 * - DDA Line, Midpoint Line
 * - Modified Midpoint Circle, Direct Circle, Bresenham Circle,
 *   Polar Circle, Iterative Polar Circle
 * - Midpoint Ellipse
 * - Fill Circle with Lines
 * - Rectangular Point, Line (Cohen-Sutherland), Polygon (Sutherland-Hodgman) Clipping
 * - Circular Point and Line Clipping
 * - Background color, Drawing color, Cursor changes
 *
 * Build:
 *   g++ LinuxMain.cpp -o cg_project $(pkg-config --cflags --libs gtk+-3.0) -lm -std=c++11
 *
 * All drawing uses mouse-only input (no keyboard).
 * All rendering happens inside the "draw" signal handler only.
 */

#include <gtk/gtk.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

// ============================================================================
// COLOR STRUCTURE
// ============================================================================
struct Color {
    double r, g, b;
};

// ============================================================================
// POINT STRUCTURE  (replaces Windows POINT for polygon clipping)
// ============================================================================
struct Point2D {
    int x, y;
    Point2D(int a = 0, int b = 0) : x(a), y(b) {}
};

// ============================================================================
// DRAW MODE
// ============================================================================
enum DrawMode {
    MODE_NONE,
    MODE_DDA_LINE,
    MODE_MIDPOINT_LINE,
    MODE_MODIFIED_MIDPOINT_CIRCLE,
    MODE_DIRECT_CIRCLE,
    MODE_BRESENHAM_CIRCLE,
    MODE_POLAR_CIRCLE,
    MODE_ITERATIVE_POLAR_CIRCLE,
    MODE_MIDPOINT_ELLIPSE,
    MODE_FILL_CIRCLE_WITH_LINES,
    MODE_CLIP_POINT,
    MODE_CLIP_LINE,
    MODE_CLIP_POLYGON,
    MODE_CLIP_CIRCLE_POINT,
    MODE_CLIP_CIRCLE_LINE,
};

// ============================================================================
// SHAPE STRUCTURE  (stores each drawn shape for redraws)
// ============================================================================
struct Shape {
    DrawMode mode;
    Color    color;
    int      x1, y1, x2, y2;
};

// ============================================================================
// GLOBAL STATE
// ============================================================================
Color    g_backgroundColor = {0.0, 0.0, 0.0};   // Default: black
Color    g_drawColor       = {1.0, 1.0, 1.0};   // Default: white
DrawMode g_currentMode     = MODE_NONE;

bool g_firstClick  = false;
bool g_secondClick = false;
int  g_startX = 0, g_startY = 0;
int  g_X1 = 0, g_Y1 = 0;

vector<Shape>   g_shapes;
vector<Point2D> g_polygonPoints;
vector<Point2D> g_polygonResult;   // Clipped polygon vertices (redrawn in on_draw)
bool             g_polygonClipped = false;  // true when right-click finishes clipping

// Clip window boundaries
int g_clipX1 = 150, g_clipY1 = 150, g_clipX2 = 600, g_clipY2 = 400;

GtkWidget* g_drawingArea = nullptr;
GtkWidget* g_mainWindow  = nullptr;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static inline int Round(double x) {
    return (int)(x + 0.5);
}

static void swap_points(int& a, int& b, int& c, int& d) {
    int t;
    t = a; a = c; c = t;
    t = b; b = d; d = t;
}

// Set a single pixel using Cairo (1x1 filled rectangle)
static void SetPixel(cairo_t* cr, int x, int y, Color color) {
    cairo_set_source_rgb(cr, color.r, color.g, color.b);
    cairo_rectangle(cr, x, y, 1.0, 1.0);
    cairo_fill(cr);
}

// Check if point (x,y) lies within the circle (xc,yc,R)
static bool CheckIFPointINCircle(int R, int xc, int yc, int x, int y) {
    return sqrt(pow((double)(x - xc), 2) + pow((double)(y - yc), 2)) <= R;
}

// Find an existing circle in g_shapes that contains point (x,y)
// Returns true and sets xc,yc,R if found
static bool FindCircle(int x, int y, int& xc, int& yc, int& R) {
    for (auto& s : g_shapes) {
        if (s.mode == MODE_DIRECT_CIRCLE ||
            s.mode == MODE_BRESENHAM_CIRCLE ||
            s.mode == MODE_MODIFIED_MIDPOINT_CIRCLE ||
            s.mode == MODE_POLAR_CIRCLE ||
            s.mode == MODE_ITERATIVE_POLAR_CIRCLE)
        {
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
// DRAW SYMMETRIC POINTS
// ============================================================================

static void Draw8Points(cairo_t* cr, int xc, int yc, int x, int y, Color color) {
    SetPixel(cr, xc + x, yc + y, color);
    SetPixel(cr, xc - x, yc + y, color);
    SetPixel(cr, xc - x, yc - y, color);
    SetPixel(cr, xc + x, yc - y, color);
    SetPixel(cr, xc + y, yc + x, color);
    SetPixel(cr, xc - y, yc + x, color);
    SetPixel(cr, xc - y, yc - x, color);
    SetPixel(cr, xc + y, yc - x, color);
}

static void Draw4Points(cairo_t* cr, int xc, int yc, int x, int y, Color color) {
    SetPixel(cr, xc + x, yc + y, color);
    SetPixel(cr, xc - x, yc + y, color);
    SetPixel(cr, xc + x, yc - y, color);
    SetPixel(cr, xc - x, yc - y, color);
}

// ============================================================================
// ALGORITHM 1: DDA LINE
// ============================================================================
static void DrawLineDDA(cairo_t* cr,
                        int x1, int y1, int x2, int y2, Color c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (abs(dy) <= abs(dx)) {
        double m = (dx != 0) ? (double)dy / dx : 0.0;
        if (x1 > x2) swap_points(x1, y1, x2, y2);
        SetPixel(cr, x1, y1, c);
        int    x = x1;
        double y = y1;
        while (x < x2) {
            x++;
            y += m;
            SetPixel(cr, x, Round(y), c);
        }
    } else {
        double mi = (dy != 0) ? (double)dx / dy : 0.0;
        if (y1 > y2) swap_points(x1, y1, x2, y2);
        SetPixel(cr, x1, y1, c);
        int    y = y1;
        double x = x1;
        while (y < y2) {
            y++;
            x += mi;
            SetPixel(cr, Round(x), y, c);
        }
    }
}

// ============================================================================
// ALGORITHM 2: MIDPOINT LINE  (Bresenham's Line)
// ============================================================================
static void DrawLineMidpoint(cairo_t* cr,
                             int x1, int y1, int x2, int y2, Color c) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int x  = x1, y = y1;

    if (dx >= dy) {
        int d       = dx - 2 * dy;
        int delta1  = 2 * dx - 2 * dy;
        int delta2  = -2 * dy;
        SetPixel(cr, x, y, c);
        while (x != x2) {
            if (d < 0) { y += sy; d += delta1; }
            else         {          d += delta2; }
            x += sx;
            SetPixel(cr, x, y, c);
        }
    } else {
        int d       = dy - 2 * dx;
        int delta1  = 2 * dy - 2 * dx;
        int delta2  = -2 * dx;
        SetPixel(cr, x, y, c);
        while (y != y2) {
            if (d < 0) { x += sx; d += delta1; }
            else         {          d += delta2; }
            y += sy;
            SetPixel(cr, x, y, c);
        }
    }
}

// ============================================================================
// ALGORITHM 3: MODIFIED MIDPOINT CIRCLE  (Faster Bresenham variant)
// ============================================================================
static void DrawCircleModifiedMidpoint(cairo_t* cr,
                                       int xc, int yc, int R, Color color) {
    if (R <= 0) return;
    int x  = 0, y = R;
    int d  = 1 - R;
    int c1 = 3;
    int c2 = 5 - 2 * R;

    Draw8Points(cr, xc, yc, x, y, color);

    while (x < y) {
        if (d < 0) {
            d  += c1;
            c2 += 2;
        } else {
            d  += c2;
            c2 += 4;
            y--;
        }
        c1 += 2;
        x++;
        Draw8Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 4: DIRECT CIRCLE  (x^2 + y^2 = R^2)
// ============================================================================
static void DrawCircleDirect(cairo_t* cr,
                             int xc, int yc, int R, Color color) {
    if (R <= 0) return;
    int limit = (int)(R / sqrt(2.0)) + 1;
    for (int x = 0; x <= limit; x++) {
        int y = (int)round(sqrt((double)R * R - (double)x * x));
        Draw8Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 5: BRESENHAM CIRCLE  (Midpoint Circle)
// ============================================================================
static void DrawCircleBresenham(cairo_t* cr,
                                int xc, int yc, int R, Color color) {
    if (R <= 0) return;
    int x = 0, y = R, d = 1 - R;
    Draw8Points(cr, xc, yc, x, y, color);
    while (x < y) {
        if (d < 0)
            d += 2 * x + 3;
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        Draw8Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 6: POLAR CIRCLE  (trigonometric)
// ============================================================================
static void DrawCirclePolar(cairo_t* cr,
                            int xc, int yc, int R, Color color) {
    if (R <= 0) return;
    int x = R, y = 0;
    double theta = 0, dtheta = 1.0 / R;
    Draw8Points(cr, xc, yc, x, y, color);
    while (x > y) {
        theta += dtheta;
        x = Round(R * cos(theta));
        y = Round(R * sin(theta));
        Draw8Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 7: ITERATIVE POLAR CIRCLE  (using rotation matrix)
// ============================================================================
static void DrawCircleIterativePolar(cairo_t* cr,
                                     int xc, int yc, int R, Color color) {
    if (R <= 0) return;
    double x = R, y = 0;
    double dtheta = 1.0 / R;
    double cdtheta = cos(dtheta), sdtheta = sin(dtheta);
    Draw8Points(cr, xc, yc, (int)x, (int)y, color);
    while (x > y) {
        double x1 = x * cdtheta - y * sdtheta;
        y = x * sdtheta + y * cdtheta;
        x = x1;
        Draw8Points(cr, xc, yc, Round(x), Round(y), color);
    }
}

// ============================================================================
// ALGORITHM 8: MIDPOINT ELLIPSE
// ============================================================================
static void DrawEllipseMidpoint(cairo_t* cr,
                                int xc, int yc, int a, int b, Color color) {
    if (a <= 0 || b <= 0) return;
    int x = 0, y = b;
    double d = 0.25 * a * a + b * b - a * a * b;
    double dx = 2.0 * b * b * x;
    double dy = 2.0 * a * a * y;

    Draw4Points(cr, xc, yc, x, y, color);
    while (dx < dy) {
        x++;
        dx += 2 * b * b;
        if (d < 0) {
            d += dx + b * b;
        } else {
            dy -= 2 * a * a;
            d += dx - dy + b * b;
            y--;
        }
        Draw4Points(cr, xc, yc, x, y, color);
    }

    d = (double)b * b * (x + 0.5) * (x + 0.5)
      + (double)a * a * (y - 1) * (y - 1)
      - (double)a * a * b * b;
    Draw4Points(cr, xc, yc, x, y, color);
    while (y >= 0) {
        y--;
        dy -= 2 * a * a;
        if (d > 0) {
            d += -dy + a * a;
        } else {
            dx += 2 * b * b;
            d += dx - dy + a * a;
            x++;
        }
        Draw4Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 9: FILL CIRCLE WITH LINES
// ============================================================================
// static void FillCircleWithLines(cairo_t* cr,
//                                 int xc, int yc, int R,
//                                 double thetaS, double thetaE, Color color) {
//     if (thetaS == thetaE) return;
//     for (double theta = thetaS; theta <= thetaE; theta += 0.001) {
//         int x = xc + (int)(R * cos(theta));
//         int y = yc + (int)(R * sin(theta));
//         DrawLineMidpoint(cr, xc, yc, x, y, color);
//     }
// }
//
static void FillCircleWithLines(cairo_t* cr, int xc, int yc, int R, double thetaS, double thetaE, Color color) {
    if (R <= 0) return;

    double dtheta = 1.0 / R; // Optimal step size
    double ct = cos(dtheta), st = sin(dtheta); // Precompute rotation factors

    // Starting point
    double x_rel = R * cos(thetaS);
    double y_rel = R * sin(thetaS);

    int steps = (int)((thetaE - thetaS) / dtheta);
    for (int i = 0; i <= steps; i++) {
        DrawLineMidpoint(cr, xc, yc, xc + Round(x_rel), yc + Round(y_rel), color);

        // Rotate point by dtheta using rotation matrix
        double next_x = x_rel * ct - y_rel * st;
        y_rel = x_rel * st + y_rel * ct;
        x_rel = next_x;
    }
}

// ============================================================================
// CLIP WINDOW DRAWING
// ============================================================================
static void DrawClipWindow(cairo_t* cr) {
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0); // Yellow
    const double dashes[] = {4.0, 4.0};
    cairo_set_dash(cr, dashes, 2, 0.0);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, g_clipX1, g_clipY1,
                        g_clipX2 - g_clipX1, g_clipY2 - g_clipY1);
    cairo_stroke(cr);
    cairo_set_dash(cr, nullptr, 0, 0.0); // Reset dash
}

// ============================================================================
// POINT CLIPPING
// ============================================================================
static void PointClipping(cairo_t* cr, int x, int y,
                          int xl, int yt, int xr, int yb, Color color) {
    if (x >= xl && x <= xr && y >= yt && y <= yb) {
        // Draw a small cross (5x5 block)
        for (int dx = -2; dx <= 2; dx++)
            for (int dy = -2; dy <= 2; dy++)
                SetPixel(cr, x + dx, y + dy, color);
        cout << "  Point INSIDE - visible" << endl;
    } else {
        cout << "  Point OUTSIDE - clipped" << endl;
    }
}

// ============================================================================
// LINE CLIPPING - Cohen-Sutherland
// ============================================================================
union OutCode {
    unsigned All : 4;
    struct { unsigned left : 1, top : 1, right : 1, bottom : 1; };
};

static OutCode GetOutCode(double x, double y,
                          int xl, int yt, int xr, int yb) {
    OutCode out;
    out.All = 0;
    if (x < xl)          out.left   = 1;
    else if (x > xr)     out.right  = 1;
    if (y < yt)          out.top    = 1;
    else if (y > yb)     out.bottom = 1;
    return out;
}

static void VIntersect(double xs, double ys, double xe, double ye,
                       int x, double* xi, double* yi) {
    *xi = x;
    *yi = ys + (x - xs) * (ye - ys) / (xe - xs);
}

static void HIntersect(double xs, double ys, double xe, double ye,
                       int y, double* xi, double* yi) {
    *yi = y;
    *xi = xs + (y - ys) * (xe - xs) / (ye - ys);
}

static void CohenSuth(cairo_t* cr,
                      int xs, int ys, int xe, int ye,
                      int xl, int yt, int xr, int yb, Color color) {
    double x1 = xs, y1 = ys, x2 = xe, y2 = ye;
    OutCode o1 = GetOutCode(x1, y1, xl, yt, xr, yb);
    OutCode o2 = GetOutCode(x2, y2, xl, yt, xr, yb);

    while ((o1.All || o2.All) && !(o1.All & o2.All)) {
        double xi, yi;
        if (o1.All) {
            if (o1.left)        VIntersect(x1, y1, x2, y2, xl, &xi, &yi);
            else if (o1.top)    HIntersect(x1, y1, x2, y2, yt, &xi, &yi);
            else if (o1.right)  VIntersect(x1, y1, x2, y2, xr, &xi, &yi);
            else                HIntersect(x1, y1, x2, y2, yb, &xi, &yi);
            x1 = xi; y1 = yi;
            o1 = GetOutCode(x1, y1, xl, yt, xr, yb);
        } else {
            if (o2.left)        VIntersect(x1, y1, x2, y2, xl, &xi, &yi);
            else if (o2.top)    HIntersect(x1, y1, x2, y2, yt, &xi, &yi);
            else if (o2.right)  VIntersect(x1, y1, x2, y2, xr, &xi, &yi);
            else                HIntersect(x1, y1, x2, y2, yb, &xi, &yi);
            x2 = xi; y2 = yi;
            o2 = GetOutCode(x2, y2, xl, yt, xr, yb);
        }
    }

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
// POLYGON CLIPPING - Sutherland-Hodgman
// ============================================================================
struct Vertex {
    double x, y;
    Vertex(double a = 0, double b = 0) : x(a), y(b) {}
};
typedef vector<Vertex> VList;
typedef bool    (*IsInFn)(Vertex&, int);
typedef Vertex (*IsectFn)(Vertex&, Vertex&, int);

static bool InLeft(Vertex& v, int e)   { return v.x >= e; }
static bool InRight(Vertex& v, int e)  { return v.x <= e; }
static bool InTop(Vertex& v, int e)    { return v.y >= e; }
static bool InBottom(Vertex& v, int e) { return v.y <= e; }

static Vertex VIsectPoly(Vertex& v1, Vertex& v2, int xe) {
    return Vertex(xe,
                  v1.y + (xe - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
}

static Vertex HIsectPoly(Vertex& v1, Vertex& v2, int ye) {
    return Vertex(v1.x + (ye - v1.y) * (v2.x - v1.x) / (v2.y - v1.y),
                  ye);
}

static VList ClipEdge(VList p, int edge, IsInFn In, IsectFn Isect) {
    VList out;
    if (p.empty()) return out;
    Vertex v1 = p.back();
    bool v1in = In(v1, edge);
    for (auto& v2 : p) {
        bool v2in = In(v2, edge);
        if (!v1in && v2in) {
            out.push_back(Isect(v1, v2, edge));
            out.push_back(v2);
        } else if (v1in && v2in) {
            out.push_back(v2);
        } else if (v1in) {
            out.push_back(Isect(v1, v2, edge));
        }
        v1 = v2;
        v1in = v2in;
    }
    return out;
}

static void PolygonClip(cairo_t* cr, vector<Point2D>& pts,
                        int xl, int yt, int xr, int yb, Color color) {
    VList vl;
    for (auto& p : pts)
        vl.push_back(Vertex(p.x, p.y));

    vl = ClipEdge(vl, xl, InLeft,   VIsectPoly);
    vl = ClipEdge(vl, yt, InTop,    HIsectPoly);
    vl = ClipEdge(vl, xr, InRight,  VIsectPoly);
    vl = ClipEdge(vl, yb, InBottom, HIsectPoly);

    if (vl.size() < 2) {
        cout << "  Polygon completely outside." << endl;
        return;
    }

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
// POINT CIRCLE CLIPPING
// ============================================================================
static void PointCircleClip(cairo_t* cr, int xc, int yc,
                            int x, int y, int R, Color color) {
    if (CheckIFPointINCircle(R, xc, yc, x, y)) {
        cairo_set_source_rgb(cr, color.r, color.g, color.b);
        for (int dy = -R; dy < R; dy++) {
            for (int dx = -R; dx < R; dx++) {
                SetPixel(cr, xc + dx, yc + dy, color);
            }
        }
    }
}

// ============================================================================
// LINE CIRCLE CLIPPING
// ============================================================================
static void LineCircleClip(cairo_t* cr, int xc, int yc,
                           int x1, int y1, int x2, int y2,
                           int R, Color color) {
    int dx = x2 - x1, dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));
    double xInc = (steps > 0) ? (double)dx / steps : 0;
    double yInc = (steps > 0) ? (double)dy / steps : 0;
    double x = x1, y = y1;

    for (int i = 0; i <= steps; i++) {
        if (CheckIFPointINCircle(R, xc, yc, Round(x), Round(y))) {
            SetPixel(cr, Round(x), Round(y), color);
        }
        x += xInc;
        y += yInc;
    }
}

// ============================================================================
// SHAPE RENDERING (for redraw in on_draw)
// ============================================================================
static void RenderShape(cairo_t* cr, const Shape& s) {
    switch (s.mode) {
    case MODE_DDA_LINE:
        DrawLineDDA(cr, s.x1, s.y1, s.x2, s.y2, s.color);
        break;
    case MODE_MIDPOINT_LINE:
        DrawLineMidpoint(cr, s.x1, s.y1, s.x2, s.y2, s.color);
        break;
    case MODE_MODIFIED_MIDPOINT_CIRCLE:
    case MODE_DIRECT_CIRCLE:
    case MODE_BRESENHAM_CIRCLE:
    case MODE_POLAR_CIRCLE:
    case MODE_ITERATIVE_POLAR_CIRCLE: {
        int dx = s.x2 - s.x1, dy = s.y2 - s.y1;
        int R = (int)sqrt(dx * dx + dy * dy);
        if (R <= 0) break;
        switch (s.mode) {
        case MODE_MODIFIED_MIDPOINT_CIRCLE:
            DrawCircleModifiedMidpoint(cr, s.x1, s.y1, R, s.color); break;
        case MODE_DIRECT_CIRCLE:
            DrawCircleDirect(cr, s.x1, s.y1, R, s.color); break;
        case MODE_BRESENHAM_CIRCLE:
            DrawCircleBresenham(cr, s.x1, s.y1, R, s.color); break;
        case MODE_POLAR_CIRCLE:
            DrawCirclePolar(cr, s.x1, s.y1, R, s.color); break;
        case MODE_ITERATIVE_POLAR_CIRCLE:
            DrawCircleIterativePolar(cr, s.x1, s.y1, R, s.color); break;
        default: break;
        }
        break;
    }
    case MODE_MIDPOINT_ELLIPSE: {
        int a = abs(s.x2 - s.x1);
        int b = abs(s.y2 - s.y1);
        DrawEllipseMidpoint(cr, s.x1, s.y1, a, b, s.color);
        break;
    }
    case MODE_FILL_CIRCLE_WITH_LINES: {
        // s.x1,s.y1 = circle center; s.x2,s.y2 = click point for quadrant
        int dx = s.x2 - s.x1, dy = s.y2 - s.y1;
        int R = (int)sqrt(dx * dx + dy * dy);
        if (R <= 0) break;
        double angle = atan2((double)(s.y2 - s.y1), (double)(s.x2 - s.x1));
        const double PI = 3.14159265;
        double thetaS, thetaE;
        if (angle >= 0 && angle < PI / 2) {
            thetaS = 0; thetaE = PI / 2;
        } else if (angle >= PI / 2 && angle < PI) {
            thetaS = PI / 2; thetaE = PI;
        } else if (angle >= -PI && angle < -PI / 2) {
            thetaS = PI; thetaE = 3 * PI / 2;
        } else {
            thetaS = 3 * PI / 2; thetaE = 2 * PI;
        }
        FillCircleWithLines(cr, s.x1, s.y1, R, thetaS, thetaE, s.color);
        break;
    }
    case MODE_CLIP_POINT:
        DrawClipWindow(cr);
        PointClipping(cr, s.x1, s.y1,
                      g_clipX1, g_clipY1, g_clipX2, g_clipY2, s.color);
        break;
    case MODE_CLIP_LINE:
        DrawClipWindow(cr);
        CohenSuth(cr, s.x1, s.y1, s.x2, s.y2,
                  g_clipX1, g_clipY1, g_clipX2, g_clipY2, s.color);
        break;
    case MODE_CLIP_CIRCLE_POINT: {
        int xc = s.x2, yc = s.y2, R = s.x1;
        // Redraw: show the point was inside circle by filling the circle
        PointCircleClip(cr, xc, yc, s.x1, s.y1, R, s.color);
        break;
    }
    case MODE_CLIP_CIRCLE_LINE: {
        // s.x1,s.y1 = line start; s.x2,s.y2 = line end
        // Find the first circle and clip the line to it
        int xc, yc, R;
        if (FindCircle(s.x2, s.y2, xc, yc, R)) {
            LineCircleClip(cr, xc, yc, s.x1, s.y1, s.x2, s.y2, R, s.color);
        }
        break;
    }
    default:
        break;
    }
}

// ============================================================================
// GTK SIGNAL CALLBACKS
// ============================================================================

// "draw" signal - replaces WM_PAINT.
// ALL rendering happens here. Never draw outside this callback.
static gboolean on_draw(GtkWidget* /*widget*/, cairo_t* cr,
                        gpointer /*data*/) {
    // Fill background
    cairo_set_source_rgb(cr, g_backgroundColor.r,
                             g_backgroundColor.g,
                             g_backgroundColor.b);
    cairo_paint(cr);

    // Redraw all stored shapes
    for (const auto& shape : g_shapes)
        RenderShape(cr, shape);

    // If a clipping mode is active, show the clip window overlay
    if (g_currentMode == MODE_CLIP_POINT ||
        g_currentMode == MODE_CLIP_LINE ||
        g_currentMode == MODE_CLIP_POLYGON) {
        DrawClipWindow(cr);
    }
    // Also show clip window if there are clipped polygon results
    if (g_polygonClipped) {
        DrawClipWindow(cr);
    }

    // If collecting polygon points, draw small markers for each
    if (g_currentMode == MODE_CLIP_POLYGON && !g_polygonPoints.empty()) {
        cairo_set_source_rgb(cr, g_drawColor.r, g_drawColor.g, g_drawColor.b);
        for (auto& pt : g_polygonPoints) {
            for (int d = -2; d <= 2; d++) {
                SetPixel(cr, pt.x + d, pt.y, g_drawColor);
                SetPixel(cr, pt.x, pt.y + d, g_drawColor);
            }
        }
    }

    // Draw clipped polygon result if available
    if (g_polygonClipped && g_polygonResult.size() >= 2) {
        cairo_set_source_rgb(cr, g_drawColor.r, g_drawColor.g, g_drawColor.b);
        cairo_set_line_width(cr, 1.0);
        size_t n = g_polygonResult.size();
        for (size_t i = 0; i < n; i++) {
            size_t j = (i + 1) % n;
            cairo_move_to(cr, g_polygonResult[i].x, g_polygonResult[i].y);
            cairo_line_to(cr, g_polygonResult[j].x, g_polygonResult[j].y);
            cairo_stroke(cr);
        }
    }

    return FALSE;
}

// "button-press-event" - replaces WM_LBUTTONDOWN / WM_RBUTTONDOWN.
// Never draws directly - just updates state and calls gtk_widget_queue_draw.
static gboolean on_button_press(GtkWidget* widget,
                                GdkEventButton* event,
                                gpointer /*data*/) {
    int x = (int)event->x;
    int y = (int)event->y;

    // ----- Right-click: finish polygon clipping -----
    if (event->button == GDK_BUTTON_SECONDARY) {
        if (g_currentMode == MODE_CLIP_POLYGON && g_polygonPoints.size() >= 3) {
            // Run Sutherland-Hodgman algorithm to clip the polygon
            // We need to convert g_polygonPoints to Vertex list for PolygonClip
            // First store the original polygon so it persists
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

            // Compute the clipped polygon vertices
            vector<Point2D> clipped;
            {
                // Use the Vertex-based PolygonClip to compute clipped vertices
                vector<Vertex> vl;
                for (auto& p : g_polygonPoints)
                    vl.push_back(Vertex(p.x, p.y));

                vl = ClipEdge(vl, g_clipX1, InLeft,   VIsectPoly);
                vl = ClipEdge(vl, g_clipY1, InTop,    HIsectPoly);
                vl = ClipEdge(vl, g_clipX2, InRight,  VIsectPoly);
                vl = ClipEdge(vl, g_clipY2, InBottom, HIsectPoly);

                for (auto& v : vl)
                    clipped.push_back(Point2D(Round(v.x), Round(v.y)));

                cout << "  Polygon clipped with " << vl.size() << " vertices." << endl;
            }

            if (clipped.size() >= 2) {
                g_polygonResult = clipped;
                g_polygonClipped = true;
            }

            g_polygonPoints.clear();
            gtk_widget_queue_draw(widget);
        }
        return TRUE;
    }

    // ----- Left-click only from here -----
    if (event->button != GDK_BUTTON_PRIMARY) return FALSE;

    if (g_currentMode == MODE_NONE) {
        cout << "Please select a drawing mode from the menu first." << endl;
        return TRUE;
    }

    // --- Point Clipping (single click) ---
    if (g_currentMode == MODE_CLIP_POINT) {
        Shape s;
        s.mode  = MODE_CLIP_POINT;
        s.color = g_drawColor;
        s.x1    = x; s.y1 = y;
        s.x2    = 0; s.y2 = 0;
        g_shapes.push_back(s);
        gtk_widget_queue_draw(widget);
        return TRUE;
    }

    // --- Polygon Clipping: collect points ---
    if (g_currentMode == MODE_CLIP_POLYGON) {
        g_polygonPoints.push_back(Point2D(x, y));
        gtk_widget_queue_draw(widget);
        cout << "  Point added (" << x << "," << y << ")" << endl;
        return TRUE;
    }

    // --- Circle Point Clipping (single click on existing circle) ---
    if (g_currentMode == MODE_CLIP_CIRCLE_POINT) {
        int xc, yc, R;
        if (FindCircle(x, y, xc, yc, R)) {
            Shape s;
            s.mode  = MODE_CLIP_CIRCLE_POINT;
            s.color = g_drawColor;
            s.x1    = R;        // store radius in x1
            s.y1    = 0;
            s.x2    = xc;       // store circle center in x2,y2
            s.y2    = yc;
            g_shapes.push_back(s);
            cout << "  Point inside circle - clipping applied." << endl;
        } else {
            cout << "  No circle found at that point." << endl;
        }
        gtk_widget_queue_draw(widget);
        return TRUE;
    }

    // --- Ellipse Mode: 3 clicks (center -> b-radius -> a-radius) ---
    if (g_currentMode == MODE_MIDPOINT_ELLIPSE) {
        if (!g_firstClick) {
            g_startX = x; g_startY = y;
            g_firstClick = true;
            cout << "  Ellipse center: (" << x << ", " << y << ")" << endl;
        } else if (!g_secondClick) {
            g_X1 = x; g_Y1 = y;
            g_secondClick = true;
            cout << "  First radius point: (" << x << ", " << y << ")" << endl;
        } else {
            int b = (int)sqrt(pow(g_X1 - g_startX, 2)
                            + pow(g_Y1 - g_startY, 2));
            int a = (int)sqrt(pow(x - g_startX, 2)
                            + pow(y - g_startY, 2));

            Shape s;
            s.mode  = MODE_MIDPOINT_ELLIPSE;
            s.color = g_drawColor;
            s.x1    = g_startX;
            s.y1    = g_startY;
            s.x2    = g_startX + a;
            s.y2    = g_startY + b;
            g_shapes.push_back(s);

            cout << "  Ellipse drawn: a=" << a << ", b=" << b << endl;
            g_firstClick = false;
            g_secondClick = false;
            gtk_widget_queue_draw(widget);
        }
        return TRUE;
    }

    // --- Fill Circle with Lines (single click on an existing circle) ---
    if (g_currentMode == MODE_FILL_CIRCLE_WITH_LINES) {
        int xc, yc, R;
        if (!FindCircle(x, y, xc, yc, R)) {
            cout << "  No circle found at that point." << endl;
            return TRUE;
        }
        // Store a shape: (xc,yc) as center, (x,y) as click point
        // R is derived from (xc,yc)-(x,y) in RenderShape
        Shape s;
        s.mode  = MODE_FILL_CIRCLE_WITH_LINES;
        s.color = g_drawColor;
        s.x1    = xc;
        s.y1    = yc;
        s.x2    = x;
        s.y2    = y;
        g_shapes.push_back(s);
        gtk_widget_queue_draw(widget);

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

    // --- Two-click shapes ---
    if (!g_firstClick) {
        g_startX    = x;
        g_startY    = y;
        g_firstClick = true;
        cout << "  First point: (" << x << ", " << y << ")" << endl;
    } else {
        g_firstClick = false;
        cout << "  Second point: (" << x << ", " << y << ")" << endl;

        Shape s;
        s.mode  = g_currentMode;
        s.color = g_drawColor;
        s.x1    = g_startX;
        s.y1    = g_startY;
        s.x2    = x;
        s.y2    = y;
        g_shapes.push_back(s);

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
                cout << "  No circle at the second point." << endl;
                // Remove the shape we just added since it's invalid
                g_shapes.pop_back();
            }
            break;
        }
        default:
            break;
        }

        gtk_widget_queue_draw(widget);
    }

    return TRUE;
}

// ============================================================================
// MENU CALLBACKS
// ============================================================================

// ---- File Menu ----
static void on_file_clear(GtkMenuItem* /*item*/, gpointer /*data*/) {
    g_shapes.clear();
    g_polygonPoints.clear();
    g_polygonResult.clear();
    g_polygonClipped = false;
    g_firstClick = false;
    g_secondClick = false;
    gtk_widget_queue_draw(g_drawingArea);
    cout << "Screen cleared." << endl;
}

// ---- Preferences - Background ----
static void on_bg_black(GtkMenuItem* /*item*/, gpointer /*data*/) {
    g_backgroundColor = {0.0, 0.0, 0.0};
    gtk_widget_queue_draw(g_drawingArea);
    cout << "Background color changed to Black." << endl;
}

static void on_bg_white(GtkMenuItem* /*item*/, gpointer /*data*/) {
    g_backgroundColor = {1.0, 1.0, 1.0};
    gtk_widget_queue_draw(g_drawingArea);
    cout << "Background color changed to White." << endl;
}

// ---- Preferences - Drawing Color ----
static void on_choose_color(GtkMenuItem* /*item*/, gpointer /*data*/) {
    GtkWidget* dialog = gtk_color_chooser_dialog_new(
        "Choose Drawing Color", GTK_WINDOW(g_mainWindow));

    GdkRGBA rgba = { g_drawColor.r, g_drawColor.g, g_drawColor.b, 1.0 };
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);
        g_drawColor = { rgba.red, rgba.green, rgba.blue };
        cout << "Drawing color changed to RGB("
             << (int)(rgba.red   * 255) << ", "
             << (int)(rgba.green * 255) << ", "
             << (int)(rgba.blue  * 255) << ")" << endl;
    }
    gtk_widget_destroy(dialog);
}

// ---- Preferences - Cursor ----
static void set_cursor_type(GdkCursorType type, const char* name) {
    GdkWindow*  win    = gtk_widget_get_window(g_drawingArea);
    GdkDisplay* disp   = gdk_display_get_default();
    GdkCursor*  cursor = gdk_cursor_new_for_display(disp, type);
    gdk_window_set_cursor(win, cursor);
    g_object_unref(cursor);
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
    gtk_widget_queue_draw(g_drawingArea);
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
    gtk_widget_queue_draw(g_drawingArea);
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
static GtkWidget* build_menu_bar() {
    GtkWidget* bar = gtk_menu_bar_new();

    // -- File --
    GtkWidget* file_menu = gtk_menu_new();
    GtkWidget* file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

        GtkWidget* clear_item = gtk_menu_item_new_with_label("Clear Screen");
        g_signal_connect(clear_item, "activate",
                         G_CALLBACK(on_file_clear), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), clear_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), file_item);

    // -- Preferences --
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

        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu),
                              gtk_separator_menu_item_new());

        // Cursor sub-menu
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

    // -- Lines --
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

    // -- Circles --
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

    // -- Ellipses --
    GtkWidget* ellipse_menu = gtk_menu_new();
    GtkWidget* ellipse_item = gtk_menu_item_new_with_label("Ellipses");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(ellipse_item), ellipse_menu);

        GtkWidget* ell_mid_item = gtk_menu_item_new_with_label("Midpoint Ellipse");
        g_signal_connect(ell_mid_item, "activate",
                         G_CALLBACK(on_ellipse_midpoint), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(ellipse_menu), ell_mid_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), ellipse_item);

    // -- Filling --
    GtkWidget* fill_menu = gtk_menu_new();
    GtkWidget* fill_item = gtk_menu_item_new_with_label("Filling");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fill_item), fill_menu);

        GtkWidget* fill_circ_item = gtk_menu_item_new_with_label("Fill Circles with Lines");
        g_signal_connect(fill_circ_item, "activate",
                         G_CALLBACK(on_fill_circles_lines), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(fill_menu), fill_circ_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), fill_item);

    // -- Clipping --
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

    return bar;
}

// ============================================================================
// MAIN
// ============================================================================
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    // -- Window --
    g_mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(g_mainWindow),
                         "Computer Graphics Project - Person 3");
    gtk_window_set_default_size(GTK_WINDOW(g_mainWindow), 800, 600);
    g_signal_connect(g_mainWindow, "destroy",
                     G_CALLBACK(gtk_main_quit), nullptr);

    // -- Layout --
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(g_mainWindow), vbox);

    // Menu bar
    GtkWidget* menu_bar = build_menu_bar();
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    // Drawing area
    g_drawingArea = gtk_drawing_area_new();
    gtk_widget_set_hexpand(g_drawingArea, TRUE);
    gtk_widget_set_vexpand(g_drawingArea, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), g_drawingArea, TRUE, TRUE, 0);

    // Enable mouse click events on the drawing area
    gtk_widget_add_events(g_drawingArea,
                          GDK_BUTTON_PRESS_MASK);

    // Connect drawing signals
    g_signal_connect(g_drawingArea, "draw",
                     G_CALLBACK(on_draw), nullptr);
    g_signal_connect(g_drawingArea, "button-press-event",
                     G_CALLBACK(on_button_press), nullptr);

    // -- Start --
    cout << "=== Computer Graphics Project - Person 3 ===" << endl;
    cout << "Select a drawing mode from the menu, then click points." << endl;
    cout << endl;

    gtk_widget_show_all(g_mainWindow);
    gtk_main();

    return 0;
}
