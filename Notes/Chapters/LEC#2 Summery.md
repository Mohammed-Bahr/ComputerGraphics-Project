---
Created: " 2026-04-07 20:18"
tags:
  - FACL
  - ComputerGraphics
Source:
---
> [!quote] No one has a finer command of language than the person who keeps his mouth shut.
> — Sam Rayburn

**"اللهم إني أسألك فهم النبيين، وحفظ المرسلين، والملائكة المقربين، اللهم اجعل ألسنتنا عامرة بذكرك، وقلوبنا بخشيتك، إنك على كل شيء قدير"**

# 1. Basic Line Rasterization (Direct Evaluation)
**Concept:** Drawing lines on a pixel grid by calculating points based on the line equation.

### The Equation
$$ \frac{Y - Y_1}{X - X_1} = \frac{Y_2 - Y_1}{X_2 - X_1} $$

### Algorithm Logic
*   **Case 1: Gentle Slope ($|m| \le 1$)**
    *   Iterate along $X$ (Step $X$ by 1).
    *   Calculate $Y$ at each step.
    *   *Code Logic:*
```cpp
for (x = x1; x <= x2; x++) {
	y = y1 + (x - x1) * (y2 - y1) / (x2 - x1);
	SetPixel(x, Round(y), Color);
}
```
*   **Case 2: Steep Slope ($|m| > 1$)**
    *   Iterate along $Y$ (Step $Y$ by 1).
    *   Calculate $X$ at each step.
    *   *Code Logic:*
```cpp
for (y = y1; y <= y2; y++) {
	x = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
	SetPixel(Round(x), y, Color);
}
```

**Drawback:** Requires floating-point multiplication and rounding inside the loop, which is computationally expensive.

---

# 2. Digital Differential Analyzer (DDA)
**Concept:** An incremental algorithm that finds the next point by adding a small value to the previous point ($Y_{i+1} = Y_i + \Delta Y$).
**Key Optimization:** Replaces multiplication with addition.

### Mathematical Basis
*   Line Equation: $Y = mX + c$
*   Derivation: $Y_{i+1} - Y_i = m(X_{i+1} - X_i) \implies \Delta Y = m \Delta X$

### DDA Algorithm (C++)

```cpp
void LineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Case 1: Gentle Slope (|m| <= 1)
    if (abs(dy) <= abs(dx)) {
        double m = (double)dy / dx;
        if (x1 > x2) swap(x1, y1, x2, y2); // Ensure Left-to-Right
        
        SetPixel(hdc, x1, y1, c);
        double y = y1;
        
        for (int x = x1 + 1; x <= x2; x++) {
            y += m; // Incremental addition
            SetPixel(hdc, x, Round(y), c);
        }
    } 
    // Case 2: Steep Slope (|m| > 1)
    else {
        double mi = (double)dx / dy; // Inverse slope
        if (y1 > y2) swap(x1, y1, x2, y2); // Ensure Bottom-to-Top
        
        SetPixel(hdc, x1, y1, c);
        double x = x1;
        
        for (int y = y1 + 1; y <= y2; y++) {
            x += mi; // Incremental addition
            SetPixel(hdc, Round(x), y, c);
        }
    }
}
```

**Why use Swap?**
The logic assumes the independent variable ($x$ or $y$) always increases. Swapping coordinates simplifies the loop condition to a simple `while (x < x2)`.

---

# 3. Midpoint Line Algorithm (Bresenham’s)
**Concept:** Uses integer arithmetic and a decision parameter to choose pixels without floating-point operations.
**Key Idea:** Test the **midpoint** between two candidate pixels to see if it lies above or below the true line.

### The Discriminant Function
Used to determine the position of a point relative to the line:
$$ f(x,y) = (y - y_1)(x_2 - x_1) - (x - x_1)(y_2 - y_1) $$

*   $f(x,y) = 0$: On the line
*   $f(x,y) > 0$: Above the line
*   $f(x,y) < 0$: Below the line

### Algorithm Logic (for $0 \le \text{slope} \le 1$)
1.  Increment $x$ by 1 every step.
2.  Choose between pixel $(x+1, y)$ (East) or $(x+1, y+1)$ (North-East).
3.  **Test Point:** $M = (x+1, y+0.5)$.
4.  **Decision Parameter ($d$):**
    *   If $d < 0$ (Midpoint is **below** line): The line is closer to the **top** pixel ($y+1$).
        *   Action: $y \gets y + 1$.
    *   If $d \ge 0$ (Midpoint is **above** line): The line is closer to the **bottom** pixel ($y$).
        *   Action: Keep $y$.

### Optimization: Integer Math
To eliminate the $0.5$ fraction in the midpoint, we multiply the entire decision parameter equation by 2.
*   **Initial $d$:** $d_{start} = \Delta x - 2\Delta y$
*   **Increment East ($y$ unchanged):** $d \gets d - 2\Delta y$
*   **Increment NE ($y$ increased):** $d \gets d + 2\Delta x - 2\Delta y$

### Midpoint Code (C++)

```cpp
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int x = x1;
    int y = y1;

    // Initial decision parameter (multiplied by 2 for integer math)
    int d = dx - (2 * dy); 

    SetPixel(hdc, x, y, c);

    while (x < x2) {
        if (d < 0) {
            // Midpoint is below line -> Choose Upper Pixel
            y++;
            d = d + (2 * dx) - (2 * dy); // Increment NE
        } else {
            // Midpoint is above line -> Choose Lower Pixel
            d = d - (2 * dy);            // Increment E
        }
        x++;
        SetPixel(hdc, x, y, c);
    }
}
```

---

# 4. Fully Optimized Bresenham (Incremental Differences)
**Concept:** Further optimizes the loop by pre-calculating the **change** in the decision parameter ($\Delta d$) so no multiplication happens inside the loop.

### Pre-calculated Constants
Since $\Delta x$ and $\Delta y$ are constant for the line, the change in $d$ is also constant:
*   `const1 = 2 * dx - 2 * dy` (Change when $y$ increments)
*   `const2 = -2 * dy`       (Change when $y$ stays same)

### Final Optimized Code (C++)

```cpp
void DrawLineBresenhamOptimized(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Pre-calculate constants to remove math from the loop
    int d_incr_y = (2 * dx) - (2 * dy); // Delta d if y increases
    int d_keep_y  = -(2 * dy);          // Delta d if y stays

    int x = x1;
    int y = y1;
    int d = dx - (2 * dy); // Initial d

    SetPixel(hdc, x, y, c);

    while (x < x2) {
        if (d < 0) {
            y++;
            d += d_incr_y;
        } else {
            d += d_keep_y;
        }
        x++;
        SetPixel(hdc, x, y, c);
    }
}
```

### Summary of Progression
1.  **Direct Evaluation:** Floating point multiplication inside loop. (Slowest)
2.  **DDA:** Floating point addition inside loop. (Faster)
3.  **Midpoint:** Integer multiplication inside loop. (Good)
4.  **Optimized Bresenham:** Integer addition only inside loop. (Fastest)