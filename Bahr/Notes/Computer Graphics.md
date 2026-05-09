---
Created: " 2026-03-07 14:50"
tags:
  - FACL
  - ComputerGraphics
Source:
---
> [!quote] The man who trusts men will make fewer mistakes than he who distrusts them.
> — Cavour

**"اللهم إني أسألك فهم النبيين، وحفظ المرسلين، والملائكة المقربين، اللهم اجعل ألسنتنا عامرة بذكرك، وقلوبنا بخشيتك، إنك على كل شيء قدير"**

# Computer Graphics Summary

## 1. Overview
- A university-level Computer Graphics course covering rasterization, line/circle drawing algorithms, polygon filling, and parametric curves/splines
- Focus on both theoretical foundations and practical C++ implementations using Win32 GDI (`SetPixel`, `LineTo`, etc.)
- Core theme: converting mathematical descriptions into pixel-based raster images efficiently

---

## Main Chapters

- [[FACL/ComputerGraphics/Chapters/LEC1/LEC1|Lec 1: Introduction to Graphics]]
- [[FACL/ComputerGraphics/Chapters/LEC2/LEC2|Lec 2: Line Drawing]]
- [[Circle Drawing Algorithms|Lec 3: Circle Drawing]]
- [[LEC4|Lec 4: Parametric Curves & Splines]]

---

## 2. Core Topics

### 2.1 Display Systems

#### Raster Scan Display
- Screen treated as a **grid of pixels** (picture elements)
- **Frame Buffer**: 2D array in memory storing pixel data
- **Color channels**: RGB (0-255 each) or RGBA (Alpha = transparency)
- **Data flow**: CPU → Display Buffer → Graphics Controller → Display
- Constant refresh rate (~25 fps); higher resolution = more computation

#### Random Vector (Calligraphic) Display
- Electron beam draws continuous lines (no pixel grid)
- Primitives: `MoveTo(x,y)`, `LineTo(x,y)`, `SetColor()`
- Superior line quality (no aliasing), but variable speed and poor at filled areas
- Used in oscilloscopes, early arcade games (e.g., Asteroids)

| Feature | Raster Scan | Vector Display |
|---------|-------------|----------------|
| Basic Unit | Pixels | Lines |
| Memory | Frame buffer | Display list |
| Line Quality | Jagged (aliased) | Smooth |
| Speed | Constant | Variable |

---

### 2.2 Line Drawing Algorithms

#### Naive Iteration Method
- Based on line equation: `(y - y₁)/(x - x₁) = (y₂ - y₁)/(x₂ - x₁)`
- **Rule**: |m| ≤ 1 → iterate X; |m| > 1 → iterate Y
- Problem: expensive floating-point multiplication and rounding per pixel

**Steps:**
1. Calculate slope `m = (y₂ - y₁) / (x₂ - x₁)`
2. If |m| ≤ 1:
   - Loop x from x₁ to x₂
   - Compute `y = y₁ + (x - x₁) * m`
   - `SetPixel(x, Round(y), color)`
3. If |m| > 1:
   - Loop y from y₁ to y₂
   - Compute `x = x₁ + (y - y₁) / m`
   - `SetPixel(Round(x), y, color)`

```cpp
void LineNaive(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    double m = (double)(y2 - y1) / (x2 - x1);
    if (abs(m) <= 1) {
        if (x1 > x2) { swap(x1, x2); swap(y1, y2); }
        for (int x = x1; x <= x2; x++) {
            double y = y1 + (x - x1) * m;
            SetPixel(hdc, x, Round(y), c);
        }
    } else {
        if (y1 > y2) { swap(x1, x2); swap(y1, y2); }
        double mi = 1.0 / m;
        for (int y = y1; y <= y2; y++) {
            double x = x1 + (y - y1) * mi;
            SetPixel(hdc, Round(x), y, c);
        }
    }
}
```

