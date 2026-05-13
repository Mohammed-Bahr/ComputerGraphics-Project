
/*
 * Computer Graphics Term Project - Person 3 Implementation
 *
 * Responsibilities:
 * - Change background color (Preferences menu)
 * - Choose shape drawing color (Preferences menu)
 * - DDA Line algorithm
 * - Midpoint Line algorithm (Bresenham's)
 * - Modified Midpoint Circle algorithm (Faster Bresenham variant)
 * - Change mouse pointer (Preferences menu)
 *
 * All drawing uses mouse-only input (no keyboard)
 */

#include <windows.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <string>
using namespace std;
// ============================================================================
// MENU IDS
// ============================================================================
#define IDM_PREFERENCES_BG_WHITE       101
#define IDM_PREFERENCES_CHOOSE_COLOR   102
#define IDM_PREFERENCES_CURSOR_ARROW   103
#define IDM_PREFERENCES_CURSOR_CROSS   104
#define IDM_PREFERENCES_CURSOR_HAND    105
#define IDM_LINES_DDA                  201
#define IDM_LINES_MIDPOINT             202
#define IDM_LINES_PARA                 203
#define IDM_CIRCLES_MODIFIED_MIDPOINT  301
#define IDM_CIRCLES_DIRECT             302
#define IDM_CIRCLES_BRESENHAM          303
#define IDM_CIRCLES_POLAR              304
#define IDM_CIRCLES_POLAR_ITERATIVE    305
#define IDM_FILE_CLEAR                 401
#define IDM_FILE_SAVE                  402
#define IDM_FILE_LOAD                  403
#define IDM_CLIP_POINT                 501
#define IDM_CLIP_LINE                  502
#define IDM_CLIP_POLYGON               503
#define IDM_CIRCLE_CLIP_POINT          504
#define IDM_CIRCLE_CLIP_LINE           505
#define IDM_ELLIPSE_DIRECT             602
#define IDM_ELLIPSE_MIDPOINT           603
#define IDM_FILL_CIRCLES_LINES         701







// GLOBAL VARIABLES
COLORREF g_backgroundColor = RGB(0, 0, 0);      // Default black background
COLORREF g_drawColor = RGB(255, 255, 255);      // Default white drawing color

enum DrawMode {
    MODE_NONE,
    MODE_DDA_LINE,
    MODE_MIDPOINT_LINE,
    MODE_PARA_LINE,
    MODE_MODIFIED_MIDPOINT_CIRCLE,
    MODE_DIRECT_CIRCLE,
    MODE_POLAR_CIRCLE,
    MODE_ITERATIVE_POLAR_CIRCLE,
    MODE_DIRECT_ELLIPSE,
    MODE_MIDPOINT_ELLIPSE,
    MODE_BRESENHAM_CIRCLE,
    MODE_FILL_CIRCLE_WITH_LINES,
    MODE_CLIP_POINT,
    MODE_CLIP_LINE,
    MODE_CLIP_POLYGON,
    MODE_CLIP_CIRCLE_POINT,
    MODE_CLIP_CIRCLE_LINE,
};

DrawMode g_currentMode = MODE_NONE;
// Mouse click state
bool g_firstClick = false, g_secondClick = false;
int  g_startX = 0, g_startY = 0, g_X1 = 0, g_Y1 = 0;
// Mouse click state
struct Shape {
    DrawMode mode;
    COLORREF color;
    int x1, y1, x2, y2;
};

vector<Shape>  g_shapes;
vector<POINT>  g_polygonPoints;

// Clip window boundaries
int g_clipX1 = 150, g_clipY1 = 150, g_clipX2 = 600, g_clipY2 = 400 , g_clipXC = 400 , g_clipYC = 300 , g_clipR = 100;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================
int Round(double x) { return (int)(x + 0.5); }

void swap(int& a, int& b, int& c, int& d) {
    int temp;
    temp = a; a = c; c = temp;
    temp = b; b = d; d = temp;
}

// helper function to check if point lies within the circle radius
bool CheckIFPointINCircle(int R, int xc, int yc, int x, int y) {
    return sqrt(pow((x - xc), 2) + pow((y - yc), 2)) <= R;
}

bool FindCircle(int x, int y, int& xc, int& yc, int& R) {
    for (auto& s : g_shapes) {
        if (s.mode == MODE_DIRECT_CIRCLE ||
            s.mode == MODE_BRESENHAM_CIRCLE || 
            s.mode == MODE_MODIFIED_MIDPOINT_CIRCLE ||
            s.mode == MODE_POLAR_CIRCLE ||
            s.mode == MODE_ITERATIVE_POLAR_CIRCLE)
        {
            R = sqrt(pow((s.x2 - s.x1), 2) + pow((s.y2 - s.y1), 2));
            xc = s.x1;yc = s.y1;

            if (CheckIFPointINCircle(R, xc, yc, x, y)) {
                return true;
            }
            {

            }
        }
    }
    return false;
}

// ============================================================================
// DRAW 8 SYMMETRIC POINTS
// ============================================================================
void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF color) {
    SetPixel(hdc, xc + x, yc + y, color);    // Quadrant I
    SetPixel(hdc, xc - x, yc + y, color);    // Quadrant II
    SetPixel(hdc, xc - x, yc - y, color);    // Quadrant III
    SetPixel(hdc, xc + x, yc - y, color);    // Quadrant IV
    SetPixel(hdc, xc + y, yc + x, color);    // Octant between I and II
    SetPixel(hdc, xc - y, yc + x, color);    // Octant between II and III
    SetPixel(hdc, xc - y, yc - x, color);    // Octant between III and IV
    SetPixel(hdc, xc + y, yc - x, color);    // Octant between IV and I
}

