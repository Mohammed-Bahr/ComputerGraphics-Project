---
Created: " 2026-04-14 15:09"
tags:
  - FACL
  - ComputerGraphics
Source:
---
> [!quote] Some people thrive on huge, dramatic change. Some people prefer the slow and steady route. Do what's right for you.
> — Julie Morgenstern

**"اللهم إني أسألك فهم النبيين، وحفظ المرسلين، والملائكة المقربين، اللهم اجعل ألسنتنا عامرة بذكرك، وقلوبنا بخشيتك، إنك على كل شيء قدير"**

---

## Filling Closed Shapes
Given a closed shape and filling color ($F_c$) Required to color the interior area by the filling color :
**Algorithms like :**
1. Flood fill .
2. Scan line filling .
3. barycentric fill (for triangles)
### 1. Flood Fill Algorithms

**The Problem**: Given a boundary color and an interior point, color all connected interior pixels.

#### Recursive Approach (4-connected)
```c
void FloodFill(HDC hdc, int x, int y, Color bc, Color fc) {
    Color c = GetPixel(hdc, x, y);
    
    // Stop if we hit boundary or already filled
    if (c == bc || c == fc) return;
    
    SetPixel(hdc, x, y, fc);
    
    // Recurse to 4 neighbors
    FloodFill(hdc, x+1, y, bc, fc);
    FloodFill(hdc, x-1, y, bc, fc);
    FloodFill(hdc, x, y-1, bc, fc);
    FloodFill(hdc, x, y+1, bc, fc);
}
```

**The Problem with Recursion**: Deep recursion causes **stack overflow** for large regions.

> [!note] 8-connected Flood Fill
> The 4-connected variant only fills pixels sharing an edge. For diagonal connectivity, use 8-connected fill which also checks the 4 diagonal neighbours. This is the standard variant used in most paint programs.

#### Non-Recursive Solution (Explicit Stack)
the idea is to use software `stack` or `queue` 
Replace the call stack with a software stack (LIFO structure):

```c++
# include <stack>
# include <iostream>

struct Point{
	int X , Y ;
	point (int x = 0, int  y = 0): X(x) ,Y(y){}
}
void NRecursiveFloodFill(HDC hdc, int x, int y, Color bc, Color fc) {
    Stack<Point> s;
    s.push(Point(x, y));
    
    while (!s.empty()) {
        Point p = s.top(); 
        s.pop();
        
        Color c = GetPixel(hdc, p.x, p.y);
        if (c == bc || c == fc) continue;
        
        SetPixel(hdc, p.x, p.y, fc);
        
        // Push 4-connected neighbors
        s.push(Point(p.x, p.y+1));
        s.push(Point(p.x, p.y-1));
        s.push(Point(p.x-1, p.y));
        s.push(Point(p.x+1, p.y));
    }
}
```

**Open vs Closed Nodes**: 
- **Open nodes**: Pixels still to be processed (in stack)
- **Closed nodes**: Pixels already colored and removed from stack

> [!warning] Common Exam Mistakes
> 1. 🔴 Recursive flood fill will always crash for large shapes — never use it in production code
> 2. 🔴 Forgetting to sort the Active Edge List by x coordinate will produce completely wrong filling
> 3. 🟡 Off-by-one errors at polygon vertices are the #1 bug in scan-line implementations

---

### 2. Scanline Rendering Polygon Filling

**More Efficient Approach**: For polygons, we can use scan-line coherence (horizontal lines crossing the polygon) rather than pixel-by-pixel exploration.

#### Step 1: Convexity Test
Before choosing an algorithm, determine if the polygon is convex using cross products:

```c
bool IsConvex(Point vertices[], int n) {
    int sign = 0;
    for (int i = 0; i < n; i++) {
        Vector v1 = vertices[(i+1)%n] - vertices[i];
        Vector v2 = vertices[(i+2)%n] - vertices[(i+1)%n];
        
        // Cross product (z-component only in 2D)
        float cross = v1.x * v2.y - v1.y * v2.x;
        
        if (sign == 0) sign = (cross > 0) ? 1 : -1;
        else if (sign * cross < 0) return false;  // Sign change = concave
    }
    return true;
}
```

#### Step 2A: Convex Polygon Fill (Edge Table)
For convex polygons, each scan line intersects exactly twice (left and right edges).

**Edge Table Structure**:
```c
struct EdgeTableEntry {
    double x_left;   // Minimum x for this y
    double x_right;  // Maximum x for this y
};

EdgeTableEntry table[SCREEN_HEIGHT];

void InitTable(EdgeTableEntry t[]) {
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        t[i].x_left = INT_MAX;
        t[i].x_right = -INT_MAX;
    }
}
```

