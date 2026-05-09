# Algorithm Implementation Notes

This document explains the mathematical foundations and implementation details of the three algorithms Person 3 is responsible for.

---

## 1. DDA Line Algorithm (Digital Differential Analyzer)

### Mathematical Foundation

The line equation connecting two points (x₁, y₁) and (x₂, y₂):

```
y - y₁     y₂ - y₁
------- = --------
x - x₁     x₂ - x₁
```

Rearranged: `y = y₁ + (x - x₁) × m` where `m = (y₂ - y₁) / (x₂ - x₁)`

### The Incremental Approach

Instead of calculating the full equation for each pixel, we use **incremental addition**:

- If we move x by 1, y changes by exactly m (the slope)
- Formula: `Δy = m × Δx`
- When Δx = 1: `Δy = m`

### Algorithm Logic

**For Gentle Slopes (|m| ≤ 1):**
- Step along x-axis (increment x by 1 each iteration)
- Calculate y incrementally: `y = y + m`
- Round y to nearest integer for pixel placement

**For Steep Slopes (|m| > 1):**
- Step along y-axis (increment y by 1 each iteration)
- Calculate x incrementally: `x = x + (1/m)`
- Round x to nearest integer for pixel placement

### Why This Works

Addition is **much faster** than multiplication for computers. By calculating:
- `y_new = y_old + m` instead of
- `y_new = y₁ + (x - x₁) × m`

We eliminate repeated multiplication, making the algorithm efficient.

### Code Implementation

```cpp
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (abs(dy) <= abs(dx)) {
        // Gentle slope: step along x
        double m = (double)dy / dx;
        if (x1 > x2) swap(x1, y1, x2, y2);
        
        int x = x1;
        double y = y1;
        SetPixel(hdc, x, y, c);
        
        while (x < x2) {
            x++;
            y += m;  // Incremental addition
            SetPixel(hdc, x, Round(y), c);
        }
    }
    else {
        // Steep slope: step along y
        double mi = (double)dx / dy;
        if (y1 > y2) swap(x1, y1, x2, y2);
        
        int y = y1;
        double x = x1;
        SetPixel(hdc, Round(x), y, c);
        
        while (y < y2) {
            y++;
            x += mi;  // Incremental addition
            SetPixel(hdc, Round(x), y, c);
        }
    }
}
```

