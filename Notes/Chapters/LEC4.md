---
Created: " 2026-03-24 12:07"
Modified: 2026-03-24 12:07
tags:
  - FACL
  - ComputerGraphics
---
> [!quote] If someone in your life talked to you the way you talk to yourself, you would have left them long ago.
> — Carla Gordon


# Parametric Curves - Polynomial Curves

## 1. The Big Picture: What is a "Parametric" Curve?

Instead of thinking of a curve as a standard graph where $y$ depends on $x$ (like $y = x^2$), imagine you are driving a car along a winding road.

Your position on the map—your X and Y coordinates—depends on the **time** you've been driving. In math, we call this "time" the **parameter**, represented by $t$. Typically, $t$ goes from $0$ (the start of the curve) to $1$ (the end of the curve).

---

## 2. Polynomial Equations and Curve Degree

### The Math: Polynomial Equations

Because your X, Y, and Z positions are completely independent of each other, they each get their own mathematical equation based on $t$.

In computer graphics, we use polynomials for these equations because computers are very fast at calculating them.

- **X position over time:**
    $$x(t) = \alpha_0 + \alpha_1 t + \alpha_2 t^2 + \dots + \alpha_n t^n$$
    
- **Y position over time:**
    
    $$y(t) = \beta_0 + \beta_1 t + \beta_2 t^2 + \dots + \beta_n t^n$$
    
- **Z position (if in 3D):**
    
    $$z(t) = \gamma_0 + \gamma_1 t + \gamma_2 t^2 + \dots + \gamma_n t^n$$

**Note**: $n$ is the curve degree factor.
	 
> **Why this matters:** This method is incredibly flexible. A standard function like $y = f(x)$ can never loop back on itself or draw a circle (because one $x$ can only have one $y$). Parametric curves can draw loops, spirals, and S-shapes effortlessly.

### Why Cubic Curves ($n=3$) are the Industry Standard

The **degree n** of the polynomial determines the curve's properties:

- **Higher n → More flexibility**: The curve can have more local minimum and maximum points, meaning it can bend and twist more.
- **Trade-off**: The higher the $n$, the more complicated calculations become.

> The degree ($n$) is simply the highest power of $t$ in your equation (for example, if $n=3$, the equation includes $t^3$). The higher the degree, the more "bends" or "wiggles" the curve can have.

| **Curve Degree**       | **Flexibility**                                 | **Drawbacks**                                                      | **Verdict in Graphics** |
| ---------------------- | ----------------------------------------------- | ------------------------------------------------------------------ | ----------------------- |
| **Quadratic ($n=2$)**  | Low. Only bends once (like a U-shape).          | Too rigid. Cannot create S-shapes or complex curves.               | Too simple.             |
| **Cubic ($n=3$)**      | Perfect. Can bend twice, allowing for S-shapes. | Slightly more math than quadratic, but computers handle it easily. | *The Industry Standard* |
| **Higher ($n \ge 4$)** | Extremely high. Can wiggle many times.          | Highly unstable. Prone to wild oscillations and heavy computation. | Too complex.            |

> [!tip] Why Cubic Curves?
> - Quadratic curves (n=2) are too rigid and cannot have inflection points
> - Cubic curves can represent S-shaped curves with inflection points
> - Higher degree curves can lead to oscillations and numerical instability

---

## 3. Segmentation: Building Complex Shapes

If you want to draw a very complicated shape (like the outline of a car), you **do not** use one massive equation with a degree of $n=50$. It would be computationally expensive and wildly unstable.

Instead, we use **segmentation**. We take several small, predictable Cubic Curves and glue them together end-to-end to build the larger shape (these are often called [[Spline|Splines]]).

**The Smoothness Rule (C¹ Continuity):**

If you just glue two curves together randomly, you'll get a sharp, ugly corner where they meet. To make the transition invisible and smooth, we apply a mathematical rule:

> [!important] C¹ Continuity Condition
> The exact slope at the **end** of Segment A must perfectly match the exact slope at the **beginning** of Segment B.

This is known as **C¹ continuity** (first derivative continuity), which ensures that the curve appears smooth without sharp corners at segment boundaries.

---

## 4. Three Interpolation Methods (Overview)

