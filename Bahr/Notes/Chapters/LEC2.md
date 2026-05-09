---
Created: " 2026-03-07 14:50"
Modified: 2026-03-24 12:09
tags:
  - FACL
  - ComputerGraphics
  - "#ComputerGraphics/DDA"
  - "#ComputerGraphics/MidPoint"
---

> [!quote] He who lives in harmony with himself lives in harmony with the universe.
> — Marcus Aurelius

**Topic:** Line Rasterization Algorithms — converting continuous line equations into discrete pixel coordinates on a raster display.

**Key Idea:** Given two points $(x_1, y_1)$ and $(x_2, y_2)$, determine which pixels to illuminate to draw a visually continuous line.
	
 the last things we talked about was :
	line rasterization (Drawing on raster scan display)
	- Equation $\rightarrow$ $$\frac{Y-Y_1}{X-X_1} = \frac {Y_2 - Y_1}{X_2 - X_1 }$$
		![[FACL/ComputerGraphics/Chapters/Attachments/image.png]]

---
### Iteration :
- **Equation** $\rightarrow$  $$\frac{Y-Y_1}{X-X_1} = \frac {Y_2 - Y_1}{X_2 - X_1 }$$
**Case 1: Gentle Slope**  <font color="#fac08f">|slope|</font> $\le$ <font color="#fac08f">1  </font>  and $X_1$  < $X_2$  :
	Here is the transcribed text:
		$X = X + 1$ ; (<font color="#4bacc6">this means</font> $\rightarrow$ $\Delta$ X = 1)
		$\Delta$ Y = m(1) = m
		for x = x₁ to x₂ step 1
			compute y = y₁ + (x - x₁)(y₂ - y₁) / (x₂ - x₁)
			$SetPixel$(x, Round(y), Color)
		end
```cpp
for (x = x1; x <= x2; x++) {
	y = y1 + (x - x1) * (y2 - y1) / (x2 - x1);
	SetPixel(x, Round(y), Color);
}
```

**Case 2: Steep Slope** <font color="#fac08f">|slope|</font> $>$ <font color="#fac08f">1  </font>  and $Y_1$  < $Y_2$    :
	$Y = Y + 1$ ; (<font color="#4bacc6">this means</font> $\rightarrow$ $\Delta$ Y = 1)
	$\Delta$ X = $\frac{1}{m}$
	for y = y₁ to y₂ step 1
		x = x₁ + (y - y₁)(x₂ - x₁) / (y₂ - y₁)
		$SetPixel$(Round(x), y, Color)
	end
Assumptions: |slope| > 1 , y₁ < y₂
```cpp
for (y = y1; y <= y2; y++) {
	x = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
	SetPixel(Round(x), y, Color);
}
```

> [!warning] Floating-point multiplication and rounding are expensive operations.
> Both the naive approach and DDA require floating-point arithmetic, which is slower than integer operations on most processors.

---
# Line Digital Differential analyser ($DDA$)

> This means i am finding the new point using older one .

- line equation :  ( $Y = mX + c$)
	- **m** $\rightarrow$ slope .
	- **c**  $\rightarrow$ represents The cut part of Y-axis (Y-intercept) .
- if we wants to calculate Two consecutive points on line :$$Y_i = mX_i + c$$$$Y_{i+1} = mX_{i+1} + c$$
- By subtract both equations we gets :$$Y_{i+1} - Y_{i} = m(X_{i+1} - X_i) \rightarrow  \Delta Y = m \Delta X$$

## The Algorithm :
```cpp
void LineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    int dx = x2 - x1, dy = y2 - y1;
    
    // Check if the line is more horizontal than vertical (|slope| <= 1)
    if (abs(dy) <= abs(dx))
    {
        double m = (double)dy / dx; 
        
        // Ensure we always draw from left to right
        if (x1 > x2) swap(x1, y1, x2, y2); 
        
        SetPixel(hdc, x1, y1, c); 
        
        int x = x1;
        double y = y1;
        
        while (x < x2)
        {
            x++;
            y += m; 
            SetPixel(hdc, x, Round(y), c);
        }
    }
    else 
	{
	    double mi = (double)dx / dy;
	    if (y1 > y2) swap(x1, y1, x2, y2);
	    
	    SetPixel(hdc, x1, y1, c);
	    
	    int y = y1;
	    double x = x1;
	    
	    while (y < y2)
	    {
	        y++;
	        x += mi;
	        SetPixel(hdc, Round(x), y, c);
	    } 
	}
}
```