### Performance
- **Time Complexity:** O(max(|dx|, |dy|)) - one iteration per pixel
- **Space Complexity:** O(1) - only a few variables
- **Operations per pixel:** 1 addition, 1 rounding
- **Uses floating-point:** Yes (slightly slower than Bresenham's)

---

## 2. Midpoint Line Algorithm (Bresenham's)

### The Core Idea

Instead of using floating-point arithmetic, we use a **decision variable** to determine which pixel is closer to the true line.

### The Line Discriminant Function

For a line from (x₁, y₁) to (x₂, y₂):

```
f(x, y) = (y - y₁)(x₂ - x₁) - (x - x₁)(y₂ - y₁)
```

Properties:
- `f(x, y) = 0`: point is **on** the line
- `f(x, y) > 0`: point is **above** the line
- `f(x, y) < 0`: point is **below** the line

### The Midpoint Test

At each step, we test the midpoint between two candidate pixels:

For pixel (x, y), the next pixel is either:
- (x+1, y) - move right
- (x+1, y+1) - move right and up

Test point: (x+1, y+0.5)

### Decision Variable

```
d = f(x+1, y+0.5)
```

Expanded:
```
d = (y + 0.5 - y₁)(x₂ - x₁) - (x + 1 - x₁)(y₂ - y₁)
```

Multiply by 2 to eliminate the 0.5 fraction:
```
d = (2y + 1 - 2y₁)(x₂ - x₁) - 2(x + 1 - x₁)(y₂ - y₁)
```

### Incremental Updates

Instead of recalculating d, we update it:

**Initial value:**
```
d_init = dx - 2dy  (for 0 ≤ slope ≤ 1)
```

**Update rules:**
- If d < 0 (midpoint below line): 
  - Move right and up (y++)
  - Update: `d = d + 2dx - 2dy`
  
- If d ≥ 0 (midpoint above line):
  - Move right only
  - Update: `d = d - 2dy`

### Optimization: Pre-compute Deltas

```cpp
delta_d1 = 2*dx - 2*dy;  // Update when moving diagonally
delta_d2 = -2*dy;        // Update when moving horizontally
```

### Code Implementation

```cpp
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;  // Direction
    int sy = (y1 < y2) ? 1 : -1;
    
    int x = x1, y = y1;
    
    if (dx >= dy) {
        // Gentle slope
        int d = dx - 2 * dy;
        int delta_d1 = 2 * dx - 2 * dy;
        int delta_d2 = -2 * dy;
        
        SetPixel(hdc, x, y, c);
        
        while (x != x2) {
            if (d < 0) {
                y += sy;
                d += delta_d1;
            } else {
                d += delta_d2;
            }
            x += sx;
            SetPixel(hdc, x, y, c);
        }
    }
    else {
        // Steep slope (symmetric logic)
        // ... similar but step along y
    }
}
```

### Performance
- **Time Complexity:** O(max(|dx|, |dy|))
- **Space Complexity:** O(1)
- **Operations per pixel:** 2-3 integer additions/subtractions only
- **No floating-point:** Faster than DDA
- **No multiplication in loop:** Very fast

---

## 3. Modified Midpoint Circle (Faster Bresenham)

### Circle Equation

For a circle centered at (xc, yc) with radius R:

```
(x - xc)² + (y - yc)² = R²
```

Centered at origin (shift later):
```
x² + y² = R²
```

### The Circle Test Function

```
F(x, y) = x² + y² - R²
```

Properties:
- `F(x, y) = 0`: point is **on** the circle
- `F(x, y) > 0`: point is **outside** the circle
- `F(x, y) < 0`: point is **inside** the circle

### 8-Way Symmetry

A circle has 8-way symmetry. If (x, y) is on the circle, so are:
- (x, y), (-x, y), (-x, -y), (x, -y)  ← Sign changes
- (y, x), (-y, x), (-y, -x), (y, -x)  ← Coordinate swap + signs

**Optimization:** Compute only 1/8 of the circle (one octant), reflect to get the rest.

We compute the octant where: **0 ≤ x ≤ y**

### The Midpoint Decision

From pixel (x, y), the next pixel is either:
- (x+1, y) - move right
- (x+1, y-1) - move right and down

Test the midpoint: (x+1, y-0.5)

### Decision Variable

```
d = F(x+1, y-0.5) = (x+1)² + (y-0.5)² - R²
```

### Initial Value

Starting at (0, R):
```
d_init = 1 - R
```

### Standard Bresenham Update

**If d < 0** (midpoint inside):
- Choose (x+1, y) - move right only
- Next d: `d_new = d + 2x + 3`

**If d ≥ 0** (midpoint outside):
- Choose (x+1, y-1) - move right and down
- Next d: `d_new = d + 2(x - y) + 5`
- Decrement y

### Modified (Faster) Version: 2nd Order Differences

Instead of computing `2x + 3` and `2(x-y) + 5` each time, we maintain **incremental variables**:

```
c1 = 2x + 3       (increment when d < 0)
c2 = 2(x-y) + 5   (increment when d ≥ 0)
```

**Initial values:**
```
c1 = 3            (when x=0)
c2 = 5 - 2R       (when x=0, y=R)
```

**Update rules after each step:**
- Always: `c1 += 2` (because x always increases)
- If d < 0: `c2 += 2` (y stays same)
- If d ≥ 0: `c2 += 4` (y decreases)

### Why This is Faster

Standard Bresenham computes:
```cpp
if (d < 0)
    d += 2*x + 3;
else
    d += 2*(x - y) + 5;
```
This has **multiplication** inside the loop (2*x, 2*y).

Modified version:
```cpp
if (d < 0)
    d += c1;    // Just addition
else
    d += c2;    // Just addition
```
**No multiplication in the loop!** Only simple additions.

### Code Implementation

```cpp
void DrawCircleModifiedMidpoint(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R;
    int d = 1 - R;      // Initial decision variable
    int c1 = 3;         // 2*0 + 3
    int c2 = 5 - 2*R;   // 2*(0-R) + 5
    
    Draw8Points(hdc, xc, yc, x, y, color);
    
    while (x < y) {
        if (d < 0) {
            // Midpoint inside: move right only
            d += c1;
            c2 += 2;
        } else {
            // Midpoint outside: move right and down
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

### The Draw8Points Helper

```cpp
void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF color) {
    SetPixel(hdc, xc+x, yc+y, color);   SetPixel(hdc, xc-x, yc+y, color);
    SetPixel(hdc, xc-x, yc-y, color);   SetPixel(hdc, xc+x, yc-y, color);
    SetPixel(hdc, xc+y, yc+x, color);   SetPixel(hdc, xc-y, yc+x, color);
    SetPixel(hdc, xc-y, yc-x, color);   SetPixel(hdc, xc+y, yc-x, color);
}
```

### Performance
- **Time Complexity:** O(R) - only compute one octant
- **Space Complexity:** O(1)
- **Operations per pixel:** 2-4 integer additions only
- **No multiplication in loop:** Extremely fast
- **No floating-point:** Pure integer arithmetic
- **8x speedup from symmetry**

---

## Comparison Summary

| Feature | DDA | Midpoint Line | Modified Circle |
|---------|-----|---------------|-----------------|
| **Arithmetic** | Float | Integer only | Integer only |
| **Speed** | Moderate | Fast | Fastest |
| **Operations/pixel** | 1 add + 1 round | 2-3 int ops | 2-4 int ops |
| **Multiplication** | Once (slope) | Pre-computed | None in loop |
| **Best for** | Learning | General use | Embedded systems |

---

## Key Takeaways

1. **DDA is intuitive** - directly implements the line equation incrementally
2. **Midpoint eliminates floating-point** - uses integer decision variables
3. **Modified Circle is fastest** - pre-computes updates, uses symmetry
4. **All avoid gaps** - carefully choose which pixels to draw
5. **Symmetry is powerful** - reduces computation by 87.5% for circles

---

## Testing Your Implementation

### Test Cases for Lines

1. **Horizontal:** (0, 0) to (100, 0)
2. **Vertical:** (50, 0) to (50, 100)
3. **45° diagonal:** (0, 0) to (100, 100)
4. **Gentle slope:** (0, 0) to (100, 30)
5. **Steep slope:** (0, 0) to (30, 100)
6. **Backwards:** (100, 100) to (0, 0)
7. **Negative slopes:** (0, 100) to (100, 0)

### Test Cases for Circles

1. **Small radius:** R = 10
2. **Medium radius:** R = 50
3. **Large radius:** R = 200
4. **Radius = 1:** Edge case
5. **Off-center:** Center at (200, 150)

### Expected Results

- **No gaps** in any line or circle
- **Smooth curves** without jagged edges
- **Perfect symmetry** in circles (all 8 octants identical)
- **Consistent speed** regardless of slope/radius

---

## References

- Course LEC2.md - DDA and Midpoint Line algorithms
- Course Circle Drawing Algorithms.md - Section 5.6
- Bresenham, J. E. (1965). Algorithm for computer control of a digital plotter. IBM Systems Journal, 4(1), 25-30.