#### DDA (Digital Differential Analyzer)
- Uses **incremental addition** instead of full recalculation
- Key insight: if x increases by 1, y increases by slope m

**Steps:**
1. Compute `dx = x₂ - x₁`, `dy = y₂ - y₁`
2. If |dy| ≤ |dx| (gentle slope):
   - Compute `m = dy / dx`
   - Ensure left-to-right: swap if x₁ > x₂
   - Draw first pixel at (x₁, y₁)
   - Loop: x++, y += m, `SetPixel(x, Round(y), c)`
3. Else (steep slope):
   - Compute `mi = dx / dy`
   - Ensure bottom-to-top: swap if y₁ > y₂
   - Draw first pixel at (x₁, y₁)
   - Loop: y++, x += mi, `SetPixel(Round(x), y, c)`

```cpp
void LineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1, dy = y2 - y1;

    if (abs(dy) <= abs(dx)) {
        double m = (double)dy / dx;
        if (x1 > x2) { swap(x1, x2); swap(y1, y2); }
        SetPixel(hdc, x1, y1, c);
        int x = x1;
        double y = y1;
        while (x < x2) {
            x++;
            y += m;
            SetPixel(hdc, x, Round(y), c);
        }
    } else {
        double mi = (double)dx / dy;
        if (y1 > y2) { swap(x1, x2); swap(y1, y2); }
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
```

#### Midpoint / Bresenham's Line Algorithm
- **Discriminant function**: `f(x,y) = (y - y₁)(x₂ - x₁) - (x - x₁)(y₂ - y₁)`
  - f(x,y) = 0 → on the line; > 0 → above; < 0 → below
- Tests the **midpoint** between two candidate pixels: (x+1, y+0.5)
- **Fully optimized**: uses only integer addition/subtraction

**Steps** (for 0 ≤ m ≤ 1, x₁ < x₂):
1. Compute `dx = x₂ - x₁`, `dy = y₂ - y₁`
2. Pre-calculate constants: `delta_d1 = 2*dx - 2*dy`, `delta_d2 = -2*dy`
3. Initialize: `x = x₁`, `y = y₁`, `d = dx - 2*dy`
4. Draw first pixel at (x, y)
5. While x < x₂:
   - If d < 0: y stays, `d += delta_d2`
   - Else: y++, `d += delta_d1`
   - x++
   - `SetPixel(x, y, c)`

```cpp
void DrawLineBresenham(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int delta_d1 = (2 * dx) - (2 * dy);
    int delta_d2 = -(2 * dy);

    int x = x1;
    int y = y1;
    int d = dx - (2 * dy);

    SetPixel(hdc, x, y, c);

    while (x < x2) {
        if (d < 0) {
            d += delta_d2;
        } else {
            y++;
            d += delta_d1;
        }
        x++;
        SetPixel(hdc, x, y, c);
    }
}
```

---

### 2.3 Circle Drawing Algorithms

#### Mathematical Foundations
- **Cartesian**: `(x - xc)² + (y - yc)² = R²`
- **Parametric (Polar)**: `x = xc + R·cos(θ)`, `y = yc + R·sin(θ)`
- **Slope**: `dy/dx = -x/y` — choose octant where |x| ≤ |y| to avoid gaps

#### 8-Way Symmetry
- If point (a, b) is on the circle, so are: (±a, ±b) and (±b, ±a)
- Reduces computation by **87.5%** — compute one octant, mirror the rest

```cpp
void Draw8Points(HDC hdc, int xc, int yc, int a, int b, COLORREF color) {
    SetPixel(hdc, xc+a, yc+b, color);
    SetPixel(hdc, xc-a, yc+b, color);
    SetPixel(hdc, xc-a, yc-b, color);
    SetPixel(hdc, xc+a, yc-b, color);
    SetPixel(hdc, xc+b, yc+a, color);
    SetPixel(hdc, xc-b, yc+a, color);
    SetPixel(hdc, xc-b, yc-a, color);
    SetPixel(hdc, xc+b, yc-a, color);
}
```

