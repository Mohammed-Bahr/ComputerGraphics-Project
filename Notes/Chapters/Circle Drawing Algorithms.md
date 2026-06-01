---
Created: " 2026-03-24 15:22"
Modified: 2026-03-24 15:22
tags:
  - FACL
  - ComputerGraphics
---
> [!quote] It is the quality of our work which will please God, not the quantity.
> — Mahatma Gandhi

## Learning Objectives

1. Explain the mathematical foundations of circle representation
2. Apply [[8-way symmetry]] to optimize circle drawing
3. Implement four different circle drawing algorithms
4. Analyze the trade-offs between algorithms (speed vs. accuracy)
5. Understand why Bresenham's algorithm is preferred in practice

---

## Table of Contents

1. [[Circle Drawing Algorithms#Section 1 Mathematical Foundations|Section 1: Mathematical Foundations]] - Circle Equations and Why They Matter
2. [[Circle Drawing Algorithms#Section 2 Circle Symmetry|Section 2: Circle Symmetry]] - The Magic of 8-Way Symmetry
3. [[Circle Drawing Algorithms#Section 3 Direct Cartesian Method|Section 3: Direct Cartesian Method]] - The Naive Approach
4. [[Circle Drawing Algorithms#Section 4 Polar Methods|Section 4: Polar Methods]] - Using Angles to Draw Circles
5. [[Circle Drawing Algorithms#Section 5 Bresenham's Algorithm|Section 5: Bresenham's Algorithm]] - The Gold Standard
6. [[Circle Drawing Algorithms#Section 6 Algorithm Comparison|Section 6 Algorithm Comparison]] - Comprehensive Performance Comparison

---

## Section 1: Mathematical Foundations

### Circle Equations and Why They Matter

Before we can draw a circle on a computer screen, we need to understand what a circle actually is from a mathematical perspective. A circle is defined as the set of all points that are at a fixed distance (the radius) from a fixed point (the center). This seemingly simple definition gives rise to two powerful mathematical representations that form the foundation of all circle drawing algorithms.

### 1.1 The Cartesian Equation

The Cartesian equation of a circle is derived directly from the distance formula. If the center is at point (x<sub>c</sub>, y<sub>c</sub>) and the radius is R, then every point (x, y) on the circle satisfies the equation:
$$(x - x_c)^2 + (y - y_c)^2 = R^2$$

This equation tells us that the sum of the squared distances from any point on the circle to the center, measured along the x and y axes, always equals the square of the radius.
Solving for y, we get:
$$y = y_c \pm \sqrt{R^2 - (x - x_c)^2}$$

This formula allows us to calculate the y-coordinate for any given x-coordinate. However, notice that we need to compute a square root for every point, which is computationally expensive. The square root operation is one of the most time-consuming arithmetic operations a computer can perform, making this approach potentially slow for real-time graphics applications.

> **💡 Intuition - Why This Matters:**
> Think of the Cartesian equation as a "translator" that converts x-values into y-values. It is like having a rule that says: "If you tell me how far right you want to go (x), I will tell you how far up you need to be (y) to stay on the circle." The square root is the price we pay for this translation - it is the mathematical operation that converts the squared distance back to actual distance. In computer graphics, avoiding square roots is often the key to faster algorithms.

### 1.2 The Parametric (Polar) Equation

The parametric equation takes a completely different approach. Instead of relating x and y directly, it introduces a third variable θ (theta), which represents an angle. The equations are:

$$x = x_c + R \cdot \cos(\theta)$$
$$y = y_c + R \cdot \sin(\theta)$$

As θ varies from 0 to 2π radians (or 0 to 360 degrees), the point (x, y) traces out the entire circle.

The beauty of the parametric form is that x and y are expressed independently in terms of θ. This means we can compute both coordinates simultaneously and uniformly sample points around the circle. However, computing sine and cosine functions is also expensive, as these are typically calculated using infinite series expansions:

$$\sin(\theta) = \theta - \frac{\theta^3}{3!} + \frac{\theta^5}{5!} - ...$$
$$\cos(\theta) = 1 - \frac{\theta^2}{2!} + \frac{\theta^4}{4!} - ...$$

### 1.3 Choosing the Right Octant

An octant is one-eighth of a circle. When drawing circles, we do not compute all points directly. Instead, we compute points in one octant and use symmetry to generate the rest. But which octant should we choose? The key insight comes from analyzing the slope of the circle.

Taking the derivative of the circle equation with respect to x gives us the slope:

$$\frac{dy}{dx} = -\frac{x}{y}$$

For the slope to have magnitude ≤ 1, we need |x/y| ≤ 1, which means |x| ≤ |y|. Therefore, we compute points in the octant where x ≤ y, starting from (0, R) and continuing until x = y.


---

## Section 2: Circle Symmetry

### The Magic of 8-Way Symmetry

One of the most elegant optimizations in computer graphics is the exploitation of circle symmetry. A circle centered at the origin exhibits 8-way symmetry, meaning that if you know one point (a, b) on the circle, you automatically know seven other points. This property reduces the computational burden by a factor of eight, making circle drawing algorithms significantly faster.

### 2.1 Understanding the Eight Symmetric Points

If point (a, b) lies on a circle centered at the origin with radius R, then a² + b² = R². Due to the properties of squaring (both positive and negative values give the same square), we can manipulate the signs and positions of a and b in eight different ways while still satisfying the circle equation.

**The eight symmetric points are:**
1. (a, b) - original
2. (-a, b) - reflected across y-axis
3. (-a, -b) - reflected across origin
4. (a, -b) - reflected across x-axis
5. (b, a) - swapped coordinates
6. (-b, a) - swapped and reflected y-axis
7. (-b, -a) - swapped and reflected origin
8. (b, -a) - swapped and reflected x-axis

Let us understand why this works. The first four points come from sign changes: if (a, b) is on the circle, then so are (-a, b), (-a, -b), and (a, -b). The next four points come from swapping x and y coordinates: (b, a) and its three sign variations. All eight points maintain the same distance R from the origin because:
$$a^2 + b^2 = (-a)^2 + b^2 = b^2 + a^2 = R^2$$

### Symmetry Relationships Table

| Original Point | Transformation | Resulting Point |
|----------------|----------------|-----------------|
| (a, b) | No change (original) | (a, b) |
| (a, b) | Reflect across y-axis | (-a, b) |
| (a, b) | Reflect across origin | (-a, -b) |
| (a, b) | Reflect across x-axis | (a, -b) |
| (a, b) | Swap x and y | (b, a) |
| (a, b) | Swap and reflect y-axis | (-b, a) |
| (a, b) | Swap and reflect origin | (-b, -a) |
| (a, b) | Swap and reflect x-axis | (b, -a) |

### 2.2 Handling General Circle Centers

When the circle center is at a general point (x<sub>c</sub>, y<sub>c</sub>) rather than the origin, we simply add the center coordinates to each symmetric point. So the eight points become:

- (x<sub>c</sub>+a, y<sub>c</sub>+b), (x<sub>c</sub>-a, y<sub>c</sub>+b)
- (x<sub>c</sub>-a, y<sub>c</sub>-b), (x<sub>c</sub>+a, y<sub>c</sub>-b)
- (x<sub>c</sub>+b, y<sub>c</sub>+a), (x<sub>c</sub>-b, y<sub>c</sub>+a)
- (x<sub>c</sub>-b, y<sub>c</sub>-a), (x<sub>c</sub>+b, y<sub>c</sub>-a)

This is equivalent to first computing points on a circle centered at the origin, then translating (shifting) all points by the center coordinates.

> **💡 Intuition - Why This Saves Work:**
> Imagine you need to draw a circular fence around a garden. Without symmetry, you would need to walk around the entire circle, measuring and placing posts. With 8-way symmetry, you only need to carefully measure and place posts in one-eighth of the circle (45 degrees), then use a mirror technique to instantly know where the other posts go. You have reduced your work by **87.5%**! This is exactly what computers do - calculate points carefully in one octant, then "mirror" them to fill the rest.


### 2.3 The Draw8Points Function

The Draw8Points function is the practical implementation of 8-way symmetry. It takes a single computed point (a, b) relative to the origin and draws all eight symmetric points by applying sign changes and coordinate swaps, then shifting by the center (x<sub>c</sub>, y<sub>c</sub>).

```cpp
void Draw8Points(HDC hdc, int xc, int yc, int a, int b, COLORREF color)
{
    // Points from (a,b): sign changes on both coordinates
    SetPixel(hdc, xc+a, yc+b, color);    // Quadrant I
    SetPixel(hdc, xc-a, yc+b, color);    // Quadrant II
    SetPixel(hdc, xc-a, yc-b, color);    // Quadrant III
    SetPixel(hdc, xc+a, yc-b, color);    // Quadrant IV
    // Points from swapped (b,a): sign changes on swapped coordinates
    SetPixel(hdc, xc+b, yc+a, color);    // Octant between I and II
    SetPixel(hdc, xc-b, yc+a, color);    // Octant between II and III
    SetPixel(hdc, xc-b, yc-a, color);    // Octant between III and IV
    SetPixel(hdc, xc+b, yc-a, color);    // Octant between IV and I
}
```

---

## Section 3: Direct Cartesian Method

### The Naive Approach - Understanding the Basics

The Direct Cartesian method is the most straightforward approach to drawing circles - it directly applies the Cartesian equation we learned earlier. While not the most efficient method, understanding it is crucial because it reveals the fundamental challenges that more sophisticated algorithms address. This method is often called the "naive" approach because it solves the problem directly without any clever optimizations.

### 3.1 Algorithm Overview

The algorithm works as follows: Starting from x = 0 (the top of the circle), we increment x step by step. For each x value, we calculate y using the equation:

$$y = \sqrt{R^2 - x^2}$$

We continue until x equals y (the diagonal line where our chosen octant ends). Each computed point is passed to Draw8Points to exploit symmetry.

The choice of stopping when x = y is deliberate. This is the boundary of the octant where the slope magnitude equals 1. Beyond this point, the slope would exceed 1 in magnitude, meaning that incrementing x by 1 could cause y to change by more than 1, potentially leaving gaps in the drawn circle. By stopping at x = y, we ensure smooth, gap-free drawing within our octant.

### 3.2 Implementation Details

The implementation is remarkably simple. We precompute R² to avoid recalculating it in every iteration. For each x from 0 to the point where x equals y, we compute y = √(R² - x²) and round to the nearest integer. The rounding is necessary because pixel coordinates must be integers, and it introduces a small amount of error that accumulates around the circle.

```cpp
void CircleDirect(HDC hdc, int xc, int yc, int R, COLORREF color)
{
    int x = 0, y = R;
    int R2 = R * R;    // Precompute R-squared
    Draw8Points(hdc, xc, yc, x, y, color);
    while(x < y)
    {
        x++;
        y = round(sqrt((double)(R2 - x*x)));
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}
```

> **💡 Intuition - Why This Works:**
> Think of this algorithm as "walking" along the top of the circle from left to right. At each step, you move one unit to the right (increment x), then look up how far down you need to go (calculate y using the circle equation). The square root tells you the exact distance from the horizontal line y = R to the circle at position x. The result is rounded to find the nearest pixel. While this sounds perfectly logical, the square root operation is like asking a very complex question every time you take a step - it works, but it is slow.

### 3.3 Performance Analysis

The Direct Cartesian method has several performance characteristics that make it suboptimal for real-time graphics.

| Characteristic | Direct Cartesian | Impact |
|----------------|------------------|--------|
| Operations per pixel | 1 sqrt, 1 multiply, 1 subtract | Slow due to sqrt |
| Integer operations | No (uses floating-point) | Additional overhead |
| Memory accesses | Minimal | Good cache performance |
| Accuracy | Good (rounding errors only) | Acceptable for most uses |

---


## Section 4: Polar Methods — Drawing Circles Using Angles

Instead of calculating y from x (like Cartesian methods), polar methods use an **angle θ** that sweeps around the circle, generating points from these parametric equations:

$$x = R\cos\theta \qquad y = R\sin\theta$$

---

### 4.1 Direct Polar Method

You start at θ = 0 (the point (R, 0)) and keep adding a small angle step **dθ** to get the next point.

**How big should dθ be?**

- Too large → gaps between points
- Too small → multiple steps land on the same pixel (wasted work)

The sweet spot: one step should move you **~1 pixel** along the arc. Since arc length = R·dθ, setting it to 1 pixel means:

$$d\theta = \frac{1}{R}$$

> **Intuition:** A small circle curves sharply → you need a bigger angle step. A large circle curves gently → you need a smaller step. That's why dθ shrinks as R grows.

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

**Problem:** `cos()` and `sin()` are called on **every iteration** — they're expensive (computed via infinite series internally).

---

### 4.2 Iterative Polar Method

**The key idea:** Instead of recomputing cos(θ) and sin(θ) from scratch each time, derive the **next point from the current point** using the angle addition formulas:$$\cos(\theta + d\theta) = \cos\theta\cos d\theta - \sin\theta\sin d\theta$$$$\sin(\theta + d\theta) = \sin\theta\cos d\theta + \cos\theta\sin d\theta$$Since the current point is (x, y) = (R·cosθ, R·sinθ), this becomes:$$x' = x\cos(d\theta) - y\sin(d\theta)$$$$y' = x\sin(d\theta) + y\cos(d\theta)$$Since dθ is **constant**, you compute `cos(dθ)` and `sin(dθ)` **only once**, then every new point costs just **4 multiplications + 2 additions** — no trig calls in the loop at all.

```cpp
void CircleIterativePolar(HDC hdc, int xc, int yc, int R, COLORREF color) {
    double x = R, y = 0;
    double dtheta = 1.0 / R;
    double c = cos(dtheta), s = sin(dtheta);  // Computed ONCE
    Draw8Points(hdc, xc, yc, R, 0, color);
    while (x > y) {
        double x1 = x*c - y*s;   // Rotate current point by dθ
        y      = x*s + y*c;
        x      = x1;
        Draw8Points(hdc, xc, yc, round(x), round(y), color);
    }
}
```

> **Intuition:** This is literally **rotating a point** by a tiny fixed angle each step — like swinging a string of length R in equal small steps. The rotation formula is the same standard 2D rotation matrix applied repeatedly, with the cos/sin precomputed as a one-time "template."

---

### Comparison at a Glance

|                   | Direct Polar                          | Iterative Polar                  |
|-------------------|---------------------------------------|-----------------------------------|
| **Per-step cost** | `cos()` + `sin()` every time          | 4 multiplications + 2 additions   |
| **Setup cost**    | None                                  | Compute `cos(dθ)`, `sin(dθ)` once |
| **Speed**         | Slower                                | Much faster                       |
| **Accuracy**      | Both drift slightly over time         | Similar                           |

The iterative version is widely used in real-time systems like **CNC machines, 3D printers, and robotic arms**, where repeated trig calls would be too slow for embedded microcontrollers.

---

> [!warning] Common Mistakes with Polar Methods
> - **Choosing dθ too large** → Gaps appear between pixels, creating a dashed circle
> - **Choosing dθ too small** → Multiple iterations land on the same pixel, wasting computation
> - **Forgetting to round** → Floating-point coordinates must be rounded to integers for pixel placement
> - **Not using 8-way symmetry** → Computing all 360° instead of just one octant wastes 87.5% of computation

---

## Section 5: Bresenham’s (Midpoint) Circle Algorithm

### What the algorithm is

- **Bresenham's circle algorithm** is the standard integer‑only method for drawing good‑quality circles in raster graphics.
    
- Uses **no square roots, no trig functions**, only integer addition, subtraction, and multiplication by small constants.
    
- Works by deciding, at each step, **which of two candidate pixels is closer** to the true circle boundary.
    
### 5.1 The Core Idea: Midpoint Testing

The key insight behind Bresenham's algorithm is elegantly simple. Instead of calculating exactly where the circle passes, we make a binary decision at each step: which of two candidate pixels is closer to the true circle? 

We do this by testing a "midpoint" - a point halfway between the two candidates - to see if it lies inside or outside the circle.

Consider we have just drawn a pixel at position (x, y) in our octant. The circle is moving downward and to the right. The next pixel must be either at:
- From pixel \((x, y)\), the next pixel is either:
		
    - **(x+1, y)** → move right, stay at same height, or
        
    - **(x+1, y-1)** → move right and down.
	
- You test the **midpoint** at $(x+1,y−0.5)$:
		
    - If midpoint is **inside** the circle → choose **(x+1, y)**.
        
    - If midpoint is **outside** the circle → choose **(x+1, y-1)**.
      
> **Intuition (fence analogy):**  
   You’re placing fence posts along a curved property line; you can’t see the exact line, but you can test the midpoint between the two next‑post options. If the midpoint is “on your side,” the curve is farther away; if it’s “on the neighbor’s side,” the curve is closer and you move down.

### 5.2 The Circle Test Function

To test whether a point is inside or outside the circle, we use the circle function:
$$F(x, y) = x^2 + y^2 - R^2$$

For a circle centered at the origin with radius R:
- If **F(x, y) < 0** → point (x,y) is **inside** the circle
- If **F(x, y) > 0** → point (x,y) is **outside** the circle
- If **F(x, y) = 0** → point (x,y) is exactly **on** the circle (rare for integer pixels)

For our midpoint test at (x+1, y-0.5), we define a *decision variable*:
$$d = F(x+1, y-0.5) = (x+1)^2 + (y-0.5)^2 - R^2$$
This expands to:
$$d = x^2 + 2x + 1 + y^2 - y + 0.25 - R^2$$
> The 0.25 is a fraction we would rather avoid. **removing 0.25 does not change the sign of d**, and the sign is all we care about! So we use:
$$d = x^2 + 2x + 1 + y^2 - y - R^2$$
### 5.3 Initial Decision Variable
Starting point is (0, R). First midpoint is at (1, R−0.5): $$d_{init} = 1 + (R-0.5)^2 - R^2 = 1.25 - R \approx \boxed{1 - R}$$
### Draft Midpoint Circle Algorithm

```C++
// Draft algorithm
int x = 0;
int y = R;

Draw8Points(hdc, xc, yc, x, y, c);

while (x < y)
{
    // The midpoint decision variable calculation
    d = pow(x + 1, 2) + pow(y - 0.5, 2) - pow(R, 2); 
    
    // If the midpoint is outside the circle, move the y coordinate down
    if (d >= 0) y--; 
    
    // Always step x to the right
    x++; 
    
    // Draw the reflected points in all 8 octants
    Draw8Points(hdc, xc, yc, x, y, c); 
}
```

 
> [!note]- How it relates to the text:
> 
> the Decision variable $d$ calculates exactly as $F(x+1, y-0.5) = (x+1)^2 + (y-0.5)^2 - R^2$.
> 
> this version is conceptually perfect but not yet fully optimized for computer graphics because it still relies on floating-point math (the `0.5`) and expensive squaring operations.
> 

### 5.4 Iterative Decision Variable <font color="#d83931">Update</font>

The true genius of Bresenham's algorithm is that we update d iteratively, never computing squares from scratch. After drawing pixel (x, y) with decision variable d, we need the new decision variable d' for the next midpoint.

> *Note* : In both cases: `x++` after the update.

#### Case 1: d < 0 (midpoint inside circle, choose pixel (x+1, y))

The next midpoint is at (x+2, y-0.5) That means we moving to the *Right* :
$$d_{new} = (x+2)^2 + (y - 0.5)^2 $$
$$\Delta d = d_{new} - d = 2x + 3$$
$$d_{new} = d + 2x + 3$$

#### Case 2: d ≥ 0 (midpoint outside or on circle, choose pixel (x+1, y-1))

The next midpoint is at (x+2, y-1.5) , That's means we moving to *Left* .$$d_{new}=(x+2)^2+(y-\frac{3}{2})^2-R^2$$$$\Delta d = d_{new} - d = 2(x-y) + 5$$$$d_{new} = d + 2(x-y) + 5$$ (and we also decrement y)

### 5.5 Standard Bresenham Algorithm

```cpp
void CircleBresenham(HDC hdc, int xc, int yc, int R, COLORREF color)
{
    int x = 0, y = R;
    int d = 1 - R;    // Initial decision variable
    Draw8Points(hdc, xc, yc, x, y, color);
    while(x < y)
    {
        if(d < 0)
            d += 2*x + 2;        // Move right: update d
        else
        {
            d += 2*(x - y) + 5;  // Move right-down: update d
            y--;                  // And decrement y
        }
        x++;
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}
```

### 5.6 The Faster Bresenham Variant, Based on $2^{nd}$ order difference .
 
We can make the algorithm even faster by computing the increment values iteratively. Define:
- **ch1 = 2x + 3** (the increment when d < 0)
- **ch2 = 2(x-y) + 5** (the increment when d ≥ 0)

Instead of recalculating these from scratch, we update them iteratively:
![[FACL/ComputerGraphics/Chapters/LEC3/Attachments/image.png]]
- **Initially:** ch1 = 3 (when x = 0) and ch2 = 5 - 2R (when x = 0, y = R)
- After each step, x increases by 1, so **ch1 increases by 2**
- If we moved right-down (d ≥ 0), y also decreases by 1, so **ch2 increases by 4**
- If we moved right only (d < 0), y stays the same, so **ch2 increases by 2**

```cpp
void CircleFasterBresenham(HDC hdc, int xc, int yc, int R, COLORREF color)
{
    int x = 0, y = R;
    int d = 1 - R;
    int c1 = 3, c2 = 5 - 2*R;    // Increment values
    Draw8Points(hdc, xc, yc, x, y, color);
    while(x < y)
    {
        if(d < 0)
        {
            d += c1;
            c2 += 2;
        }
        else
        {
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

> [!warning] Common Mistakes with Bresenham's Algorithm
> - **Wrong initial value for d** → Using `1 - R` (standard) vs `1.25 - R` (exact). The rounded version is standard.
> - **Forgetting to decrement y** → When `d ≥ 0`, you must do `y--` or the circle won't curve correctly.
> - **Mixing up increment formulas** → `2x + 3` is for movement right only, `2(x-y) + 5` is for right-down movement.
> - **Wrong stopping condition** → Use `while(x < y)`, not `while(x <= y)`, to avoid duplicate pixels at the diagonal.
> - **Not using 8-way symmetry** → Only compute one octant and use `Draw8Points()` to generate the rest.

### 5.7 Worked Example: Tracing R = 8

Let's trace through Bresenham's algorithm step by step for a circle of radius 8 to see how the decision variable evolves.

**Initial values:** x = 0, y = 8, d = 1 - 8 = -7

| Step | x | y | d (before) | Decision | Increment Used | d (after) | 8 Points Drawn |
|------|---|---|------------|----------|----------------|-----------|----------------|
| 0 | 0 | 8 | - | - | - | -7 | (0,8) and symmetric points |
| 1 | 1 | 8 | -7 | d < 0 | 2·0 + 3 = 3 | -4 | (1,8) + symmetric |
| 2 | 2 | 8 | -4 | d < 0 | 2·1 + 3 = 5 | 1 | (2,8) + symmetric |
| 3 | 3 | 7 | 1 | d ≥ 0 | 2(2-8) + 5 = -7 | -6 | (3,7) + symmetric |
| 4 | 4 | 7 | -6 | d < 0 | 2·3 + 3 = 9 | 3 | (4,7) + symmetric |
| 5 | 5 | 7 | 3 | d ≥ 0 | 2(4-7) + 5 = -1 | 2 | (5,7) + symmetric |
| 6 | 6 | 6 | 2 | d ≥ 0 | 2(5-7) + 5 = 1 | 3 | (6,6) + symmetric |

**Stop:** x = 6, y = 6 → condition `x < y` is now false (6 < 6 is false).

> [!tip] Understanding the Trace
> Notice how the decision variable `d` oscillates between negative and positive values. When `d < 0`, we move horizontally (staying at the same y), and when `d ≥ 0`, we move diagonally down (decrementing y). This creates a smooth approximation of the circle's curvature in the first octant.

---

## Section 6: Algorithm Comparison

### Comprehensive Performance Comparison

| Algorithm | Operations/Step | Floating-Point? | Trig Functions? | Speed | Best Use Case |
|-----------|-----------------|-----------------|------------------|-------|---------------|
| **Direct Cartesian** | 1 sqrt, 1 subtract | Yes | No | Slowest | Educational, simplicity |
| **Direct Polar** | cos + sin per pixel | Yes | Yes (every iteration) | Slow | Uniform sampling needed |
| **Iterative Polar** | 4 mul + 2 add | Yes | Yes (once at start) | Faster | CNC, robotics, real-time |
| **Standard Bresenham** | 2-5 integer ops | No | No | Fast | General-purpose graphics |
| **Faster Bresenham** | 2-4 integer ops | No | No | Fastest | Embedded systems, games |

### When to Use Which Algorithm

> [!tip] Algorithm Selection Guide
> - **Learning/Teaching:** Direct Cartesian or Direct Polar — easiest to understand
> - **Real-time graphics/games:** Standard Bresenham — excellent speed/accuracy balance
> - **Embedded systems (no FPU):** Faster Bresenham — pure integer arithmetic, fastest
> - **CNC/robotics (uniform arc length):** Iterative Polar — consistent angular steps
> - **Need sub-pixel precision:** Direct Polar with anti-aliasing

---

## Summary & Key Takeaways

### Core Concepts

1. **8-Way Symmetry is the Key Optimization** — A circle has inherent symmetry: compute points in one octant (45° arc), then reflect to fill all 8 octants. This reduces computation by **87.5%**.

2. **Bresenham's Algorithm Wins Through Incremental Updates** — Instead of computing square roots or trigonometric functions, it maintains a decision variable `d` that updates incrementally. Only **integer addition, subtraction, and small multiplications** are needed.

3. **The Midpoint Test Drives Pixel Selection** — At each step, we test whether the midpoint between two candidate pixels lies inside or outside the circle. This binary decision determines which pixel to choose.

4. **Float-Free is Fast** — The primary reason Bresenham dominates in practice: **no floating-point operations**. This matters on embedded systems, GPUs, and any hardware where floating-point is expensive.

5. **Trade-off Spectrum: Accuracy vs. Speed** — Direct methods (Cartesian, Polar) are accurate but slow due to sqrt/trig. Bresenham achieves similar visual quality with dramatic speedup by accepting small rounding errors.

6. **Choosing the Correct Octant** — We compute points where `|x| ≤ |y|` (slope magnitude ≤ 1) because this ensures incrementing x by 1 never causes y to change by more than 1, preventing gaps in the drawn circle.

---

