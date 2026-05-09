---
Created: " 2026-04-11 18:29"
tags:
  - FACL
  - ComputerGraphics
  - ComputerGraphics/FillingAlgorithms
Source:
---
> [!quote] If one is lucky, a solitary fantasy can totally transform one million realities.
> — Maya Angelou

**"اللهم إني أسألك فهم النبيين، وحفظ المرسلين، والملائكة المقربين، اللهم اجعل ألسنتنا عامرة بذكرك، وقلوبنا بخشيتك، إنك على كل شيء قدير"**

---

**Overview: What Are Filling Algorithms?**
**Purpose**: Given a closed shape, color its interior area with a specified fill color.

**Three Main Types** (from your materials):
1. *Flood Fill* — for irregular/unknown shapes
2. *Scanline Filling* — for polygons (convex & non-convex)
3. *Barycentric Fill* — specifically for triangles (with textures/colors)

---

## 1. Flood Fill Algorithm

### The Core Problem
You have:
- A starting point (x, y) inside a shape
- A ==boundary color== ($Bc$) — the outline color
- A ==fill color== ($Fc$) — the color you want to apply

> **Goal**: Color all connected interior pixels without crossing the boundary.

---

### Recursive Approach (4-Connected)

```c
void FloodFill(HDC hdc, int x, int y, Color bc, Color fc) {
    Color c = GetPixel(hdc, x, y);
    
    // STOP if we hit boundary OR already filled
    if (c == bc || c == fc) return;
    
    SetPixel(hdc, x, y, fc);
    
    // Recurse to 4 neighbors
    FloodFill(hdc, x+1, y, bc, fc);  // right
    FloodFill(hdc, x-1, y, bc, fc);  // left
    FloodFill(hdc, x, y-1, bc, fc);  // up
    FloodFill(hdc, x, y+1, bc, fc);  // down
}
```

**How it works**: Start at a point, color it, then recursively do the same for each neighbor.
> [!example]- Here is a breakdown of the mechanics and the "why" behind that critical crash risk.
> Think of this algorithm like a **digital wildfire** that starts at one pixel and spreads to its neighbors until it hits a "firebreak" (the boundary color).
> 
> 
> ### 1. The Recursive Logic (Step-by-Step)
> The algorithm follows a simple **"Check, Paint, Move"** pattern:
> 
> 1.  **Check:** It looks at the pixel's current color ($c$).
> 2.  **Base Case (The Stop Sign):** If the color is already the "Fill Color" ($fc$) or if it hits the "Boundary Color" ($bc$), it stops and "returns" to the previous pixel.
> 3.  **Paint:** If it’s an empty space, it changes the pixel to the new color.
> 4.  **Spread:** It then tells the neighbors (Right, Left, Up, Down) to do the exact same thing.
> 
> 
> 
> ---
> 
> ### 2. The 4-Connected Pattern
> The "4-connected" label refers to the directions the algorithm travels. It only moves along the cardinal axes ($x\pm1, y\pm1$). This creates a diamond-shaped expansion pattern.
> * **Advantage:** Simple and fast for basic shapes.
> * **Limitation:** It cannot "leak" through diagonal gaps. If two boundary pixels are touching only at a corner, the fill stays contained.
> 
> ---
> 
> ### 3. Why it Crashes: The "Stack Overflow"
> This is the most important part . To understand the crash, imagine the computer's **Stack** as a physical stack of trays.
> 
> * Every time the function calls itself (`FloodFill` calling `FloodFill`), the computer puts a new "tray" (a stack frame) on the pile to remember where it was.
> * For a $100 \times 100$ pixel area, you could potentially have **10,000 trays** stacked up before the first one is ever finished.
> * **The Result:** Most operating systems give a program a very limited "stack" size (often only 1MB). Once you run out of room for more "trays," the program crashes instantly.
> 
> 
> 
> ---
> 
> ### 4. How to Fix It
> In professional software (like Photoshop or MS Paint), we avoid the recursive approach for large areas. Instead, we use an ==Iterative Approach== with a **Queue** or **Stack** data structure in the "*Heap*" memory, which is much larger than the *system stack*. 
> > 

**Critical Problem**: **Stack Overflow**(which we explain above there) — each recursive call uses *system stack memory*. For large shapes, you exhaust the stack and crash.

---

### Non-Recursive Solution (Explicit Stack)