#### Direct Cartesian Method
- Solves `y = √(R² - x²)` for each x in the first octant

**Steps:**
1. Initialize: `x = 0`, `y = R`, `R2 = R * R`
2. Draw 8 symmetric points for (x, y)
3. While x < y:
   - x++
   - `y = Round(sqrt(R2 - x*x))`
   - Draw 8 symmetric points for (x, y)

```cpp
void CircleDirect(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R;
    int R2 = R * R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x < y) {
        x++;
        y = round(sqrt((double)(R2 - x*x)));
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}
```

#### Direct Polar Method
- Uses angle θ with optimal step `dθ = 1/R`

**Steps:**
1. Initialize: `θ = 0`, `dθ = 1.0/R`, `x = R`, `y = 0`
2. Draw 8 symmetric points for (x, y)
3. While x > y:
   - `θ += dθ`
   - `x = Round(R * cos(θ))`
   - `y = Round(R * sin(θ))`
   - Draw 8 symmetric points

```cpp
void CirclePolar(HDC hdc, int xc, int yc, int R, COLORREF color) {
    double theta = 0, dtheta = 1.0 / R;
    int x = R, y = 0;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x > y) {
        theta += dtheta;
        x = round(R * cos(theta));
        y = round(R * sin(theta));
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}
```

#### Iterative Polar Method
- Uses rotation matrix; pre-computes cos(dθ) and sin(dθ) once

**Steps:**
1. Initialize: `x = R`, `y = 0`, `dθ = 1.0/R`
2. Pre-compute: `c = cos(dθ)`, `s = sin(dθ)`
3. Draw 8 symmetric points for (R, 0)
4. While x > y:
   - `x1 = x*c - y*s`
   - `y = x*s + y*c`
   - `x = x1`
   - Draw 8 symmetric points for (Round(x), Round(y))

```cpp
void CircleIterativePolar(HDC hdc, int xc, int yc, int R, COLORREF color) {
    double x = R, y = 0;
    double dtheta = 1.0 / R;
    double c = cos(dtheta), s = sin(dtheta);
    Draw8Points(hdc, xc, yc, R, 0, color);
    while (x > y) {
        double x1 = x*c - y*s;
        y = x*s + y*c;
        x = x1;
        Draw8Points(hdc, xc, yc, round(x), round(y), color);
    }
}
```

#### Bresenham's Circle Algorithm
- **Test function**: `F(x,y) = x² + y² - R²`
  - < 0 → inside; > 0 → outside; = 0 → on circle
- **Initial decision variable**: `d = 1 - R`

**Steps:**
1. Initialize: `x = 0`, `y = R`, `d = 1 - R`
2. Draw 8 symmetric points for (x, y)
3. While x < y:
   - If d < 0: `d += 2*x + 3`
   - Else: `d += 2*(x - y) + 5`, y--
   - x++
   - Draw 8 symmetric points for (x, y)

```cpp
void CircleBresenham(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R;
    int d = 1 - R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x < y) {
        if (d < 0)
            d += 2*x + 3;
        else {
            d += 2*(x - y) + 5;
            y--;
        }
        x++;
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}
```

#### Faster Bresenham Circle (Second-Order Differences)
- Pre-computes and iteratively updates increment values

**Steps:**
1. Initialize: `x = 0`, `y = R`, `d = 1 - R`
2. Initialize increments: `c1 = 3`, `c2 = 5 - 2*R`
3. Draw 8 symmetric points for (x, y)
4. While x < y:
   - If d < 0: `d += c1`, `c2 += 2`
   - Else: `d += c2`, `c2 += 4`, y--
   - `c1 += 2`, x++
   - Draw 8 symmetric points for (x, y)