// ============================================================================
// DRAW 4 SYMMETRIC POINTS FOR ELLIPSE
// ============================================================================
void Draw4Points(HDC hdc, int xc, int yc, int x, int y, COLORREF color) {
    SetPixel(hdc, xc + x, yc + y, color); 
    SetPixel(hdc, xc - x, yc + y, color);    
    SetPixel(hdc, xc + x, yc - y, color);    
    SetPixel(hdc, xc - x, yc - y, color);   
}

// ============================================================================
// ALGORITHM 1: DDA LINE
// Reference: LEC2.md - Digital Differential Analyzer
// ============================================================================
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Check if the line is more horizontal than vertical (|slope| <= 1)
    if (abs(dy) <= abs(dx)) {
        double m = (double)dy / dx;

        // Ensure we always draw from left to right
        if (x1 > x2) swap(x1, y1, x2, y2);

        SetPixel(hdc, x1, y1, c);

        int x = x1;
        double y = y1;

        while (x < x2) {
            x++;
            y += m;
            SetPixel(hdc, x, Round(y), c);
        }
    }
    else {
        // Steep slope: step along y-axis
        double mi = (double)dx / dy;

        if (y1 > y2) swap(x1, y1, x2, y2);

        SetPixel(hdc, x1, y1, c);

        int y = y1;
        double x = x1;

        while (y < y2) {
            y++;
            x += mi;
            SetPixel(hdc, Round(x), y, c);
        }
    }
}

// ============================================================================
// ALGORITHM 2: MIDPOINT LINE (Bresenham's Line)
// Reference: LEC2.md - MidPoint Algorithm
// ============================================================================
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    // Determine direction of line
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int x = x1;
    int y = y1;

    // Case 1: Gentle slope (|m| <= 1)
    if (dx >= dy) {
        int d = dx - 2 * dy;
        int delta_d1 = 2 * dx - 2 * dy;
        int delta_d2 = -2 * dy;

        SetPixel(hdc, x, y, c);

        while (x != x2) {
            if (d < 0) {
                y += sy;
                d += delta_d1;
            }
            else {
                d += delta_d2;
            }
            x += sx;
            SetPixel(hdc, x, y, c);
        }
    }
    // Case 2: Steep slope (|m| > 1)
    else {
        int d = dy - 2 * dx;
        int delta_d1 = 2 * dy - 2 * dx;
        int delta_d2 = -2 * dx;

        SetPixel(hdc, x, y, c);

        while (y != y2) {
            if (d < 0) {
                x += sx;
                d += delta_d1;
            }
            else {
                d += delta_d2;
            }
            y += sy;
            SetPixel(hdc, x, y, c);
        }
    }
}