Replace the system call stack with your own software stack:

```c
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

> [!example]- The Algorithm code idea ?
> the standard way to fix the stack overflow issue in basic flood fill operations. 
> 
> By replacing the function's recursive calls with your own `Stack<Point> s;`, you are moving the memory burden from the tiny **System Call Stack** to the computer's massive **Heap Memory**. 
> 
> ### 1. The Anatomy of the Loop
> The `while (!s.empty())` loop is the engine of this algorithm. It effectively creates a "to-do list" of pixels:
> 
> * **Pop & Evaluate:** It grabs a point from the list. If it's the boundary color ($bc$) or already the fill color ($fc$), it uses `continue` to skip it instantly. This handles the "closed" nodes.
> * **Action:** If it passes the check, the pixel is painted.
> * **Expand the Frontier:** It immediately pushes the four neighboring coordinates onto the stack. These become the new "open" nodes waiting to be processed.
> 
> ### 2. Stack (DFS) vs. Queue (BFS)
> Swapping the `Stack` for a `Queue` fundamentally changes the order in which pixels are processed, creating completely different visual patterns if you were to animate the fill.
> 
> 
> 
> | Feature | Stack (`std::stack`) | Queue (`std::queue`) |
> | :--- | :--- | :--- |
> | **Data Flow** | LIFO (Last-In, First-Out) | FIFO (First-In, First-Out) |
> | **Search Type** | Depth-First Search (DFS) | Breadth-First Search (BFS) |
> | **Fill Pattern** | Snaking and erratic. It plunges deep in the last direction added (e.g., constantly moving "Right") until it hits a boundary, then backtracks. | Expanding ripples. It fills the immediate neighbors, then the neighbors' neighbors, growing outward in a uniform diamond shape. |
> | **Frontier Size** | The number of "Open" nodes stays relatively small. | The number of "Open" nodes grows significantly as the outer perimeter of the fill expands. |
> 
> 
	
**Key distinction**: (The "Open" vs. "Closed" Concept)
- **Open Nodes (The Stack/Queue):** These are the pixels the algorithm "knows" about but hasn't visited yet. They are the frontier of the fill.
    
- **Closed Nodes:** These are the pixels that have been popped, colored, and their neighbors checked. Once a node is closed, the algorithm never needs to touch it again.


**Important**: You can also use a **Queue** instead of Stack. 
- **Stack (LIFO)** → Depth-First behavior (explores one direction deeply first)
- **Queue (FIFO)** → Breadth-First behavior (expands outward in layers)
	- What will change :
		1. Declaration
		2. `s.top()` $\rightarrow$ `s.front()`
		   
	
> [!bug]- A Hidden Inefficiency in the Code
> While this code won't crash your program, it does have a memory quirk. Because it blindly pushes all four neighbors before checking their colors, the stack can become flooded with duplicates. A single pixel could easily be pushed into the stack up to four separate times by its surrounding neighbors before it finally gets popped and evaluated.
> 
> ### The Optimized Stack Fill (Preventing Duplicates)
> 
> The trick here is simple: Look before you leap. Instead of blindly throwing all four neighbors onto the stack and evaluating them later, we check their color before pushing them. This keeps the stack extremely lean and saves memory.
> 
> ```cpp
> void OptimizedFloodFill(HDC hdc, int x, int y, Color bc, Color fc) {
>     Color startColor = GetPixel(hdc, x, y);
>     if (startColor == bc || startColor == fc) return; // Prevent instant exit
> 
>     Stack<Point> s;
>     s.push(Point(x, y));
>     
>     while (!s.empty()) {
>         Point p = s.top(); 
>         s.pop();
>         
>         // Safety check in case it was modified while in stack
>         if (GetPixel(hdc, p.x, p.y) == fc) continue; 
>         
>         SetPixel(hdc, p.x, p.y, fc);
>         
>         // OPTIMIZATION: Check color BEFORE pushing to avoid duplicate stack entries
>         if (GetPixel(hdc, p.x, p.y + 1) != bc && GetPixel(hdc, p.x, p.y + 1) != fc) 
>             s.push(Point(p.x, p.y + 1)); // Down
>             
>         if (GetPixel(hdc, p.x, p.y - 1) != bc && GetPixel(hdc, p.x, p.y - 1) != fc) 
>             s.push(Point(p.x, p.y - 1)); // Up
>             
>         if (GetPixel(hdc, p.x - 1, p.y) != bc && GetPixel(hdc, p.x - 1, p.y) != fc) 
>             s.push(Point(p.x - 1, p.y)); // Left
>             
>         if (GetPixel(hdc, p.x + 1, p.y) != bc && GetPixel(hdc, p.x + 1, p.y) != fc) 
>             s.push(Point(p.x + 1, p.y)); // Right
>     }
> }
> ```

---

## 2. Scanline Polygon Filling
![[image.png]]
**Scanline Polygon Filling** is a classic, efficient algorithm in computer graphics for filling polygons (drawing their interior pixels). It’s much smarter and faster than naive methods like flood fill for most cases.

### Core Idea
Instead of checking every single pixel (which is slow), the algorithm exploits **horizontal coherence**:

- Polygons have edges.
- A horizontal line (called a **scanline**, y = constant) crosses the polygon at certain points.
- For each scanline from top to bottom, find all intersection points with the polygon edges.
- Sort those intersections by x-coordinate.
- Fill the segments **between pairs** of intersections (left to right).

For a simple convex polygon, each scanline intersects the boundary at **exactly two points** (entry and exit), so you just draw a horizontal line between them.

**Why it's better**: It processes entire rows at once using simple arithmetic, avoiding expensive per-pixel tests or recursion (like flood fill). This was especially important in the old days of slow CPUs.

---

### Step 1: Convexity Test

Before using the fast convex method, you should check if the polygon is convex: 
```c
bool IsConvex(Point vertices[], int n) {
    int sign = 0;
    for (int i = 0; i < n; i++) {
        Vector v1 = vertices[(i+1)%n] - vertices[i];
        Vector v2 = vertices[(i+2)%n] - vertices[(i+1)%n];
        
        float cross = v1.x * v2.y - v1.y * v2.x;  // Z-component of cross product
        
        if (sign == 0) sign = (cross > 0) ? 1 : -1;
        else if (sign * cross < 0) return false;  // Sign changed = concave
    }
    return true;
}
```
	
**How it works**:
- For every three consecutive vertices, compute the **cross product** of the two edges.
- The sign of this cross product tells you if the turn is left or right.
- In a **convex** polygon, all turns are in the same direction (all positive or all negative).
- If the sign changes anywhere → the polygon is **concave** (has a "dent").

> This is important because the simple Edge Table method below only works reliably for **convex** polygons (or simple polygons without self-intersections). Concave polygons may need a more general (slightly slower) scanline approach.
	
---

### Step 2A: Convex Polygon Fill (Edge Table)
> For **convex polygons**: Every horizontal scan-line intersects the polygon at **at most 2 points** (left and right).
	
**Data Structure — Edge Table**:
```c
struct EdgeTableEntry {
    double x_left;   // minimum x for this y
    double x_right;  // maximum x for this y
    EdgeTableEntry(double x = 0 , double x2 = 0): x_left(x), x_right(x2)
};