### How this Algorithm Works

Instead of calculating the full equation $y = y_1 + (x - x_1) \cdot m$ from scratch for every single pixel, the DDA algorithm uses **incremental addition**. Since $x$ increases by exactly 1 in each step, $y$ must increase by exactly the slope ($m$) in each step. Addition is much faster for a computer to process than multiplication!

Here is the step-by-step breakdown: 

#### If( slope <= 1):
$$|\frac{dy}{dx}| \le 1$$

1. **Calculate the Differences:** It finds the total horizontal distance ($dx$) and vertical distance ($dy$) between the two points.
    
2. **The Slope Check:** The condition `if (abs(dy) <= abs(dx))` checks if the absolute value of the slope is $\le 1$. As noted  ($$|\frac{dy}{dx}| \le 1 \Rightarrow |dy| \le |dx|$$), this means the line is "gentle" (more horizontal than vertical). If this is true, we step through the $x$-axis.
    
3. **Calculate Slope ($m$):** It divides $dy$ by $dx$ to find the slope. It casts to `double` to prevent around number too early and loss the accuracy.
    
4. **Enforce Left-to-Right Drawing:** The `if (x1 > x2) swap(...)` line is a clever safety check. If point 1 is to the right of point 2, it swaps their coordinates so that the loop can safely assume $x$ will always increase from left to right.
    
5. **Draw the First Pixel:** It plots the very first point at $(x_1, y_1)$.
    
6. **The Loop:** It initializes $x$ and $y$. As long as $x$ hasn't reached $x_2$:
	    
    - $x$ goes up by exactly 1 (`x++`).
        
    - $y$ increases by the fractional slope (`y += m`).
        
    - It draws the pixel at the current $x$, and rounds $y$ to the nearest integer grid coordinate.
        
#### Else :
$$|\frac{dy}{dx}| > 1$$
7. **How the Steep Slope** (`else`) **Block Works**
	This part of the algorithm triggers when the absolute vertical distance is strictly greater than the horizontal distance ($|dy| > |dx|$). This means the line is steep, and if we stepped along the $x$-axis, we would leave gaps in the line.
	
	Here is the step-by-step breakdown of this section:
	
	- **Calculate the Inverse Slope (`mi`):** Instead of calculating the standard slope $m = \frac{dy}{dx}$, the code calculates the inverse slope `mi = dx / dy`. Because we are taking steps of exactly 1 along the y-axis, we need to know what fraction of a step $x$ takes for every vertical step.
	    
	- **Enforce Bottom-to-Top Drawing:** The condition `if (y1 > y2)` checks if the starting point is mathematically higher up the y-axis than the ending point. If it is, it `swap` the coordinates. This ensures the `while` loop can safely assume $y$ is always increasing.
	    
	- **Set the First Pixel:** It explicitly draws the starting pixel at `(x1, y1)` using the chosen color `c`.
	    
	- **The Y-Axis Loop:** The algorithm initializes `y` as an integer and `x` as a double. As long as `y` hasn't reached `y2`:
		    
	    - It moves exactly one pixel row up: `y++`.
	        
	    - It adjusts the exact horizontal position by the inverse slope fraction: `x += mi`.
	        
	    - It plots the pixel at the current exact `y` row, while `Round(x)` finds the closest integer pixel column.
	        
---
### Summary of DDA

By combining both parts of your code, you have a complete DDA algorithm. It smartly checks if a line is gentle or steep first, then decides whether to march along the $x$-axis (calculating $y$) or the $y$-axis (calculating $x$). This guarantees a continuous, gap-free line every single time!

**Time Complexity:** $O(\max(|dx|, |dy|))$ — one iteration per unit of the longer axis.

**Space Complexity:** $O(1)$ — only a few variables needed.

---


#  MidPoint Algorithm :

>The goal of this algorithm is to figure out exactly which pixels on a screen should be turned on to draw a straight line between two points.