```cpp
void CircleFasterBresenham(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R;
    int d = 1 - R;
    int c1 = 3, c2 = 5 - 2*R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x < y) {
        if (d < 0) {
            d += c1;
            c2 += 2;
        } else {
            d += c2;
            c2 += 4;
            y--;
        }
        c1 += 2;
        x++;
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}
```

---

### 2.4 Polygon Filling

#### Flood Fill (Recursive)
- Fills a connected region by recursing to neighbors

**Steps:**
1. Get current pixel color at (x, y)
2. If color equals boundary or fill color, return
3. Set pixel to fill color
4. Recurse to 4 neighbors: (x+1,y), (x-1,y), (x,y+1), (x,y-1)

```cpp
void MyFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    COLORREF c = GetPixel(hdc, x, y);
    if (c == bc || c == fc) return;
    SetPixel(hdc, x, y, fc);
    MyFloodFill(hdc, x+1, y, bc, fc);
    MyFloodFill(hdc, x-1, y, bc, fc);
    MyFloodFill(hdc, x, y+1, bc, fc);
    MyFloodFill(hdc, x, y-1, bc, fc);
}
```

#### Flood Fill (Non-Recursive with Stack)
- Avoids stack overflow by using explicit stack (DFS)

**Steps:**
1. Push starting point onto stack
2. While stack is not empty:
   - Pop point p
   - If p's color equals boundary or fill color, continue
   - Set pixel to fill color
   - Push 4 neighbors onto stack

```cpp
struct Point { int x, y; Point(int x=0, int y=0): x(x), y(y) {} };

void NRecFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    stack<Point> s;
    s.push(Point(x, y));
    while (!s.empty()) {
        Point p = s.top(); s.pop();
        COLORREF c = GetPixel(hdc, p.x, p.y);
        if (c == bc || c == fc) continue;
        SetPixel(hdc, p.x, p.y, fc);
        s.push(Point(p.x, p.y+1));
        s.push(Point(p.x, p.y-1));
        s.push(Point(p.x-1, p.y));
        s.push(Point(p.x+1, p.y));
    }
}
```

#### Scan-Line Fill (Convex Polygon)
- Builds an edge table, then fills horizontal spans per scanline

**Steps:**
1. Initialize table: xLeft = INT_MAX, xRight = INT_MIN for all scanlines
2. For each edge (p1, p2):
   - Skip horizontal edges (p1.y == p2.y)
   - Ensure p1.y < p2.y (swap if needed)
   - Compute inverse slope `mi = (p2.x - p1.x) / (p2.y - p1.y)`
   - For each y from p1.y to p2.y: update xLeft/xRight, x += mi
3. For each scanline: draw line from ceil(xLeft) to floor(xRight)

```cpp
struct Rec { double xLeft, xRight; Rec(double l, double r): xLeft(l), xRight(r) {} };
typedef Rec Table[800];

void Init(Table t) {
    for (int i = 0; i < 800; i++) {
        t[i].xLeft = INT_MAX;
        t[i].xRight = INT_MIN;
    }
}

void Edge2Table(Table t, MyPoint p1, MyPoint p2) {
    if (p1.y == p2.y) return;
    if (p1.y > p2.y) SWAP(p1, p2);
    int y = p1.y;
    double x = p1.x;
    double mi = (double)(p2.x - p1.x) / (p2.y - p1.y);
    while (y < p2.y) {
        if (x < t[y].xLeft) t[y].xLeft = x;
        if (x > t[y].xRight) t[y].xRight = x;
        y++;
        x += mi;
    }
}

void ConvexPolyFill(HDC hdc, MyPoint p[], int n, COLORREF c) {
    Table t;
    Init(t);
    MyPoint v1 = p[n-1];
    for (int i = 0; i < n; i++) {
        MyPoint v2 = p[i];
        Edge2Table(t, v1, v2);
        v1 = v2;
    }
    for (int i = 0; i < 800; i++) {
        if (t[i].xLeft < t[i].xRight)
            DrawLine(hdc, ceil(t[i].xLeft), i, floor(t[i].xRight), i, c);
    }
}
```