> [!note] This section provides a quick comparison. Detailed derivations for Hermite and Bezier follow in sections 5 and 6.

| Curve Type | Input | Characteristics | Usage in Graphics |
|------------|-------|------------------|-------------------|
| **Lagrange** | Control points only | Passes through all points; can oscillate; global control | Rarely used |
| **Hermite** | 2 endpoints + 2 tangent vectors | Direct control over position and direction; good for interpolation | Used when tangent control is needed |
| **Bezier** | 4 control points | Curve passes through first and last points; intuitive control handles | **Most commonly used** |

### Lagrange Interpolation (Brief)

> [!warning] Not Used in Practice
> Lagrange interpolation is **NOT commonly used in Computer Graphics** because:
> - Produces curves that pass exactly through all control points
> - Can cause unwanted oscillations ([[Runge's phenomenon]])
> - Does not provide intuitive control over curve shape
> - Global control: moving one point affects the entire curve

---

## 5. Hermite Cubic Curve (Detailed)

### 5.1 Setup and Parameters

![[FACL/ComputerGraphics/Chapters/LEC4/Attachments/image.png]]

**Control Points:**
- $P_1(x_1, y_1)$ at $t = 0$
- $P_2(x_2, y_2)$ at $t = 1$

**Tangent Vectors:**
- $T_1(u_1, v_1)$ at $t = 0$ (tangent at starting point)
- $T_2(u_2, v_2)$ at $t = 1$ (tangent at ending point)

**Key Feature:** Hermite curves are defined by two endpoints and their tangent vectors, giving direct control over both position and direction at endpoints.

### 5.2 Mathematical Derivation

The Hermite curve is defined by cubic polynomials:

$$x(t) = \alpha_0 + \alpha_1 t + \alpha_2 t^2 + \alpha_3 t^3$$
$$y(t) = \beta_0 + \beta_1 t + \beta_2 t^2 + \beta_3 t^3$$

**Derivatives:**
$$x'(t) = \alpha_1 + 2\alpha_2 t + 3\alpha_3 t^2$$
$$y'(t) = \beta_1 + 2\beta_2 t + 3\beta_3 t^2$$

### 5.3 Four Constraints (Boundary Conditions)

| Condition     | Equation                                          | Result                                                                 |
| ------------- | ------------------------------------------------- | ---------------------------------------------------------------------- |
| $x(0) = x_1$  | $\alpha_0 = x_1$                                  | ✓ $\alpha_0 = x_1$                                                     |
| $x'(0) = u_1$ | $\alpha_1 = u_1$                                  | ✓ $\alpha_1 = u_1$                                                     |
| $x(1) = x_2$  | $\alpha_0 + \alpha_1 + \alpha_2 + \alpha_3 = x_2$ | $\alpha_2+ \alpha_3 = x_2-x_1-u_1$ |
| $x'(1) = u_2$ | $\alpha_1 + 2\alpha_2 + 3\alpha_3 = u_2$          | $2\alpha_2+ 3\alpha_3 = u_2-u_1$   |

To get $\alpha_2 , \alpha_3$ we need to solve two equations in two variables.

### 5.4 Solving for Coefficients

**From Constraints 3 and 4:**
$$\alpha_2 + \alpha_3 = x_2 - x_1 - u_1$$
$$2\alpha_2 + 3\alpha_3 = u_2 - u_1$$

**Solution:**
$$\alpha_2 = 3x_2 - 3x_1 - 2u_1 - u_2$$
$$\alpha_3 = -2x_2 + 2x_1 + u_1 + u_2$$

### 5.5 Hermite Matrix ($H$)

The relationship between geometric constraints and polynomial coefficients is:

$$\begin{bmatrix} \alpha_0 \\ \alpha_1 \\ \alpha_2 \\ \alpha_3 \end{bmatrix} = \underbrace{\begin{bmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ -3 & -2 & 3 & -1 \\ 2 & 1 & -2 & 1 \end{bmatrix}}_{H} \begin{bmatrix} x_1 \\ u_1 \\ x_2 \\ u_2 \end{bmatrix}$$

From:
$$\alpha_0 = x_1$$
$$\alpha_1 = u_1$$
$$\alpha_2 = 3x_2 - 3x_1 - 2u_1 - u_2$$
$$\alpha_3 = -2x_2 + 2x_1 + u_1 + u_2$$

This is the **Hermite Basis Matrix** ($M_H$), which transforms geometric information (points and tangents) into polynomial coefficients.

### 5.6 Efficient Computation (Matrix Form)

$$\begin{bmatrix} \alpha_0 & \beta_0 \\ \alpha_1 & \beta_1 \\ \alpha_2 & \beta_2 \\ \alpha_3 & \beta_3 \end{bmatrix} = H \begin{bmatrix} x_1 & y_1 \\ u_1 & v_1 \\ x_2 & y_2 \\ u_2 & v_2 \end{bmatrix}$$

$$x(t) = \alpha_0 + \alpha_1 t + \alpha_2 t^2 + \alpha_3 t^3 = \begin{bmatrix} 1 & t & t^2 & t^3 \end{bmatrix} \begin{bmatrix} \alpha_0 \\ \alpha_1 \\ \alpha_2 \\ \alpha_3 \end{bmatrix}$$

$$y(t) = \begin{bmatrix} 1 & t & t^2 & t^3 \end{bmatrix} \begin{bmatrix} \beta_0 \\ \beta_1 \\ \beta_2 \\ \beta_3 \end{bmatrix}$$

**Combined Form:**
$$[x(t) \quad y(t)] = \begin{bmatrix} 1 & t & t^2 & t^3 \end{bmatrix} \begin{bmatrix} \alpha_0 & \beta_0 \\ \alpha_1 & \beta_1 \\ \alpha_2 & \beta_2 \\ \alpha_3 & \beta_3 \end{bmatrix}  = \begin{bmatrix} 1 & t & t^2 & t^3 \end{bmatrix} \cdot H \underbrace{\begin{bmatrix} x_1 & y_1 \\ u_1 & v_1 \\ x_2 & y_2 \\ u_2 & v_2 \end{bmatrix}}_{G}$$

Where $G$ stands for **Geometric Matrix**.

### 5.7 Hermite Algorithm

![[FACL/ComputerGraphics/Chapters/LEC4/Attachments/image-2.png]]

**Given:** $(x_1, y_1)$, $(u_1, v_1)$, $(x_2, y_2)$, $(u_2, v_2)$, color

**Steps:**

**① Construct H:**
$$H = \begin{bmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ -3 & -2 & 3 & -1 \\ 2 & 1 & -2 & 1 \end{bmatrix}$$

**② Construct G:**
$$G = \begin{bmatrix} x_1 & y_1 \\ u_1 & v_1 \\ x_2 & y_2 \\ u_2 & v_2 \end{bmatrix}$$

**③ Compute C:**
$$C = H \times G$$

**④ For $t = 0$ to $1$, step $dt$:**
- Construct: $$V = \begin{bmatrix} 1 & t & t^2 & t^3 \end{bmatrix}^T$$
- Compute: $$x = V^T \times C$$
- Where $C$ stands for Coefficient Matrix.

> [!tip] This is a Cubic Hermite Spline algorithm — used to draw smooth curves between two points given their positions and tangent vectors.

### 5.8 Worked Example

**Problem:** Generate a Hermite curve with the following parameters:
- $P_1 = (1, 1)$, $P_2 = (4, 3)$
- $T_1 = (1, 2)$ (tangent at start)
- $T_2 = (-1, -1)$ (tangent at end)

**Step 1: Construct G (Geometric Matrix)**
$$G = \begin{bmatrix} 1 & 1 \\ 1 & 2 \\ 4 & 3 \\ -1 & -1 \end{bmatrix}$$

**Step 2: Compute C (Coefficient Matrix)**
$$C = H \times G = \begin{bmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ -3 & -2 & 3 & -1 \\ 2 & 1 & -2 & 1 \end{bmatrix} \begin{bmatrix} 1 & 1 \\ 1 & 2 \\ 4 & 3 \\ -1 & -1 \end{bmatrix}$$

For x-coefficients:
$$\begin{bmatrix} \alpha_0 \\ \alpha_1 \\ \alpha_2 \\ \alpha_3 \end{bmatrix} = \begin{bmatrix} 1 \\ 1 \\ 3(4) - 3(1) - 2(1) - (-1) = 10 \\ -2(4) + 2(1) + 1 + (-1) = -6 \end{bmatrix} = \begin{bmatrix} 1 \\ 1 \\ 10 \\ -6 \end{bmatrix}$$

**Step 3: Evaluate at sample points**

At $t = 0$: $x(0) = 1$, $y(0) = 1$ ✓ (passes through $P_1$)

At $t = 0.5$: 
$$x(0.5) = 1 + 1(0.5) + 10(0.25) - 6(0.125) = 1 + 0.5 + 2.5 - 0.75 = 3.25$$

At $t = 1$: $x(1) = 1 + 1 + 10 - 6 = 4$ ✓ (passes through $P_2$)

---

## 6. Bezier Curve (Cubic) - Detailed Derivation

### 6.1 Setup

![[FACL/ComputerGraphics/Chapters/LEC4/Attachments/image-1.png]]

**Given:** Four control points $(x_1, y_1), (x_2, y_2), (x_3, y_3), (x_4, y_4)$

**Control Points:**
- **$P_1$ and $P_4$**: First and last points lie ON the curve → *End Points*
- **$P_2$ and $P_3$**: Guidance points (control handles) that influence the curve shape → *Control Points*

**Parameter Values:**
$$t = 0, \quad t = \frac{1}{3}, \quad t = \frac{2}{3}, \quad t = 1$$

### 6.2 Bezier Curve Properties

- The curve always passes through the first and last control points
- The curve is pulled toward (but doesn't pass through) intermediate control points
- The tangent at the first point points toward $P_2$
- The tangent at the last point points toward $P_3$
- The curve is completely contained within the [[Convex Hull|convex hull]] of the control points

**Derivative Relationships:**
- At first point: $\text{speed} = \frac{P_2 - P_1}{1/3} = 3(P_2 - P_1)$
- At end point: $\text{speed} = \frac{P_4 - P_3}{1/3} = 3(P_4 - P_3)$

### 6.3 Mathematical Derivation

The cubic Bezier curve is defined by:
$$x(t) = \alpha_0 + \alpha_1 t + \alpha_2 t^2 + \alpha_3 t^3$$
$$y(t) = \beta_0 + \beta_1 t + \beta_2 t^2 + \beta_3 t^3$$

**Constraints (Boundary Conditions):**

| Constraint                          | Mathematical Expression | Result                                    |
| ----------------------------------- | ----------------------- | ----------------------------------------- |
| Curve passes through $P_1$ at $t=0$ | $x(0) = x_1$            | $\alpha_0 = x_1$                          |
| Derivative at $t=0$                 | $x'(0) = 3(x_2 - x_1)$  | $\alpha_1 = 3(x_2 - x_1)$                 |
| Curve passes through $P_4$ at $t=1$ | $x(1) = x_4$            | $\alpha_0 + \alpha_1 + \alpha_2 + \alpha_3 = x_4$ |
| Derivative at $t=1$                 | $x'(1) = 3(x_4 - x_3)$  | $\alpha_1 + 2\alpha_2 + 3\alpha_3 = 3(x_4 - x_3)$ |

**Understanding the Derivatives:**
- At $t=0$: The tangent points from $P_1$ toward $P_2$
- At $t=1$: The tangent points from $P_3$ toward $P_4$

### 6.4 Solving for Coefficients

From constraints 3 and 4:
$$\alpha_2 + \alpha_3 = x_4 - x_1 - 3(x_2 - x_1) = x_4 - 3x_2 + 2x_1$$
$$2\alpha_2 + 3\alpha_3 = 3x_4 - 3x_3 - 3x_2 + 3x_1$$

**Solution:**
$$\alpha_0 = x_1$$
$$\alpha_1 = 3(x_2 - x_1)$$
$$\alpha_2 = 3x_1 - 6x_2 + 3x_3$$
$$\alpha_3 = -x_1 + 3x_2 - 3x_3 + x_4$$

### 6.5 Bezier Matrix

The coefficients can be expressed compactly using matrix multiplication:

$$\begin{bmatrix} \alpha_0 \\ \alpha_1 \\ \alpha_2 \\ \alpha_3 \end{bmatrix} = \underbrace{\begin{bmatrix} 1 & 0 & 0 & 0 \\ -3 & 3 & 0 & 0 \\ 3 & -6 & 3 & 0 \\ -1 & 3 & -3 & 1 \end{bmatrix}}_{M_B} \begin{bmatrix} x_1 \\ x_2 \\ x_3 \\ x_4 \end{bmatrix}$$

This 4×4 matrix is the **Bezier Basis Matrix** ($M_B$). The same matrix applies to compute the $\beta$ coefficients for the y-component.

### 6.6 Bernstein Basis Functions

The Bezier curve basis functions are derived from the **binomial expansion:**
$$(a + b)^3 = \binom{3}{0} a^0 b^3 + \binom{3}{1} a^1 b^2 + \binom{3}{2} a^2 b^1 + \binom{3}{3} a^3 b^0$$
$$(a + b)^3 = b^3 + 3ab^2 + 3a^2b + a^3$$

**Substitution for Bezier Curves:**
Let $a = t$ and $b = (1-t)$:
$$(t + (1-t))^3 = (1-t)^3 + 3t(1-t)^2 + 3t^2(1-t) + t^3 = 1$$

> [!important] Key Property
> The Bernstein basis functions sum to 1, ensuring the curve has the convex hull property.

**Bernstein Polynomials (Cubic):**
$$B_0^3(t) = (1-t)^3$$
$$B_1^3(t) = 3t(1-t)^2$$
$$B_2^3(t) = 3t^2(1-t)$$
$$B_3^3(t) = t^3$$

### 6.7 Bezier Curve Forms

**Matrix Form:**
$$x(t) = \begin{bmatrix} 1 & t & t^2 & t^3 \end{bmatrix} \begin{bmatrix} 1 & 0 & 0 & 0 \\ -3 & 3 & 0 & 0 \\ 3 & -6 & 3 & 0 \\ -1 & 3 & -3 & 1 \end{bmatrix} \begin{bmatrix} x_1 \\ x_2 \\ x_3 \\ x_4 \end{bmatrix}$$

**Bernstein Form:**
$$x(t) = \begin{bmatrix} (1-t)^3 & 3t(1-t)^2 & 3t^2(1-t) & t^3 \end{bmatrix} \begin{bmatrix} x_1 \\ x_2 \\ x_3 \\ x_4 \end{bmatrix}$$

Both forms are mathematically equivalent.

### 6.8 General Bezier Formula

The general form of an $n^{th}$-degree Bezier curve:
$$P(t) = \sum_{i=0}^{n} \binom{n}{i} t^i (1-t)^{n-i} P_i$$

Where $\binom{n}{i}$ is the binomial coefficient (n choose i).

### 6.9 Quadratic Bezier (Reference)

For a 2nd-degree (quadratic) Bezier curve with 3 control points:

$$P(t) = (1-t)^2 P_0 + 2t(1-t) P_1 + t^2 P_2$$

**Quadratic Bezier Matrix:**
$$M_{B,quad} = \begin{bmatrix} 1 & -2 & 1 \\ -2 & 2 & 0 \\ 1 & 0 & 0 \end{bmatrix}$$

### 6.10 Bezier Algorithm

**Given:** Four control points $P_1, P_2, P_3, P_4$ and color

**Steps:**

**① Construct Bezier Matrix $M_B$:**
$$M_B = \begin{bmatrix} 1 & 0 & 0 & 0 \\ -3 & 3 & 0 & 0 \\ 3 & -6 & 3 & 0 \\ -1 & 3 & -3 & 1 \end{bmatrix}$$

**② Construct Control Points Matrix:**
$$P = \begin{bmatrix} x_1 & y_1 \\ x_2 & y_2 \\ x_3 & y_3 \\ x_4 & y_4 \end{bmatrix}$$

**③ Compute Coefficients:**
$$C = M_B \times P$$

**④ For $t = 0$ to $1$, step $dt$:**
- Construct: $$V = \begin{bmatrix} 1 & t & t^2 & t^3 \end{bmatrix}^T$$
- Compute: $$[x(t) \quad y(t)] = V^T \times C$$
- Round coordinates and set pixel color

### 6.11 Worked Example

**Problem:** Generate a Bezier curve with control points:
- $P_1 = (0, 0)$, $P_2 = (1, 2)$, $P_3 = (3, 2)$, $P_4 = (4, 0)$

**Step 1: Construct P (Control Points Matrix)**
$$P = \begin{bmatrix} 0 & 0 \\ 1 & 2 \\ 3 & 2 \\ 4 & 0 \end{bmatrix}$$

**Step 2: Compute C (using matrix form)**
Using the Bernstein form for direct evaluation:
$$x(t) = (1-t)^3 \cdot 0 + 3t(1-t)^2 \cdot 1 + 3t^2(1-t) \cdot 3 + t^3 \cdot 4$$
$$y(t) = (1-t)^3 \cdot 0 + 3t(1-t)^2 \cdot 2 + 3t^2(1-t) \cdot 2 + t^3 \cdot 0$$

**Step 3: Evaluate at sample points**

At $t = 0$: 
- $x(0) = 0$
- $y(0) = 0$
- ✓ Point: $(0, 0)$ — passes through $P_1$

At $t = 0.5$:
- $x(0.5) = 0 + 3(0.5)(0.25)(1) + 3(0.25)(0.5)(3) + (0.125)(4) = 0.375 + 1.125 + 0.5 = 2$
- $y(0.5) = 0 + 3(0.5)(0.25)(2) + 3(0.25)(0.5)(2) + 0 = 0.75 + 0.75 = 1.5$
- Point: $(2, 1.5)$

At $t = 1$:
- $x(1) = 4$
- $y(1) = 0$
- ✓ Point: $(4, 0)$ — passes through $P_4$

---

## 7. Summary and Key Concepts

### The Three Main Curve Types

| Curve Type | Input | Characteristics | When to Use |
|------------|-------|-----------------|-------------|
| **Lagrange** | Control points only | Passes through all points; can oscillate; global control | Rarely used in graphics |
| **Hermite** | 2 endpoints + 2 tangent vectors | Direct control over position and direction; good for interpolation | Animation paths, camera motion |
| **Bezier** | 4 control points | Curve passes through first and last points; intuitive control handles | **Most common**: fonts, vector graphics, UI design |

### Why Bezier Curves Dominate in Computer Graphics

1. **Intuitive Control:** Moving a control point has predictable effects on the curve
2. **Convex Hull Property:** The curve is always contained within the convex hull of control points
3. **Affine Invariance:** Transforming control points transforms the curve
4. **Variation Diminishing:** The curve doesn't oscillate more than its control polygon
5. **Endpoint Interpolation:** The curve always passes through the first and last control points

### Quick Reference: Key Formulas and Matrices

> [!summary] Hermite Curve
> **Matrix:** $M_H = \begin{bmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ -3 & -2 & 3 & -1 \\ 2 & 1 & -2 & 1 \end{bmatrix}$
> 
> **Input:** 2 endpoints $(P_1, P_2)$ + 2 tangent vectors $(T_1, T_2)$
> 
> **Equation:** $P(t) = [1, t, t^2, t^3] \cdot M_H \cdot G$

> [!summary] Bezier Curve (Cubic)
> **Matrix:** $M_B = \begin{bmatrix} 1 & 0 & 0 & 0 \\ -3 & 3 & 0 & 0 \\ 3 & -6 & 3 & 0 \\ -1 & 3 & -3 & 1 \end{bmatrix}$
> 
> **Bernstein Basis:** $B_0(t) = (1-t)^3$, $B_1(t) = 3t(1-t)^2$, $B_2(t) = 3t^2(1-t)$, $B_3(t) = t^3$
> 
> **Input:** 4 control points $(P_1, P_2, P_3, P_4)$
> 
> **Equation:** $P(t) = \sum_{i=0}^{3} B_i(t) \cdot P_i$

> [!tip] Computational Note
> For efficient rendering, use **[[DDA Algorithm]]** (Digital Differential Analyzer) which uses incremental calculations to reduce multiplication operations when plotting pixels.

### Practical Applications

These concepts are fundamental to:
- **Font Design:** TrueType and PostScript fonts use Bezier curves
- **Vector Graphics:** SVG paths, Adobe Illustrator curves
- **3D Modeling:** Surface modeling, animation paths
- **Animation:** Motion paths and interpolation
- **CAD/CAM:** Industrial design and manufacturing

---