**Building the Table** (DDA Algorithm):
```c
void EdgeToTable(Point p1, Point p2, EdgeTableEntry T[]) {
    if (p1.y == p2.y) return;  // Horizontal edge
    
    // Ensure p1 is above p2
    if (p1.y > p2.y) swap(p1, p2);
    
    double x = p1.x;
    double mi = (double)(p2.x - p1.x) / (p2.y - p1.y);  // Inverse slope (1/m)
    
    for (int y = p1.y; y < p2.y; y++) {
        if (x < T[y].x_left) T[y].x_left = x;
        if (x > T[y].x_right) T[y].x_right = x;
        x += mi;  // Increment by inverse slope
    }
}
```

**Rendering**:
```c
void TableToScreen(HDC hdc, EdgeTableEntry T[], Color c) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        if (T[y].x_left < T[y].x_right) {
            DrawLine(hdc, ceil(T[y].x_left), y, floor(T[y].x_right), y, c);
        }
    }
}
```

#### Step 2B: Non-Convex Polygon Fill (Active Edge List)
For concave polygons, scan lines may intersect multiple times. We use a *array of linked list* of active edges.

**Edge Node Structure**:
```
[x, y_max, 1/m, next_pointer]
```

**Algorithm Overview**:
1. **Edge Table ($ET$)**: Array of linked lists, sorted by y-coordinate. Each edge enters at its minimum y.
2. **Active Edge List ($AEL$)**: Edges currently intersecting the scan line.
3. **Process**: For each scan line $y$:
	- Move edges from $ET[y]$ to $AEL$
	 - Sort $AEL$ by x-intersection
	 - Fill between pairs of intersections ($x0$ to $x1$, $x2$ to $x3$, etc.)
	 - Remove edges where $y = y_{max}$
	 - Update x-intersections: $x_{new} = x_{old} + (1/m)$

```c
void FillNonConvex(Polygon poly) {
    // Build Edge Table
    LinkedList ET[HEIGHT];
    for (each edge in poly) {
        Node* node = CreateNode(edge);
        ET[edge.y_min].append(node);
    }
    
    LinkedList active = NULL;
    int y = 0;
    
    while (active != NULL || y < HEIGHT) {
        // Add new edges
        if (ET[y] != NULL) active.merge(ET[y]);
        
        // Sort by x-intersection
        active.sort_by_x();
        
        // Draw between pairs
        for (int i = 0; i < active.size(); i += 2) {
            DrawLine(active[i].x, y, active[i+1].x, y);
        }
        
        // Remove finished edges and update x
        active.remove_if(y >= node.y_max);
        active.update_x_by_inverse_slope();
        y++;
    }
}
```

---

## 3. Barycentric Fill (for Triangles)

Used for **interpolating values** across a triangle's interior — colors, texture coordinates, normals, etc.

**Concept**: Every point P inside a triangle can be expressed as:
$$P = \alpha V_0 + \beta V_1 + \gamma V_2$$

Where $\alpha + \beta + \gamma = 1$ and all are ≥ 0.

**Applications** (shown in your images):
- **Flat color**: All pixels same color
- **Vertex colors**: Interpolate red→cyan→green across the triangle
- **Texture mapping**: Use (u,v) coordinates to sample a texture image

## Summary: Connecting the Concepts

### Mathematical Continuity → Visual Smoothness
| Curve Type | Continuity | Control | Best For |
|------------|-----------|---------|----------|
| Bezier | $C^0$ between segments | Global | Simple curves, font design |
| Cardinal | $C^1$ | Local | Interpolation through points |
| B-Spline | $C^2$ | Local | Industrial design, smooth surfaces |

### From Math to Pixels
1. **Parametric curves** (Bezier/B-splines) are converted to polylines via **recursive subdivision** or **De Casteljau's algorithm**
2. **Polygons** are filled using **scan-line coherence** rather than expensive per-pixel tests
3. **Irregular shapes** use **flood fill** with explicit stacks to avoid recursion limits

### Key Implementation Insights
- **Recursion is elegant but dangerous**: Always consider stack depth for production graphics code
- **Coherence is performance**: Scan-line algorithms exploit the fact that adjacent pixels share properties
- **Data structures matter**: Edge tables vs. linked lists trade memory for flexibility (convex vs. concave)

The lecture progression mirrors real graphics pipelines: **mathematical representation** → **geometric processing** → **scan conversion** → **pixel output**.