#### Scan-Line Fill (Non-Convex Polygon)
- Uses linked-list edge table with active list; fills even-odd spans

**Steps:**
1. Build edge table: for each edge, create node [x, y2, mi, next] appended to t[y1]
2. Find first non-empty scanline y
3. Initialize active list = t[y]
4. While active is not empty:
   - Sort active by x ascending
   - Draw lines from even-indexed to odd-indexed nodes (ceil → floor)
   - y++
   - Remove nodes where node.y2 == y
   - Update all active nodes: x += mi
   - Append t[y] to active list

---

### 2.5 Parametric Curves & Splines

#### Fundamentals
- Curves defined as `P(t) = [x(t), y(t)]` where t ∈ [0, 1]
- **Polynomial form**: `x(t) = α₀ + α₁t + α₂t² + ... + αₙtⁿ`
- **Cubic curves (n=3)** are the industry standard

#### Hermite Cubic Curve
- Defined by: 2 endpoints + 2 tangent vectors
- **Basis matrix (M_H)**:
  ```
  [ 1   0   0   0 ]
  [ 0   1   0   0 ]
  [-3  -2   3  -1 ]
  [ 2   1  -2   1 ]
  ```

**Steps:**
1. Construct Hermite basis matrix H
2. Construct geometric matrix G = [P₀, T₀, P₁, T₁]
3. Compute coefficients: C = H × G
4. For t from 0 to 1, step dt:
   - Build power vector V = [1, t, t², t³]
   - Compute point: P(t) = V · C
   - Round and draw pixel/line segment

```cpp
struct Vector2 { double x, y; Vector2(double a=0, double b=0): x(a), y(b) {} };

Vector4 GetHermiteCoeff(double x0, double s0, double x1, double s1) {
    static double H[16] = { 2, 1, -2, 1, -3, -2, 3, -1, 0, 1, 0, 0, 1, 0, 0, 0 };
    static Matrix4 basis(H);
    Vector4 v(x0, s0, x1, s1);
    return basis * v;
}

void DrawHermiteCurve(HDC hdc, Vector2& P0, Vector2& T0,
                      Vector2& P1, Vector2& T1, int numpoints) {
    Vector4 xcoeff = GetHermiteCoeff(P0.x, T0.x, P1.x, T1.x);
    Vector4 ycoeff = GetHermiteCoeff(P0.y, T0.y, P1.y, T1.y);
    if (numpoints < 2) return;
    double dt = 1.0 / (numpoints - 1);
    bool first = true;
    for (double t = 0; t <= 1; t += dt) {
        Vector4 vt;
        vt[3] = 1;
        for (int i = 2; i >= 0; i--) vt[i] = vt[i+1] * t;
        int x = round(DotProduct(xcoeff, vt));
        int y = round(DotProduct(ycoeff, vt));
        if (first) { MoveToEx(hdc, x, y, NULL); first = false; }
        else LineTo(hdc, x, y);
    }
}
```

#### Bezier Curve (Cubic)
- Defined by 4 control points: P₁, P₄ on curve; P₂, P₃ as handles
- **Tangents**: T₀ = 3(P₂ - P₁), T₁ = 3(P₄ - P₃)

**Steps (via Hermite):**
1. Compute tangents: `T0 = 3*(P2 - P1)`, `T1 = 3*(P4 - P3)`
2. Call Hermite curve with (P1, T0, P4, T1)

**Steps (direct Bernstein):**
1. For t from 0 to 1, step dt:
   - Compute basis: `mt = 1-t`, `mt²`, `mt³`, `t²`, `t³`
   - `x = mt³·P1 + 3·mt²·t·P2 + 3·mt·t²·P3 + t³·P4`
   - `y = mt³·P1 + 3·mt²·t·P2 + 3·mt·t²·P3 + t³·P4`
   - Round and draw