// ============================================================================
// PARAMETRIC LINE
// ============================================================================
void DrawLineParametric(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    // Parametric drawing
    for (int i = 0; i <= steps; i++)
    {
        double t = (double)i / steps;

        int x = Round(x1 + t * dx);
        int y = Round(y1 + t * dy);

        SetPixel(hdc, x, y, color);
    }
}
// ============================================================================
// ALGORITHM 3: MODIFIED MIDPOINT CIRCLE (Faster Bresenham variant)
// Reference: Circle Drawing Algorithms.md - Section 5.6
// Uses 2nd order differences for maximum speed
// ============================================================================
void DrawCircleModifiedMidpoint(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R;
    int d = 1 - R;                    // Initial decision variable
    int c1 = 3;                       // Initial increment value (2*x + 3)
    int c2 = 5 - 2 * R;               // Initial increment value (2*(x-y) + 5)

    Draw8Points(hdc, xc, yc, x, y, color);

    while (x < y) {
        if (d < 0) {
            // Move right only (midpoint inside circle)
            d += c1;
            c2 += 2;
        }
        else {
            // Move right and down (midpoint outside circle)
            d += c2;
            c2 += 4;
            y--;
        }
        c1 += 2;
        x++;
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}
// ============================================================================
// ALGORITHM 4: DIRECT CIRCLE
// ============================================================================
void DrawCircleDirect(HDC hdc, int xc, int yc, int R, COLORREF color) {
    for (int x = 0; x <= (int)(R / sqrt(2.0)) + 1; x++) {
        int y = (int)round(sqrt((double)R * R - (double)x * x));
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}

// ============================================================================
// ALGORITHM 5: BRESENHAM CIRCLE
// ============================================================================
void DrawCircleBresenham(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R, d = 1 - R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x < y) {
        if (d < 0) d += 2 * x + 3;
        else { d += 2 * (x - y) + 5; y--; }
        x++;
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}

// ============================================================================
// POLAR CIRCLE
// ============================================================================
void DrawCirclePolar(HDC hdc,int xc,int yc, int R,COLORREF color) {
    int x = R ,y = 0;
    double theta = 0, dtheta = 1.0 / R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x > y) {
        theta += dtheta;
        x = round(R * cos(theta));
        y = round(R * sin(theta));
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}

// ============================================================================
// ITERATIVE POLAR CIRCLE
// ============================================================================
void DrawCircleIterativePolar(HDC hdc, int xc, int yc, int R, COLORREF color) {
    double x = R, y = 0;
    double dtheta = 1.0 / R;
    double cdtheta = cos(dtheta), sdtheta = sin(dtheta);
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x > y) {
        double x1 = x * cdtheta - y * sdtheta;
        y = x * sdtheta + y * cdtheta;
        x = x1;
        Draw8Points(hdc, xc, yc, round(x), round(y), color);
    }
}
// ============================================================================
// DIRECT ELLIPSE
// ============================================================================
void DrawEllipseDirect(HDC hdc, int xc, int yc, int a, int b, COLORREF color)
{

    for (int x = -a; x <= a; x++)
    {
        // From ellipse equation
        double y = b * sqrt(1.0 - (double)(x * x) / (a * a));

        // Upper half
        SetPixel(hdc, xc + x, Round(yc + y), color);

        // Lower half
        SetPixel(hdc, xc + x, Round(yc - y), color);
    }
}
// ============================================================================
// MIDPOINT ELLIPSE
// ============================================================================

void DrawEllipseMidpoint(HDC hdc , int xc , int yc , int a , int b , COLORREF color) {
    int x = 0, y = b;
    double d = 0.25 * a * a + b * b - a * a * b;
    double dx = 2 * b * b * x, dy = 2 * a * a * y;

    Draw4Points(hdc, xc, yc, x, y, color);
    while (dx < dy) {
        x++;
        dx += 2 * b * b;
        if (d < 0) {
            d += dx + b * b;
        }
        else
        {
            dy -= 2 * a * a;
            d += dx - dy + b * b;
            y--;
        }
        Draw4Points(hdc, xc, yc, x, y, color);
    }

    d = pow(b, 2) * pow((x + 0.5), 2) + pow(a, 2) * pow((y - 1), 2) - pow(a * b, 2);
    Draw4Points(hdc, xc, yc, x, y, color);
    while (y >= 0) {

        y--;
        dy -= 2 * a * a;
        if (d > 0) {
            d += -dy + a * a;
        }
        else
        {
            dx += 2 * b * b;
            d += dx - dy + a * a;
            x++;
        }
        Draw4Points(hdc, xc, yc, x, y, color);
    }

}

// ============================================================================
// FILL CIRCLE WITH LINES
// ============================================================================
void FillCircleWithLines(HDC hdc, int xc, int yc, int R, double thetaS, double thetaE, COLORREF color) {
    if (thetaS == thetaE) return;

    int x, y;
    for (double theta = thetaS; theta <= thetaE; theta += (double)1.0/90.0)
    {
        x = xc + int(R * cos(theta));
        y = yc + int(R * sin(theta));
        DrawLineMidpoint(hdc, xc, yc, x, y, color);
    }
}

// ============================================================================
// CLIP WINDOW DRAWING
// ============================================================================
void DrawClipWindow(HDC hdc) {
    HPEN pen = CreatePen(PS_DASH, 1, RGB(255, 255, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, g_clipX1, g_clipY1, g_clipX2, g_clipY2);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);
}

void DrawCircleClipWindow(HDC hdc) {
    HPEN pen = CreatePen(PS_DASH, 1, RGB(255, 255, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    DrawCircleBresenham(hdc,g_clipXC,g_clipYC,g_clipR, RGB(255, 255, 0));
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);
}

// ============================================================================
// POINT CLIPPING
// ============================================================================
void PointClipping(HDC hdc, int x, int y,
    int xl, int yt, int xr, int yb, COLORREF color)
{
    if (x >= xl && x <= xr && y >= yt && y <= yb) {
        for (int dx = -2; dx <= 2; dx++)
            for (int dy = -2; dy <= 2; dy++)
                SetPixel(hdc, x + dx, y + dy, color);
        cout << "  Point INSIDE - visible" << endl;
    }
    else {
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

OutCode GetOutCode(double x, double y, int xl, int yt, int xr, int yb) {
    OutCode out; out.All = 0;
    if (x < xl) out.left = 1;
    else if (x > xr) out.right = 1;
    if (y < yt) out.top = 1;
    else if (y > yb) out.bottom = 1;
    return out;
}

void VIntersect(double xs, double ys, double xe, double ye,
    int x, double* xi, double* yi) {
    *xi = x;
    *yi = ys + (x - xs) * (ye - ys) / (xe - xs);
}
void HIntersect(double xs, double ys, double xe, double ye,
    int y, double* xi, double* yi) {
    *yi = y;
    *xi = xs + (y - ys) * (xe - xs) / (ye - ys);
}

void CohenSuth(HDC hdc, int xs, int ys, int xe, int ye,
    int xl, int yt, int xr, int yb, COLORREF color)
{
    double x1 = xs, y1 = ys, x2 = xe, y2 = ye;
    OutCode o1 = GetOutCode(x1, y1, xl, yt, xr, yb);
    OutCode o2 = GetOutCode(x2, y2, xl, yt, xr, yb);

    while ((o1.All || o2.All) && !(o1.All & o2.All)) {
        double xi, yi;
        if (o1.All) {
            if (o1.left)   VIntersect(x1, y1, x2, y2, xl, &xi, &yi);
            else if (o1.top)    HIntersect(x1, y1, x2, y2, yt, &xi, &yi);
            else if (o1.right)  VIntersect(x1, y1, x2, y2, xr, &xi, &yi);
            else                HIntersect(x1, y1, x2, y2, yb, &xi, &yi);
            x1 = xi; y1 = yi;
            o1 = GetOutCode(x1, y1, xl, yt, xr, yb);
        }
        else {
            if (o2.left)   VIntersect(x1, y1, x2, y2, xl, &xi, &yi);
            else if (o2.top)    HIntersect(x1, y1, x2, y2, yt, &xi, &yi);
            else if (o2.right)  VIntersect(x1, y1, x2, y2, xr, &xi, &yi);
            else                HIntersect(x1, y1, x2, y2, yb, &xi, &yi);
            x2 = xi; y2 = yi;
            o2 = GetOutCode(x2, y2, xl, yt, xr, yb);
        }
    }

    if (!o1.All && !o2.All) {
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, Round(x1), Round(y1), NULL);
        LineTo(hdc, Round(x2), Round(y2));
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        cout << "  Line clipped and drawn." << endl;
    }
    else {
        cout << "  Line completely outside - clipped." << endl;
    }
}

// ============================================================================
// POLYGON CLIPPING - Sutherland-Hodgman
// ============================================================================
struct Vertex { double x, y; Vertex(double a = 0, double b = 0) :x(a), y(b) {} };
typedef vector<Vertex> VList;
typedef bool    (*IsInFn)(Vertex&, int);
typedef Vertex(*IsectFn)(Vertex&, Vertex&, int);

bool InLeft(Vertex& v, int e) { return v.x >= e; }
bool InRight(Vertex& v, int e) { return v.x <= e; }
bool InTop(Vertex& v, int e) { return v.y >= e; }
bool InBottom(Vertex& v, int e) { return v.y <= e; }

Vertex VIsectPoly(Vertex& v1, Vertex& v2, int xe) {
    return Vertex(xe, v1.y + (xe - v1.x) * (v2.y - v1.y) / (v2.x - v1.x));
}
Vertex HIsectPoly(Vertex& v1, Vertex& v2, int ye) {
    return Vertex(v1.x + (ye - v1.y) * (v2.x - v1.x) / (v2.y - v1.y), ye);
}

VList ClipEdge(VList p, int edge, IsInFn In, IsectFn Isect) {
    VList out;
    if (p.empty()) return out;
    Vertex v1 = p.back(); bool v1in = In(v1, edge);
    for (auto& v2 : p) {
        bool v2in = In(v2, edge);
        if (!v1in && v2in) { out.push_back(Isect(v1, v2, edge)); out.push_back(v2); }
        else if (v1in && v2in) out.push_back(v2);
        else if (v1in)         out.push_back(Isect(v1, v2, edge));
        v1 = v2; v1in = v2in;
    }
    return out;
}

void PolygonClip(HDC hdc, vector<POINT>& pts,
    int xl, int yt, int xr, int yb, COLORREF color)
{
    VList vl;
    for (auto& p : pts) vl.push_back(Vertex(p.x, p.y));
    vl = ClipEdge(vl, xl, InLeft, VIsectPoly);
    vl = ClipEdge(vl, yt, InTop, HIsectPoly);
    vl = ClipEdge(vl, xr, InRight, VIsectPoly);
    vl = ClipEdge(vl, yb, InBottom, HIsectPoly);
    if (vl.size() < 2) { cout << "  Polygon completely outside." << endl; return; }

    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    Vertex v1 = vl.back();
    for (auto& v2 : vl) {
        MoveToEx(hdc, Round(v1.x), Round(v1.y), NULL);
        LineTo(hdc, Round(v2.x), Round(v2.y));
        v1 = v2;
    }
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
    cout << "  Polygon clipped with " << vl.size() << " vertices." << endl;
}

// ============================================================================
// POINT CIRCLE CLIPPING
// ============================================================================
void PointCircleClip(HDC hdc , int xc,int yc,int x,int y,int R, COLORREF color) {
    if (CheckIFPointINCircle(R, xc, yc, x, y)) {
        for (int dy = -2; dy <= 2; dy++)
        {
            for (int dx = -2; dx <= 2; dx++)
            {
                SetPixel(hdc, x + dx, y + dy, color);
            }
        }
    }
}
// ============================================================================
// LINE CIRCLE CLIPPING
// ============================================================================
void LineCircleClip(HDC hdc, int xc, int yc, int x1, int y1,int x2,int y2, int R, COLORREF color) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    double xInc = (double)dx / steps , yInc = (double)dy / steps;
    double x = x1, y = y1;

    for (int i = 0; i <= steps; i++)
    {
        if (CheckIFPointINCircle(R,xc,yc,round(x),round(y)))
        {
            SetPixel(hdc, round(x), round(y), color);
        }
        x += xInc;
        y += yInc;
    }

}
// ============================================================================
// SHAPE RENDERING  (FIX 5: clipping modes also redraw clip window)
// ============================================================================
void RenderShape(HDC hdc, const Shape& s) {
    switch (s.mode) {
    case MODE_DDA_LINE:
        DrawLineDDA(hdc, s.x1, s.y1, s.x2, s.y2, s.color);
        break;
    case MODE_MIDPOINT_LINE:
        DrawLineMidpoint(hdc, s.x1, s.y1, s.x2, s.y2, s.color);
        break;
    case MODE_MODIFIED_MIDPOINT_CIRCLE: {
        int R = (int)sqrt((double)(s.x2 - s.x1) * (s.x2 - s.x1) + (double)(s.y2 - s.y1) * (s.y2 - s.y1));
        DrawCircleModifiedMidpoint(hdc, s.x1, s.y1, R, s.color);
        break;
    }
    case MODE_DIRECT_CIRCLE: {
        int R = (int)sqrt((double)(s.x2 - s.x1) * (s.x2 - s.x1) + (double)(s.y2 - s.y1) * (s.y2 - s.y1));
        DrawCircleDirect(hdc, s.x1, s.y1, R, s.color);
        break;
    }
    case MODE_BRESENHAM_CIRCLE: {
        int R = (int)sqrt((double)(s.x2 - s.x1) * (s.x2 - s.x1) + (double)(s.y2 - s.y1) * (s.y2 - s.y1));
        DrawCircleBresenham(hdc, s.x1, s.y1, R, s.color);
        break;
    }
    case MODE_CLIP_LINE:
        DrawClipWindow(hdc);
        CohenSuth(hdc, s.x1, s.y1, s.x2, s.y2,
            g_clipX1, g_clipY1, g_clipX2, g_clipY2, s.color);
        break;
    default: break;
    }
}
// ============================================================================
// COLOR CHOOSER HELPERS
// ============================================================================
void ChooseDrawingColor(HWND hwnd) {
    CHOOSECOLOR cc;
    static COLORREF customColors[16];

    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.lpCustColors = customColors;
    cc.rgbResult = g_drawColor;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&cc)) {
        g_drawColor = cc.rgbResult;
        cout << "Drawing color changed to RGB("
            << (int)GetRValue(g_drawColor) << ", "
            << (int)GetGValue(g_drawColor) << ", "
            << (int)GetBValue(g_drawColor) << ")" << endl;
    }
}
// ============================================================================
// MENU CREATION
// ============================================================================
HMENU CreateMainMenu() {
    HMENU bar = CreateMenu();

    HMENU hFile = CreatePopupMenu();
    AppendMenu(hFile, MF_STRING, IDM_FILE_CLEAR, L"Clear Screen");
    AppendMenu(hFile, MF_STRING, IDM_FILE_SAVE, L"Save to File");
    AppendMenu(hFile, MF_STRING, IDM_FILE_LOAD, L"Load from File");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)hFile, L"File");

    HMENU hPref = CreatePopupMenu();
    AppendMenu(hPref, MF_STRING, IDM_PREFERENCES_BG_WHITE, L"Background: white");
    AppendMenu(hPref, MF_STRING, IDM_PREFERENCES_CHOOSE_COLOR, L"Choose Drawing Color...");
    AppendMenu(hPref, MF_SEPARATOR, 0, NULL);
    HMENU hCur = CreatePopupMenu();
    AppendMenu(hCur, MF_STRING, IDM_PREFERENCES_CURSOR_ARROW, L"Arrow");
    AppendMenu(hCur, MF_STRING, IDM_PREFERENCES_CURSOR_CROSS, L"Crosshair");
    AppendMenu(hCur, MF_STRING, IDM_PREFERENCES_CURSOR_HAND, L"Hand");
    AppendMenu(hPref, MF_POPUP, (UINT_PTR)hCur, L"Mouse Cursor");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)hPref, L"Preferences");

    HMENU hLines = CreatePopupMenu();
    AppendMenu(hLines, MF_STRING, IDM_LINES_DDA, L"DDA");
    AppendMenu(hLines, MF_STRING, IDM_LINES_MIDPOINT, L"Midpoint (Bresenham)");
    AppendMenu(hLines, MF_STRING, IDM_LINES_PARA, L"Parametric");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)hLines, L"Lines");

    HMENU hCirc = CreatePopupMenu();
    AppendMenu(hCirc, MF_STRING, IDM_CIRCLES_MODIFIED_MIDPOINT, L"Modified Midpoint");
    AppendMenu(hCirc, MF_STRING, IDM_CIRCLES_DIRECT, L"Direct Equation");
    AppendMenu(hCirc, MF_STRING, IDM_CIRCLES_BRESENHAM, L"Midpoint (Bresenham)");
    AppendMenu(hCirc, MF_STRING, IDM_CIRCLES_POLAR ,L"Polar Circle");
    AppendMenu(hCirc, MF_STRING, IDM_CIRCLES_POLAR_ITERATIVE, L"Iterative Polar Circle");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)hCirc, L"Circles");

    HMENU hEllipse = CreatePopupMenu();
    AppendMenu(hEllipse, MF_STRING, IDM_ELLIPSE_DIRECT, L"DIRECT Ellipse");
    AppendMenu(hEllipse, MF_STRING, IDM_ELLIPSE_MIDPOINT, L"Midpoint Ellipse");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)hEllipse, L"Ellipses");

    HMENU hFill = CreatePopupMenu();
    AppendMenu(hFill, MF_STRING, IDM_FILL_CIRCLES_LINES, L"Fill Circles with Lines");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)hFill, L"Filling");

    HMENU hClip = CreatePopupMenu();
    AppendMenu(hClip, MF_STRING, IDM_CLIP_POINT, L"Rectangular Point Clipping");
    AppendMenu(hClip, MF_STRING, IDM_CLIP_LINE, L"Rectangular Line Clipping");
    AppendMenu(hClip, MF_STRING, IDM_CLIP_POLYGON, L"Rectangular Polygon Clipping");
    AppendMenu(hClip, MF_STRING, IDM_CIRCLE_CLIP_POINT, L"Circular Point Clipping");
    AppendMenu(hClip, MF_STRING, IDM_CIRCLE_CLIP_LINE, L"Circular Line Clipping");
    AppendMenu(bar, MF_POPUP, (UINT_PTR)hClip, L"Clipping");

    return bar;
}