EdgeTableEntry table[SCREEN_HEIGHT];
```
One entry per screen row (`y`). For each y, we track the minimum and maximum x where the scanline crosses the polygon. (which is left and right boundaries of the polygon at that height)
> [!info]- Explanation
> - **struct EdgeTableEntry**: Defines one entry for a single horizontal scanline (one row `y`).
>   - `x_left`: Stores the **leftmost** x-coordinate where the polygon intersects this scanline.
>   - `x_right`: Stores the **rightmost** x-coordinate where the polygon intersects this scanline.
> - The constructor `EdgeTableEntry(double x = 0, double x2 = 0)` allows you to create an entry with default or custom values (useful for initialization).
> - `EdgeTableEntry table[SCREEN_HEIGHT | 800];`  
>   Creates an array with one entry per possible screen row.  
>   `SCREEN_HEIGHT | 800` is likely a mistake or a quick hack — it should probably be just `SCREEN_HEIGHT` or `max(SCREEN_HEIGHT, 800)`. The `|` (bitwise OR) here doesn't make much sense for array size.
> 
	
	
**Initialize** (set impossible values):
```c
void InitTable(EdgeTableEntry t[]) {
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        t[i].x_left = INT_MAX;   // Start very large
        t[i].x_right = -INT_MAX; // Start very small
    }
}
```
> [!info]- Explanation
> 
> - Loop through every possible scanline (every row on the screen).
> - Set `x_left` to `INT_MAX` (a very large positive number).  
>   → Any real edge intersection will be **smaller** than this, so it will be updated.
> - Set `x_right` to `-INT_MAX` (a very large negative number).  
>   → Any real edge intersection will be **larger** than this, so it will be updated.
> 
> **Why?**  
> This is a common initialization trick. After processing all edges, if `x_left` is still `INT_MAX`, it means **no edge** crossed this scanline → the row is outside the polygon.
	
	
**Build the table** using $DDA$ (Digital Differential Analyzer):
```c
void EdgeToTable(Point p1, Point p2, EdgeTableEntry T[]) {
    if (p1.y == p2.y) return;  // Skip horizontal edges
    
    // Ensure p1 is the upper point (smaller y)
    if (p1.y > p2.y) swap(p1, p2);
    
    double x = p1.x;
    double mi = (double)(p2.x - p1.x) / (p2.y - p1.y);  // Inverse slope (Δx/Δy)
    
    for (int y = p1.y; y < p2.y; y++) {
        if (x < T[y].x_left)  T[y].x_left = x;
        if (x > T[y].x_right) T[y].x_right = x;
        x += mi; // Increment x by inverse slope
        // y++; // increment y by one (from lecture)
    }
}
```

**Why inverse slope (1/m)?** Because we iterate by **y** (going down), and need to calculate the corresponding **x** at each step.
> [!info]- Explanation
> This is the most important function.
> 
> ```c
> void EdgeToTable(Point p1, Point p2, EdgeTableEntry T[]) {
>     if (p1.y == p2.y) return;  // Skip horizontal edges
> ```
> 
> - If the edge is perfectly horizontal, skip it.  
>   Horizontal edges don’t change the `x_left`/`x_right` for scanlines (they only define the top or bottom).
> 
> ```c
>     // Ensure p1 is the upper point (smaller y)
>     if (p1.y > p2.y) swap(p1, p2);
> ```
> 
> - We always want to process edges from **top to bottom**.  
>   If `p1` is lower than `p2`, we swap them so `p1.y < p2.y`.
> 
> ```c
>     double x = p1.x;
>     double mi = (double)(p2.x - p1.x) / (p2.y - p1.y);  // Inverse slope (Δx/Δy)
> ```
> 
> - `x = p1.x`: Starting x-coordinate at the top of the edge.
> - `mi`: This is the **inverse slope** (`Δx / Δy`).  
>   **Why inverse?** Because we are stepping **one pixel in y** each time, and we want to know **how much x changes** per y step.
> 
> ```c
>     for (int y = p1.y; y < p2.y; y++) {
>         if (x < T[y].x_left)  T[y].x_left = x;
>         if (x > T[y].x_right) T[y].x_right = x;
>         x += mi; // Increment x by inverse slope
>         y++;     // ← BUG!
>     }
> ```
> 
> **This loop has a serious bug!**
> 
> **Correct version should be:**
> 
> ```c
>     for (int y = p1.y; y < p2.y; y++) {
>         if (x < T[y].x_left)  T[y].x_left = x;
>         if (x > T[y].x_right) T[y].x_right = x;
>         x += mi;
>     }
> ```
> 
> **Explanation of correct logic:**
> - For each integer y from the top to the bottom of the edge:
>   - Update `x_left` and `x_right` for that row if this edge gives a more extreme value.
>   - Then move `x` to the next row by adding `mi`.
> 
	
> [!bug] The extra `y++` from the lecture inside the loop will cause it to skip every other row — this is **incorrect**.
	
	
**Polygon To Table :**
```cpp
void Polygon_to_Table(MyPoint p[] , int n , table t){
	myPoint v1 = p[n-1];
	for (int i = 0; i < n ; i++){
		myPoint v2 = p[i];
		EdgeToTable(v1 , v2,t);
		v1 = v2 ;	
	}
}
```
**Purpose**:  
This function takes the entire polygon (array of points) and sends *every edge* to the Edge Table using the `EdgeToTable` function we explained earlier.
> [!info]- Explanation
> 
> - `myPoint v1 = p[n-1];`  
>   → Initializes `v1` as the **last point** of the polygon.  
>   This is done because polygons are closed shapes — the last point connects back to the first point.
> 
> - `for (int i = 0; i < n ; i++)`  
>   → Loops through all vertices of the polygon.
> 
> - `myPoint v2 = p[i];`  
>   → Takes the current point as `v2`.
> 
> - `EdgeToTable(v1 , v2, t);`  
>   → Sends the edge from `v1 → v2` to be processed into the scanline table.  
>   This is where the DDA-style calculation happens for that edge.
> 
> - `v1 = v2 ;`  
>   → Moves `v1` forward to the current point, so in the next iteration the new edge will be `v2 → next point`.
> 
> **How it works visually** (example with 4 points):
> - Iteration 0: Edge = p[3] → p[0]
> - Iteration 1: Edge = p[0] → p[1]
> - Iteration 2: Edge = p[1] → p[2]
> - Iteration 3: Edge = p[2] → p[3]
> 
	
This technique (using `v1` and `v2`) is a common clean way to process all edges of a closed polygon.
	
	
**Render from table**:
```c
void TableToScreen(HDC hdc, EdgeTableEntry T[], Color c) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        if (T[y].x_left < T[y].x_right) {
            DrawLine(hdc, ceil(T[y].x_left), y, floor(T[y].x_right), y, c);
        }
    }
}
```
**Purpose**:  
Draws the actual filled polygon on the screen using the data built in the Edge Table.
> [!info]- Explanation 
> - `for (int y = 0; y < SCREEN_HEIGHT; y++)`  
>   Loops through **every scanline** (every row) on the screen.
> 
> - `if (T[y].x_left < T[y].x_right)`  
>   Checks if this row was touched by any edge.  
>   If `x_left` is still `INT_MAX` and `x_right` is `-INT_MAX`, this condition will be **false** → row is outside the polygon.
> 
> - `DrawLine(hdc, ceil(T[y].x_left), y, floor(T[y].x_right), y, c);`  
>   Draws one horizontal line at row `y`, from left x to right x, with the given color.
> 
	
	
```cpp
void ConvexPolyFill (HDC hdc , MyPoints p[] , int n , COLORFUL c ){
	Tabl e t;
	Init(t);
	Polygon_to_Table(p,n,t);
	TableToScreen(hdc , t,c );
}
```
**Purpose**:  
This is the **high-level function** you call to fill a convex polygon. It orchestrates everything.
	![[image-1.png]]
> [!info]- Explanation
> - `Table t;`  
>   Declares the Edge Table (array of `EdgeTableEntry`).
> 
> - `Init(t);`  
>   Initializes the table — sets all `x_left = INT_MAX` and `x_right = -INT_MAX`.
> 
> - `Polygon_to_Table(p, n, t);`  
>   Processes **all edges** of the polygon and fills the table with intersection data.
> 
> - `TableToScreen(hdc , t, c );`  
>   Renders the filled polygon on the screen using the prepared table.
> 
> **Note**: There is a small typo in the code you posted:  
> `Tabl e t;` → should be `Table t;`
> 
	
	
**Why `ceil` and `floor`?**
- `ceil(x_left)` — round UP to ensure the point is **inside** the polygon
- `floor(x_right)` — round DOWN for the same reason

---

### Step 2B: Non-Convex Polygon Fill (Active Edge List)

**The Problem:** For ==concave polygons==, a single horizontal scanline can intersect the polygon boundary multiple times (not just twice like in convex shapes). We need a smarter algorithm to handle multiple entry/exit pairs.

**The Solution:** We use the **Active Edge List ($AEL$)** algorithm, which maintains a dynamic list of edges currently intersecting each scanline.


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

---

## Summary Table: Which Algorithm When?

| Situation | Algorithm | Why |
|-----------|-----------|-----|
| Irregular shape, unknown boundary | Flood Fill (non-recursive) | Flexible, works with any boundary |
| Convex polygon | Scanline with Edge Table | Simple, fast, exactly 2 intersections |
| Non-convex polygon | Scanline with Active Edge List | Handles multiple intersections |
| Triangle with gradients/textures | Barycentric Fill | Natural interpolation of vertex attributes |

---

> [!tldr]- ## Critical Exam/Implementation Warnings
> 1. **🔴 Recursive flood fill will crash for large shapes** — always use explicit stack/queue in production
> 2. **🔴 Forgetting to sort the Active Edge List by x** produces completely wrong filling
> 3. **🟡 Off-by-one errors at polygon vertices** are the #1 bug in scan-line implementations
> 