```cpp
void DrawBezierCurve(HDC hdc, Vector2& P0, Vector2& P1,
                     Vector2& P2, Vector2& P3, int numpoints) {
    Vector2 T0(3*(P1.x - P0.x), 3*(P1.y - P0.y));
    Vector2 T1(3*(P3.x - P2.x), 3*(P3.y - P2.y));
    DrawHermiteCurve(hdc, P0, T0, P3, T1, numpoints);
}

void DrawBezierCurveDirect(HDC hdc, Vector2& P0, Vector2& P1,
                           Vector2& P2, Vector2& P3, int numpoints) {
    if (numpoints < 2) return;
    double dt = 1.0 / (numpoints - 1);
    bool first = true;
    for (double t = 0; t <= 1; t += dt) {
        double t2 = t*t, t3 = t2*t, mt = 1-t, mt2 = mt*mt, mt3 = mt2*mt;
        double x = mt3*P0.x + 3*mt2*t*P1.x + 3*mt*t2*P2.x + t3*P3.x;
        double y = mt3*P0.y + 3*mt2*t*P1.y + 3*mt*t2*P2.y + t3*P3.y;
        int ix = round(x), iy = round(y);
        if (first) { MoveToEx(hdc, ix, iy, NULL); first = false; }
        else LineTo(hdc, ix, iy);
    }
}
```

#### De Casteljau's Algorithm
- Numerically stable recursive evaluation of Bezier curves

**Steps:**
1. Copy control points into temporary array
2. For r from 1 to n-1:
   - For i from 0 to n-r-1:
     - `temp[i] = (1-t)*temp[i] + t*temp[i+1]`
3. Return temp[0] as the point on the curve

```cpp
Vector2 DeCasteljau(Vector2 P[], int n, double t) {
    Vector2* temp = new Vector2[n];
    for (int i = 0; i < n; i++) temp[i] = P[i];
    for (int r = 1; r < n; r++) {
        for (int i = 0; i < n - r; i++) {
            temp[i].x = (1-t)*temp[i].x + t*temp[i+1].x;
            temp[i].y = (1-t)*temp[i].y + t*temp[i+1].y;
        }
    }
    Vector2 result = temp[0];
    delete[] temp;
    return result;
}
```

#### Cardinal Spline
- Automatic tangent computation from neighboring points
- Tangent: `Tᵢ = (1-c)(Pᵢ₊₁ - Pᵢ₋₁)` where c is tension

**Steps:**
1. Set `c1 = 1 - c`
2. Compute tangent at first interior point: `T0 = c1 * (P[2] - P[0])`
3. For i from 2 to n-2:
   - Compute next tangent: `T1 = c1 * (P[i+1] - P[i-1])`
   - Draw Hermite curve from P[i-1] to P[i] with tangents T0, T1
   - Set T0 = T1 for next segment

```cpp
void DrawCardinalSpline(HDC hdc, Vector2 P[], int n, double c, int numpix) {
    double c1 = 1 - c;
    Vector2 T0(c1*(P[2].x - P[0].x), c1*(P[2].y - P[0].y));
    for (int i = 2; i < n - 1; i++) {
        Vector2 T1(c1*(P[i+1].x - P[i-1].x), c1*(P[i+1].y - P[i-1].y));
        DrawHermiteCurve(hdc, P[i-1], T0, P[i], T1, numpix);
        T0 = T1;
    }
}
```

---

### 2.6 Rendering Pipeline

1. **Model description** → vertices, polygons, normals, materials
2. **World → Camera space** transformation
3. **Hidden face removal** (visible face determination)
4. **Clipping** (remove objects outside view volume)
5. **Projection** (3D → 2D mapping)
6. **Color computation** (lighting × material reflection)
7. **Shading** (interpolation across surfaces)
8. **Texture mapping**