// ============================================================================
// FILE I/O
// ============================================================================
void SaveToFile(const wchar_t* fn) {
    FILE* f; _wfopen_s(&f, fn, L"wb");
    if (f) {
        int n = (int)g_shapes.size();
        fwrite(&n, sizeof(int), 1, f);
        fwrite(g_shapes.data(), sizeof(Shape), n, f);
        fclose(f);
    }
}

void LoadFromFile(HWND hwnd, const wchar_t* fn) {
    FILE* f; _wfopen_s(&f, fn, L"rb");
    if (f) {
        int n = 0; fread(&n, sizeof(int), 1, f);
        if (n > 0 && n < 100000) {
            g_shapes.resize(n);
            fread(g_shapes.data(), sizeof(Shape), n, f);
        }
        fclose(f);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

// ============================================================================
// WINDOW PROCEDURE
// ============================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HCURSOR hCur = NULL;

    switch (msg) {

    case WM_CREATE: {
        // Attach menu to window
        SetMenu(hwnd, CreateMainMenu());

        // Open console for messages
        AllocConsole();
        FILE* fp; freopen_s(&fp, "CONOUT$", "w", stdout);
        cout << "=== Computer Graphics Project ===" << endl;
        cout << "Select mode from menu, then click to draw." << endl;
        hCur = LoadCursor(NULL, IDC_ARROW);
        break;
    }

    case WM_COMMAND: {
        int menuId = LOWORD(wParam);
        switch (menuId) {

        case IDM_FILE_CLEAR:
            g_shapes.clear(); g_firstClick = false;
            InvalidateRect(hwnd, NULL, TRUE);
            cout << "Screen cleared." << endl;
            break;

            // Preferences - Background
        case IDM_PREFERENCES_BG_WHITE:
            g_backgroundColor = RGB(255, 255, 255);
            InvalidateRect(hwnd, NULL, TRUE);
            cout << "Background color changed to White." << endl;
            break;

        case IDM_PREFERENCES_CHOOSE_COLOR:
            ChooseDrawingColor(hwnd);
            break;
            // Preferences - Cursor
        case IDM_PREFERENCES_CURSOR_ARROW:
            hCur = LoadCursor(NULL, IDC_ARROW);
            SetCursor(hCur);
            cout << "Mouse cursor changed to Arrow." << endl;
            break;
        case IDM_PREFERENCES_CURSOR_CROSS:
            hCur = LoadCursor(NULL, IDC_CROSS);
            SetCursor(hCur);
            cout << "Mouse cursor changed to Crosshair." << endl;
            break;
        case IDM_PREFERENCES_CURSOR_HAND:
            hCur = LoadCursor(NULL, IDC_HAND);
            SetCursor(hCur);
            cout << "Mouse cursor changed to Hand." << endl;
            break;


            // Lines Menu
        case IDM_LINES_DDA:
            g_currentMode = MODE_DDA_LINE;
            g_firstClick = false;
            cout << "\n[DDA Line Mode] Click two points to draw a line." << endl;
            break;
        case IDM_LINES_MIDPOINT:
            g_currentMode = MODE_MIDPOINT_LINE;
            g_firstClick = false;
            cout << "\n[Midpoint Line Mode] Click two points to draw a line." << endl;
            break;
        case IDM_LINES_PARA:
            g_currentMode = MODE_PARA_LINE;
            g_firstClick = false;
            cout << "\n[Parametric Line Mode] Click two points to draw a line." << endl;
            break;

            // Circles Menu
        case IDM_CIRCLES_MODIFIED_MIDPOINT:
            g_currentMode = MODE_MODIFIED_MIDPOINT_CIRCLE;
            g_firstClick = false;
            cout << "\n[Modified Midpoint Circle Mode] Click center, then a point on the radius." << endl;
            break;

        case IDM_CIRCLES_DIRECT:
            g_currentMode = MODE_DIRECT_CIRCLE; g_firstClick = false;
            cout << "Mode: Direct Circle" << endl; break;
        case IDM_CIRCLES_BRESENHAM:
            g_currentMode = MODE_BRESENHAM_CIRCLE; g_firstClick = false;
            cout << "Mode: Bresenham Circle" << endl; break;
        case IDM_CIRCLES_POLAR:
            g_currentMode = MODE_POLAR_CIRCLE; g_firstClick = false;
            cout << "Polar Circle Mode -> Click center , then a point to form a radius" << endl;break;
        case IDM_CIRCLES_POLAR_ITERATIVE:
            g_currentMode = MODE_ITERATIVE_POLAR_CIRCLE; g_firstClick = false;
            cout << "Iterative Polar Mode -> Click center , then a point to form a radius" << endl;break;

            // Ellipse Menu
        case IDM_ELLIPSE_DIRECT:
            g_currentMode = MODE_DIRECT_ELLIPSE; g_firstClick = false;
            cout << "Direct Ellipse Mode -> Click center , then click two points to form two radii a and b" << endl;
            break;
        case IDM_ELLIPSE_MIDPOINT:
            g_currentMode = MODE_MIDPOINT_ELLIPSE; g_firstClick = false;
            cout << "Midpoint Ellipse Mode -> Click center , then click a point to focus the ellipse" << endl;
            break;

            // Filling Menu
        case IDM_FILL_CIRCLES_LINES:
            g_currentMode = MODE_FILL_CIRCLE_WITH_LINES; g_firstClick = false;
            cout << " Fill Circle With Lines Mode -> Click a quarter of circle to fill it" << endl;
            break;
            // Clipping Menu
        case IDM_CLIP_POINT:
            g_currentMode = MODE_CLIP_POINT; g_firstClick = false;
            g_polygonPoints.clear();
            InvalidateRect(hwnd, NULL, TRUE);
            cout << "Mode: Point Clipping - click a point" << endl; break;
        case IDM_CLIP_LINE:
            g_currentMode = MODE_CLIP_LINE; g_firstClick = false;
            cout << "Mode: Line Clipping - click 2 points" << endl; break;
        case IDM_CLIP_POLYGON:
            g_currentMode = MODE_CLIP_POLYGON; g_firstClick = false;
            g_polygonPoints.clear();
            InvalidateRect(hwnd, NULL, TRUE);
            cout << "Mode: Polygon Clipping - LClick to add, RClick to finish" << endl; break;
        case IDM_CIRCLE_CLIP_POINT:
            g_currentMode = MODE_CLIP_CIRCLE_POINT; g_firstClick = false;
            g_polygonPoints.clear();
            InvalidateRect(hwnd, NULL, TRUE);
            cout << "Circle Point Clipping Mode -> click a point" << endl; break;
        case IDM_CIRCLE_CLIP_LINE:
            g_currentMode = MODE_CLIP_CIRCLE_LINE; g_firstClick = false;
            cout << "Circle Line Clipping Mode -> click 2 points" << endl; break;
        case IDM_FILE_SAVE: {
            OPENFILENAME ofn; wchar_t sz[260] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd;
            ofn.lpstrFile = sz; ofn.nMaxFile = 260;
            ofn.lpstrFilter = L"Project Files (*.bin)\0*.bin\0All\0*.*\0";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
            if (GetSaveFileName(&ofn)) {
                wstring fn = sz;
                if (fn.find(L".bin") == wstring::npos) fn += L".bin";
                SaveToFile(fn.c_str());
                MessageBox(hwnd, L"Saved successfully.", L"Save", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case IDM_FILE_LOAD: {
            OPENFILENAME ofn; wchar_t sz[260] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd;
            ofn.lpstrFile = sz; ofn.nMaxFile = 260;
            ofn.lpstrFilter = L"Project Files (*.bin)\0*.bin\0All\0*.*\0";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn)) {
                LoadFromFile(hwnd, ofn.lpstrFile);
                MessageBox(hwnd, L"Loaded successfully.", L"Load", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        }
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int   y = HIWORD(lParam);
        if (g_currentMode == MODE_NONE) {
            cout << "Please select a drawing mode from the menu first." << endl;
            break;
        }
        // --- Point Clipping ---
        if (g_currentMode == MODE_CLIP_POINT) {
            HDC hdc = GetDC(hwnd);
            DrawClipWindow(hdc);
            PointClipping(hdc, x, y, g_clipX1, g_clipY1, g_clipX2, g_clipY2, g_drawColor);
            ReleaseDC(hwnd, hdc);
            break;
        }

        // --- Polygon Clipping: collect points ---
        if (g_currentMode == MODE_CLIP_POLYGON) {
            POINT pt = { x,y }; g_polygonPoints.push_back(pt);
            HDC hdc = GetDC(hwnd);
            for (int d = -2; d <= 2; d++) { SetPixel(hdc, x + d, y, g_drawColor); SetPixel(hdc, x, y + d, g_drawColor); }
            ReleaseDC(hwnd, hdc);
            cout << "  Point added (" << x << "," << y << ")" << endl;
            break;
        }

        if (g_currentMode == MODE_MIDPOINT_ELLIPSE ||
            g_currentMode == MODE_DIRECT_ELLIPSE) {
            if (!g_firstClick)
            {
                g_startX = x, g_startY = y;
                g_firstClick = true;
            }
            else if (!g_secondClick) {
                g_X1 = x, g_Y1 = y;
                g_secondClick = true;
            }
            else {
                int b = (int)sqrt(pow(g_X1 - g_startX, 2) + pow(g_Y1 - g_startY, 2));
                int a = (int)sqrt(pow(x - g_startX, 2) + pow(y - g_startY, 2));
                HDC hdc = GetDC(hwnd);
                if (g_currentMode == MODE_MIDPOINT_ELLIPSE)
                    DrawEllipseMidpoint(hdc, g_startX, g_startY, a, b, g_drawColor);
                else {
                    DrawEllipseDirect(hdc, g_startX, g_startY, a, b, g_drawColor);
                }
                ReleaseDC(hwnd, hdc);
                cout << "Ellipse drawn with two radii a =" << a << " b=" << b << endl;
                g_firstClick = false, g_secondClick = false;
            }
            break;
        }
        if (g_currentMode == MODE_CLIP_CIRCLE_POINT)
        {
            HDC hdc = GetDC(hwnd);
            DrawCircleClipWindow(hdc);
            PointCircleClip(hdc,g_clipXC,g_clipYC,x,y,g_clipR,g_drawColor);
            ReleaseDC(hwnd, hdc);
            break;
        }

        if (g_currentMode == MODE_FILL_CIRCLE_WITH_LINES) {
            int xc, yc, R;

            if (!FindCircle(x, y, xc, yc, R))
            {
                cout << "there are no circle exist to fill" << endl; break;
            }

            double dx = x - xc, dy = y - yc;
            double angle = atan2(dy, dx), PI = 3.14159265;
            double thetaS, thetaE;
            string Q;

            if (angle >= 0 && angle < PI / 2)
            {
                thetaS = 0, thetaE = PI / 2, Q = "1st";
            }
            else if (angle >= PI / 2 && angle < PI)
            {
                thetaS = PI / 2, thetaE = PI, Q = "2nd";
            }
            else if (angle >= -PI && angle < -PI / 2)
            {
                thetaS = PI, thetaE = 3 * PI / 2, Q = "3th";
            }
            else {
                thetaS = 3 * PI / 2, thetaE = 2 * PI, Q = "4th";
            }
            HDC hdc = GetDC(hwnd);
            FillCircleWithLines(hdc, xc, yc, R, thetaS, thetaE, g_drawColor);
            ReleaseDC(hwnd, hdc);

            cout << "Filling Circle the " << Q << "quarter" << endl;

            break;
        }


        // --- Two-click shapes ---
        if (!g_firstClick) {
            g_startX = x; g_startY = y; g_firstClick = true;
            cout << "  First point: (" << x << "," << y << ")" << endl;
        }
        else {
            g_firstClick = false;
            cout << "  Second point: (" << x << "," << y << ")" << endl;

            HDC hdc = GetDC(hwnd);

            // FIX 2: All modes handled
            switch (g_currentMode) {
            case MODE_DDA_LINE:
                DrawLineDDA(hdc, g_startX, g_startY, x, y, g_drawColor);
                cout << "  -> DDA Line drawn." << endl; break;
            case MODE_MIDPOINT_LINE:
                DrawLineMidpoint(hdc, g_startX, g_startY, x, y, g_drawColor);
                cout << "  -> Midpoint Line drawn." << endl; break;
            case MODE_PARA_LINE:
                DrawLineParametric(hdc, g_startX, g_startY, x, y, g_drawColor);
                cout << "  -> Parametric Line drawn." << endl; break;
            case MODE_MODIFIED_MIDPOINT_CIRCLE: {
                int R = (int)sqrt((double)(x - g_startX) * (x - g_startX) + (double)(y - g_startY) * (y - g_startY));
                DrawCircleModifiedMidpoint(hdc, g_startX, g_startY, R, g_drawColor);
                cout << "  -> Modified Midpoint Circle R=" << R << endl; break;
            }
            case MODE_DIRECT_CIRCLE: {
                int R = (int)sqrt((double)(x - g_startX) * (x - g_startX) + (double)(y - g_startY) * (y - g_startY));
                DrawCircleDirect(hdc, g_startX, g_startY, R, g_drawColor);
                cout << "  -> Direct Circle R=" << R << endl; break;
            }
            case MODE_BRESENHAM_CIRCLE: {
                int R = (int)sqrt((double)(x - g_startX) * (x - g_startX) + (double)(y - g_startY) * (y - g_startY));
                DrawCircleBresenham(hdc, g_startX, g_startY, R, g_drawColor);
                cout << "  -> Bresenham Circle R=" << R << endl; break;
            }
            case MODE_POLAR_CIRCLE: {
                int R = (int)sqrt((double)(x - g_startX) * (x - g_startX) + (double)(y - g_startY) * (y - g_startY));
                DrawCirclePolar(hdc, g_startX, g_startY, R, g_drawColor);
                cout << " -> Polar Circle R=" << R << endl;break;
            }
            case MODE_ITERATIVE_POLAR_CIRCLE: {
                int R = (int)sqrt((double)(x - g_startX) * (x - g_startX) + (double)(y - g_startY) * (y - g_startY));
                DrawCircleIterativePolar(hdc, g_startX, g_startY, R, g_drawColor);
                cout << " -> Iterative Polar Circle R=" << R << endl;break;
            }
            
            case MODE_CLIP_LINE:
                DrawClipWindow(hdc);
                CohenSuth(hdc, g_startX, g_startY, x, y,
                    g_clipX1, g_clipY1, g_clipX2, g_clipY2, g_drawColor);
                break;
            case MODE_CLIP_CIRCLE_LINE: {

                
            }
            default: break;
            }

            ReleaseDC(hwnd, hdc);

            // FIX 3: Save all shapes including clip line
            Shape ns; ns.mode = g_currentMode; ns.color = g_drawColor;
            ns.x1 = g_startX; ns.y1 = g_startY; ns.x2 = x; ns.y2 = y;
            g_shapes.push_back(ns);
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        if (g_currentMode == MODE_CLIP_POLYGON && g_polygonPoints.size() >= 3) {
            HDC hdc = GetDC(hwnd);
            DrawClipWindow(hdc);
            PolygonClip(hdc, g_polygonPoints,
                g_clipX1, g_clipY1, g_clipX2, g_clipY2, g_drawColor);
            ReleaseDC(hwnd, hdc);
            g_polygonPoints.clear();
        }
        break;
    }

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && hCur) {
            SetCursor(hCur); return TRUE;
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT r; GetClientRect(hwnd, &r);
        HBRUSH br = CreateSolidBrush(g_backgroundColor);
        FillRect(hdc, &r, br); DeleteObject(br);
        for (auto& s : g_shapes) RenderShape(hdc, s);
        // If a clipping mode is active, show the clip window
        if (g_currentMode == MODE_CLIP_POINT ||
            g_currentMode == MODE_CLIP_LINE ||
            g_currentMode == MODE_CLIP_POLYGON)
            DrawClipWindow(hdc);
        if (g_currentMode == MODE_CLIP_CIRCLE_POINT ||
            g_currentMode == MODE_CLIP_CIRCLE_LINE ) {
            DrawCircleClipWindow(hdc);
        }
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
// ============================================================================
// MAIN
// ============================================================================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"CGProject";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"RegisterClass failed!", L"Error", MB_ICONERROR);
        return 0;
    }

    RECT wr = { 0, 0, 800, 600 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, TRUE);

    HWND hwnd = CreateWindow(
        L"CGProject",
        L"Computer Graphics Project",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL, NULL, hInst, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, L"CreateWindow failed!", L"Error", MB_ICONERROR);
        return 0;
    }

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
