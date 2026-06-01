# Displays & Rasterization

## 1. Graphics Display Systems

There are two primary technologies for displaying computer graphics:
![raster vs vector graphics, AI generated](https://encrypted-tbn1.gstatic.com/licensed-image?q=tbn:ANd9GcR585mk6l8LMlAgO0nfsJiPYbMaYEylEnlku27MHTE_9U-LNSK1svHuaBsa33irsxVh4OFgQdf1_1GAYs1Rqy299aQxE7efXUD-GLpDi8I6ArLZrNU)
### 1.1 Raster Scan Display
Modern standard (monitors, phones, TVs). The screen is treated as a **grid of pixels** (picture elements).

**Key Components:**
- **Frame Buffer (Display Buffer):** Dedicated memory storing a 2D array where each cell represents one pixel
- **Pixel Data:** Each pixel is defined by **color channels**:
  - **RGB:** Red, Green, Blue values (0-255) mixed to create colors
  - **RGBA:** RGB + **Alpha** channel (transparency: 0 = invisible, 1/255 = solid)

**Data Flow:**
$$CPU \rightarrow \text{Display Buffer} \rightarrow \text{Graphics Controller} \rightarrow \text{Graphics Display}$$

**Performance Note:** Higher resolution means more pixels to process. More frames per second (FPS) requires faster computation; if the CPU/GPU cannot keep up, the display becomes choppy.

---

### 1.2 Random Vector (Calligraphic) Display
Older technology (oscilloscopes, early arcade games like *Asteroids*). Uses an electron beam like a pen drawing continuous lines.

**Characteristics:**
- **High Quality:** No pixel grid means no jagged edges (aliasing)
- **Variable Speed:** Drawing speed depends on complexity; solid fills are slow and difficult
- **Primitives:** Uses vector commands:
  - `MoveTo(x₁, y₁)` — Move pen to start position (no drawing)
  - `LineTo(x₂, y₂)` — Draw line to new position
  - `SetColor(color)` — Define line color

**Comparison:** Raster offers static speed regardless of content; Vector offers superior line quality but struggles with complex filled scenes.

---

## 2. Rasterization: Drawing Lines on a Grid

Since raster displays use discrete pixels, drawing a geometric line requires **rasterization** — determining which pixels best approximate the mathematical line.

### 2.1 The Line Equation
Given two points $(x_1, y_1)$ and $(x_2, y_2)$, the slope-intercept relationship is:

$$\frac{y - y_1}{x - x_1} = \frac{y_2 - y_1}{x_2 - x_1}$$

The slope $m = \frac{y_2 - y_1}{x_2 - x_1}$ determines which algorithm to use.

### 2.2 Algorithm Selection Based on Slope

To avoid gaps in the line, choose the iteration axis based on slope steepness:

**Method A: Iterate over X** (Gentle Slopes)
- **Condition:** $|slope| \leq 1$ (line is more horizontal than vertical)
- **Logic:** Step through each $x$ from $x_1$ to $x_2$, calculate corresponding $y$

```pseudocode
for x = x₁ to x₂ step 1
    y = y₁ + (x - x₁)(y₂ - y₁) / (x₂ - x₁)
    SetPixel(x, Round(y), Color)
end
```

**Method B: Iterate over Y** (Steep Slopes)
- **Condition:** $|slope| > 1$ (line is more vertical than horizontal)
- **Logic:** Step through each $y$ from $y_1$ to $y_2$, calculate corresponding $x$
- **Assumption:** $y_1 < y_2$ (swap points if necessary)

```pseudocode
for y = y₁ to y₂ step 1
    x = x₁ + (y - y₁)(x₂ - x₁) / (y₂ - y₁)
    SetPixel(Round(x), y, Color)
end
```

**Why this matters:** 
- If slope > 1 and we iterate over X, y jumps by multiple pixels per step, creating gaps
- If slope ≤ 1 and we iterate over Y, we waste computation on multiple pixels per column
- **Rounding:** Since calculated coordinates are decimal values (e.g., 4.7), `Round()` selects the nearest integer pixel coordinate

---

## Summary Table

| Feature | Raster Scan | Vector Display |
|---------|-------------|----------------|
| **Basic Unit** | Pixels (grid) | Lines (continuous) |
| **Memory** | Frame buffer (2D array) | Display list (commands) |
| **Line Quality** | Jagged (aliased) | Perfectly smooth |
| **Speed** | Constant (scanning) | Variable (depends on complexity) |
| **Best For** | Filled areas, photos, general use | Wireframes, precise technical drawings |

**Line Drawing Rule of Thumb:** 
- $|m| \leq 1$ → Loop through X, calculate Y
- $|m| > 1$ → Loop through Y, calculate X

---