**Color formula**: `Color = Reflection Coefficient × Light Source Color × Intensity`

---

## 3. Important Formulas

| Formula | Description |
|---------|-------------|
| `m = (y₂ - y₁)/(x₂ - x₁)` | Line slope |
| `f(x,y) = (y-y₁)(x₂-x₁) - (x-x₁)(y₂-y₁)` | Line discriminant function |
| `d_init = Δx - 2Δy` | Bresenham line initial decision variable |
| `Δd₁ = 2Δx - 2Δy` | Bresenham line update (y changes) |
| `Δd₂ = -2Δy` | Bresenham line update (y stays) |
| `(x-xc)² + (y-yc)² = R²` | Circle Cartesian equation |
| `x = xc + R·cos(θ), y = yc + R·sin(θ)` | Circle parametric equation |
| `dθ = 1/R` | Optimal angular step for circle |
| `d_init = 1 - R` | Bresenham circle initial decision variable |
| `F(x,y) = x² + y² - R²` | Circle test function |
| `dy/dx = -x/y` | Circle slope |
| `P(t) = Σ Bᵢⁿ(t) · Pᵢ` | General Bezier curve |
| `Tᵢ = (1-c)(Pᵢ₊₁ - Pᵢ₋₁)` | Cardinal spline tangent |
| `Color = Reflection × Light × Intensity` | Color computation |

---

## 4. Code Insights

### Line Drawing Implementations
- **DDA**: Uses `double` for y (or x), incremental addition with `Round()` — simple but floating-point
- **Bresenham Line**: Pure integer arithmetic with pre-computed `delta_d1` and `delta_d2` — fastest
- Both handle gentle (|m| ≤ 1) and steep (|m| > 1) slopes via conditional branching and coordinate swapping

### Circle Drawing Implementations
- **Draw8Points**: Core utility that exploits 8-way symmetry — single call draws 8 pixels
- **Direct Cartesian**: `y = round(sqrt(R² - x²))` — simplest but slowest
- **Iterative Polar**: Pre-computes `cos(dθ)` and `sin(dθ)`, then rotates point each step — good balance
- **Bresenham Circle**: Integer-only with decision variable; faster variant iteratively updates increments `c1` and `c2`

### Curve Drawing Implementations
- **Hermite**: Matrix multiplication `C = M_H × G` to get coefficients, then evaluate `[1, t, t², t³] · C`
- **Bezier**: Can reuse Hermite by computing tangents from control points, or evaluate Bernstein polynomials directly
- **De Casteljau**: Recursive linear interpolation — stable and intuitive
- **Cardinal Spline**: Loops through point array, computes tangents, draws Hermite segments

### Filling Implementations
- **Flood Fill**: Recursive (simple but stack-overflow prone) → non-recursive with `std::stack`/`std::queue`
- **Scan-Line Fill**: Edge table → per-scanline xLeft/xRight → draw horizontal lines; extended to non-convex with active linked list

---

## 5. Key Takeaways

- **Rasterization** is the core problem: converting continuous mathematical shapes into discrete pixel grids
- **Incremental algorithms** (DDA, Bresenham) are faster than direct computation because they replace expensive operations (multiplication, sqrt, trig) with simple additions
- **Bresenham's algorithm** is the gold standard for line and circle drawing — pure integer arithmetic, no floating-point
- **8-way symmetry** reduces circle drawing work by 87.5%
- **Cubic curves** (n=3) are the sweet spot between flexibility and computational cost
- **Bezier curves** dominate graphics because of intuitive control, convex hull property, and affine invariance
- **Segmentation** (splines) enables complex shapes while maintaining local control and numerical stability
- **Continuity levels** (C⁰, C¹, C²) determine how smoothly curve segments join
- The **rendering pipeline** transforms 3D scene descriptions into 2D pixel images through a series of well-defined stages