To determine where pixels lie relative to the true mathematical line, we need a mathematical "detector." This is the **Line Discriminant Function**, denoted as $f(x,y)$.

**Deriving the function:**

We start with the standard equation for a line passing through points $(x_1, y_1)$ and $(x_2, y_2)$:

$$\frac{y - y_1}{x - x_1} = \frac{y_2 - y_1}{x_2 - x_1}$$

By cross-multiplying and moving everything to one side so the equation equals zero, we get the **discriminant function**:

$$f(x,y) = (y - y_1)(x_2 - x_1) - (x - x_1)(y_2 - y_1)$$

**How it works as a "detector":**

This function $f(x,y)$ divides the entire 2D space into three regions. Assuming we are drawing the line from left to right ($x_1 < x_2$, which means $x_2 - x_1 > 0$):

- **$f(x,y) = 0$**: The point $(x,y)$ is exactly **on** the line.
    
- **$f(x,y) > 0$**: The point $(x,y)$ is **above** the line.
    
- **$f(x,y) < 0$**: The point $(x,y)$ is **under** the line.
    

---

## The Midpoint Concept

**Case:** $0 \le slope \le 1$  $\rightarrow$  $x_1 < x_2$
	
**Initialization:** $x = x_1$, $y = y_1$
	
**Loop:**
	
```cpp
Loop :
	IF (x + 1, y + 1/2) is under the Line then
	    y++
	end IF
	x++
	SetPixel(x, y, color)
end Loop
```
$$\downarrow$$
The algorithm assumes a specific case: we are drawing a line from left to right ($x_1 < x_2$) with a gentle upward slope (slope between $0$ and $1$).

Because the slope is between 0 and 1, for every single step we move to the right ($x+1$), the $y$ value will either stay the same or go up by exactly $1$.

**The Question:** When we move to $x+1$, do we pick the pixel at $y$, or the pixel at $y+1$?

**The Solution:** We test the exact midpoint between those two choices, which is at coordinates $(x+1, y + \frac{1}{2})$.

---

## The Decision Variable ($d$) 

To make the choice, we plug the midpoint into our discriminant function. This creates our **decision variable**, $d$.

$$d = f(x+1, y+\frac{1}{2}) = (y + \frac{1}{2} - y_1)(x_2 - x_1) - (x + 1 - x_1)(y_2 - y_1)$$

**The Algorithm Logic:**
![[FACL/ComputerGraphics/Chapters/Attachments/image-1.png|500]]

1. **Initialization:** Start at the first point: $x = x_1$ and $y = y_1$.
    
2. **The Loop (moving pixel by pixel):**
    
    - Calculate $d$ for the next step.
        
    - **If $d < 0$:** This means the midpoint $(x+1, y+0.5)$ is _under_ the actual mathematical line. Therefore, the actual line is closer to the upper pixel. We must increment $y$ (`y++`).
        
    - **If $d \ge 0$:** The midpoint is _above_ (or exactly on) the actual line, meaning the line is closer to the lower pixel. We keep $y$ as it is.
        
    - We always move one step to the right: `x++`.
        
    - Draw the pixel at the new $(x,y)$ coordinates: `SetPixel(hdc, x, y, c)`.
        

- To get rid of the $\frac{1}{2}$ in the equation of $d$ just multiply by two :$$d = (2y + 1 - 2y_1)(x_2 - x_1) - 2(x + 1 - x_1)(y_2 - y_1)$$

---

> [!tip] A Note on Performance
> What makes Bresenham's algorithm famous is that in actual implementation, you don't calculate that complicated formula for $d$ from scratch every time. You use **integer arithmetic** to simply update $d$ based on its previous value — eliminating all multiplication inside the loop.

## Algorithm Code :
Here is the code for the **Midpoint Line Algorithm** (Bresenham's), written in C++ (since your notes use the Win32 `SetPixel` function syntax).

To make this algorithm incredibly fast, computer scientists apply a clever trick: they multiply the decision variable $d$ by 2. This completely eliminates the fractions (like the $\frac{1}{2}$ from the midpoint) and allows the computer to calculate everything using only fast integer math.

Here is how that translates into clean, optimized code for the case where $0 \le \text{slope} \le 1$ and $x_1 < x_2$:

```C++
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    // 1. Calculate the differences
    int dx = x2 - x1;
    int dy = y2 - y1;

    // 2. Initialize starting points
    int x = x1;
    int y = y1;

    // 3. Initialize the integer decision variable 'd'
    // This is mathematically equivalent to d = f(x+1, y+0.5) multiplied by 2 
    int d = dx - (2 * dy); 

    // Draw the very first pixel
    SetPixel(hdc, x, y, c);

    // 4. The Loop
    while (x < x2) {
        // If d < 0, the midpoint is under the line.
        // The line is closer to the upper pixel, so we increment y.
        if (d < 0) {
            y++;
            // Update the decision variable for the next step (x+1, y+1)
            d = d + (2 * dx) - (2 * dy); 
        } 
        // If d >= 0, the midpoint is above or on the line.
        // The line is closer to the lower pixel, so y stays the same.
        else {
            // Update the decision variable for the next step (x+1, y)
            d = d - (2 * dy); 
        }

        // We always move one step to the right
        x++;

        // Draw the pixel at the new calculated position
        SetPixel(hdc, x, y, c);
    }
}
```

### Why this is so elegant:

Notice what happens inside the `while` loop: there is no multiplication, no division, and no floating-point decimals. By pre-calculating the updates (`2 * dx` and `2 * dy`), the computer only has to perform basic addition and subtraction for every pixel it draws. This is exactly why this algorithm became the industry standard in computer graphics!

---

## Bresenham's Line Algorithm

This shows how to mathematically derive the optimized code ([[FACL/ComputerGraphics/Chapters/LEC2/LEC2#Algorithm Code|This code]]) focusing on how to completely eliminate multiplication inside the drawing loop.

### The Power of Incremental Differences
![[Screenshot From 2026-03-15 07-39-29.png]]

this image demonstrates how to use the principles of a Digital Differential Analyzer (DDA) to calculate the decision variable $d$ incrementally. Instead of recalculating the entire formula for every pixel, we just figure out how much $d$ _changes_ from one step ($d_i$) to the next ($d_{i+1}$).

**The Math Breakdown:**

This show the expanded formula for the decision variable at step $i$ ($d_i$) and the next step $i+1$ ($d_{i+1}$).
	
Because we always move exactly one pixel to the right ($x_{i+1} = x_i + 1$), the change in our decision variable, denoted as $\Delta d = d_{i+1} - d_i$, depends entirely on what happens to $y$:
	
- **Case 1: The line is "above" the midpoint ($d_i < 0$)**
    
    We must move $y$ up by 1 ($y_{i+1} = y_i + 1$). If you subtract the $d_i$ formula from the $d_{i+1}$ formula (with both $x$ and $y$ increasing by 1), the massive equation simplifies perfectly to:$$\Delta d = 2\Delta x - 2\Delta y$$
    
- **Case 2: The line is "below" the midpoint ($d_i \ge 0$)**
    
    We keep $y$ exactly the same ($y_{i+1} = y_i$). If you subtract the formulas with only $x$ increasing by 1, it simplifies to:$$\Delta d = -2\Delta y$$
    
**The Starting Point:**
To kick off the loop, we need the very first $d$ value. By plugging the starting coordinates $x_1$ and $y_1$ into the $d$ formula, the math collapses down to a simple starting value:
$$\text{Initial } d = \Delta x - 2\Delta y$$

---

### 2. The Final Optimized Algorithm (Image 2)
![[Screenshot From 2026-03-15 07-40-50-1.png]]

This image takes those mathematical discoveries and writes the final, hyper-optimized pseudocode for the case where the slope is between 0 and 1 ($0 \le \text{slope} \le 1$).

Notice the brilliant optimization here: the values for $\Delta d$ are entirely constant for the whole line because $\Delta x$ and $\Delta y$ never change!

**Code** :
```cpp
void DrawLineBresenhamOptimized(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    // 1. Calculate the total distances
    int dx = x2 - x1;
    int dy = y2 - y1;

    // 2. Pre-calculate the constant updates (The DDA optimization)
    // This eliminates all multiplication inside the while loop
    int delta_d1 = (2 * dx) - (2 * dy);  // The update value if 'y' increments
    int delta_d2 = -(2 * dy);            // The update value if 'y' stays the same

    // 3. Initialize starting coordinates
    int x = x1;
    int y = y1;

    // 4. Calculate the starting decision variable
    int d = dx - (2 * dy);

    // Draw the very first pixel
    SetPixel(hdc, x, y, c);

    // 5. The Fast Loop
    while (x < x2) {
        // If d < 0, the line is above the midpoint
        if (d < 0) {
            y++;              // Move up one pixel
            d += delta_d1;    // Add the pre-calculated constant
        } 
        // If d >= 0, the line is below or on the midpoint
        else {
            // y remains the same
            d += delta_d2;    // Add the pre-calculated constant
        }

        // We always step one pixel to the right
        x++;

        // Draw the pixel at the new (x, y) coordinate
        SetPixel(hdc, x, y, c);
    }
}
```

**The Algorithm Flow:**

1. **Pre-calculate Constants:** Before the loop even begins, calculate the differences and the constant update values to save processing time later:
	    
    - $\Delta d1 = 2\Delta x - 2\Delta y$ (The update if $y$ changes)
        
    - $\Delta d2 = -2\Delta y$ (The update if $y$ stays the same)
    
2. **Initialize:** Set $x$ and $y$ to the start point, draw the first pixel, and set the starting $d = \Delta x - 2\Delta y$.
    
3. **The Fast Loop:**
	    
    - Check if $d < 0$.
        
    - If it is, increment $y$ and simply add the pre-calculated $\Delta d1$ to $d$.
        
    - If it isn't, just add the pre-calculated $\Delta d2$ to $d$.
        
    - Increment $x$ and draw the pixel.
        

By calculating $\Delta d1$ and $\Delta d2$ in advance, the code inside the `while` loop is reduced to nothing but simple integer addition and basic `if` checks. This is what allows older, slower computers to draw lines instantly.

**Time Complexity:** $O(\max(|dx|, |dy|))$ — one iteration per unit of the longer axis.

**Space Complexity:** $O(1)$ — only a few integer variables needed.

---

## Algorithm Comparison Table

| Algorithm | Operations per Pixel | Uses Floating-Point? | Accuracy | Speed | Best For |
|-----------|---------------------|---------------------|----------|-------|----------|
| **Naive (Equation)** | Multiplication + Division + Rounding | Yes | Good | Slow | Teaching concepts |
| **DDA** | Addition + Rounding | Yes | Good | Moderate | Simple implementations |
| **Midpoint/Bresenham** | Integer Addition/Subtraction only | No | Excellent | Fastest | Real-time graphics, games |

> [!note] Why Bresenham is fastest
> - **No multiplication or division** inside the loop
> - **No floating-point arithmetic** — pure integer math
> - **Incremental updates** — only add/subtract constants
> - This makes it the industry standard for line drawing in computer graphics.

---

## Worked Example: Drawing Line (2, 3) to (10, 7)

Let's trace through Bresenham's algorithm step-by-step for the line from $(x_1, y_1) = (2, 3)$ to $(x_2, y_2) = (10, 7)$.

### Parameters
- $dx = 10 - 2 = 8$
- $dy = 7 - 3 = 4$
- Slope $m = \frac{dy}{dx} = \frac{4}{8} = 0.5$
- Since $|m| \le 1$, we step along the x-axis

### Initialization
- Starting point: $(x, y) = (2, 3)$
- Initial decision variable: $d = dx - 2 \cdot dy = 8 - 2 \cdot 4 = 0$
- Pre-calculated updates:
  - $\Delta d_1 = 2dx - 2dy = 16 - 8 = 8$ (when y increments)
  - $\Delta d_2 = -2dy = -8$ (when y stays same)

### Step-by-Step Trace

| Step | x | y | d (before) | Decision | d (after) | Pixel Drawn |
|------|---|---|------------|----------|-----------|-------------|
| Start | 2 | 3 | 0 | — | — | (2, 3) |
| 1 | 3 | 3 | 0 | $d \ge 0$ → keep y | $d = 0 + (-8) = -8$ | (3, 3) |
| 2 | 4 | 4 | -8 | $d < 0$ → y++ | $d = -8 + 8 = 0$ | (4, 4) |
| 3 | 5 | 4 | 0 | $d \ge 0$ → keep y | $d = 0 + (-8) = -8$ | (5, 4) |
| 4 | 6 | 5 | -8 | $d < 0$ → y++ | $d = -8 + 8 = 0$ | (6, 5) |
| 5 | 7 | 5 | 0 | $d \ge 0$ → keep y | $d = 0 + (-8) = -8$ | (7, 5) |
| 6 | 8 | 6 | -8 | $d < 0$ → y++ | $d = -8 + 8 = 0$ | (8, 6) |
| 7 | 9 | 6 | 0 | $d \ge 0$ → keep y | $d = 0 + (-8) = -8$ | (9, 6) |
| 8 | 10 | 7 | -8 | $d < 0$ → y++ | $d = -8 + 8 = 0$ | (10, 7) |

**Final pixels drawn:** (2,3) → (3,3) → (4,4) → (5,4) → (6,5) → (7,5) → (8,6) → (9,6) → (10,7)

This creates a smooth, gap-free line with no floating-point operations!

---

## Edge Cases

### 1. Horizontal Line ($dy = 0$)
When the line is completely horizontal, the slope $m = 0$. The algorithm naturally handles this:
- DDA: $m = 0$, so $y$ never changes
- Bresenham: $d$ condition keeps $y$ constant

```cpp
// Example: Line from (5, 10) to (15, 10)
// Result: All pixels at y = 10, x from 5 to 15
```

### 2. Vertical Line ($dx = 0$)
When the line is completely vertical, the slope is infinite. The algorithm must handle this specially:
- DDA: Takes the `else` branch, increments along y-axis
- Bresenham: Standard implementations handle $|slope| > 1$ cases separately

```cpp
// Example: Line from (10, 5) to (10, 15)
// Result: All pixels at x = 10, y from 5 to 15
```

### 3. Line Drawn Backwards ($x_1 > x_2$)
When the starting point is to the right of the ending point:
- The `swap(x1, y1, x2, y2)` logic ensures we always draw left-to-right
- Both DDA and Bresenham handle this automatically

```cpp
// Example: Line request from (15, 10) to (5, 5)
// After swap: Treated as line from (5, 5) to (15, 10)
```

### 4. Negative Slope
When the line goes downward ($y_2 < y_1$):
- $dy$ becomes negative
- The slope $m$ is negative
- DDA: `y += m` decrements y each step
- Bresenham: Adjusts decision logic to move downward

```cpp
// Example: Line from (2, 10) to (10, 2)
// dy = -8, dx = 8, slope = -1
```

---

## Summary & Key Takeaways

1. **Line rasterization** converts continuous mathematical line equations into discrete pixel coordinates on a raster display.

2. **Naive approach** recalculates the full line equation for each pixel — slow due to multiplication and division at every step.

3. **DDA (Digital Differential Analyzer)** uses **incremental addition** ($\Delta y = m$ for each x-step) instead of full recalculation — faster than naive but still uses floating-point arithmetic.

4. **Midpoint algorithm** introduces a **decision variable** $d$ that tests whether the true line passes above or below the midpoint between two candidate pixels.

5. **Bresenham's optimization** eliminates fractions by multiplying $d$ by 2, enabling pure integer arithmetic — **no multiplication inside the loop**.

6. **Slope determines stepping direction:**
   - $|m| \le 1$: Step along x-axis, calculate y
   - $|m| > 1$: Step along y-axis, calculate x

7. **Why Bresenham is the industry standard:**
   - Only integer addition and subtraction
   - No floating-point operations
   - Pre-calculated constant updates
   - Fastest possible line drawing

8. **All three algorithms guarantee** a continuous, gap-free line with $O(\max(|dx|, |dy|))$ time complexity.

---

> [!tip] Quick Revision Summary
> - **Naive**: Full recalculation — slowest
> - **DDA**: Incremental addition — moderate speed, uses floats
> - **Bresenham**: Integer-only — fastest, industry standard
