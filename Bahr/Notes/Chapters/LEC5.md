---
Created: " 2026-04-12 17:21"
tags:
  - FACL
  - ComputerGraphics
  - Bezier
  - Splines
  - FloodFill
  - ComputerGraphics/FillingAlgorithms
Source:
---
> [!quote] Limitations live only in our minds. But if we use our imaginations, our possibilities become limitless.
> — Jamie Paolinetti

**"اللهم إني أسألك فهم النبيين، وحفظ المرسلين، والملائكة المقربين، اللهم اجعل ألسنتنا عامرة بذكرك، وقلوبنا بخشيتك، إنك على كل شيء قدير"**


This lecture bridges the gap between **mathematical curve representation** and **practical rendering algorithms**:

> Representing smooth curves using control points — from the recursive elegance of Bezier curves through to the continuity guarantees of Cardinal and B-splines.

---

## Bezier Curves: Curve Generation

**The Core Idea**: Instead of defining curves through explicit equations, we use **control points** and **blending functions** to "pull" the curve into shape.
$$X(t) = \sum_{r=0}^{n} C_r^n \cdot t^r \cdot (1-t)^{n-r} \cdot X_{r+1}$$
$$Y(t) = \sum_{r=0}^{n} C_r^n \cdot t^r \cdot (1-t)^{n-r} \cdot Y_{r+1}$$
#### Mathematical Definition
A Bezier curve of degree $n$ with control points $P_1, P_2, ..., P_{n+1}$:

$$P(t) = \sum_{r=0}^{n} C_r^n \cdot t^r (1-t)^{n-r} P_{r+1} \quad \text{where } P \text{ is a vector.}$$

Where:
- $t \in [0, 1]$ (parameter along the curve)
- $C_r^n = \binom{n}{r}$ (binomial coefficient from Pascal's Triangle)
- $P_{r+1}$ are the control points (vectors containing x and y)

> [!tip] **Bernstein Polynomials**
> The functions $B_r^n(t) = C_r^n \cdot t^r \cdot (1-t)^{n-r}$ are called **Bernstein polynomials**. They form a valid basis for blending because:
> - **Partition of unity**: $\sum_{r=0}^{n} B_r^n(t) = 1$ for all $t$
> - **Non-negative**: $B_r^n(t) \geq 0$ for $t \in [0,1]$
> - **Symmetry**: $B_r^n(t) = B_{n-r}^n(1-t)$
>
> This means each control point gets a "weight" between 0 and 1, and all weights sum to 1 — perfect for interpolation.

> [!note] **Why $t \in [0,1]$?**
> The parameter $t$ lives in the normalized interval $[0,1]$ because:
> - $t = 0$ maps to the **start** of the curve ($P_1$)
> - $t = 1$ maps to the **end** of the curve ($P_{n+1}$)
> - All intermediate values interpolate smoothly between these endpoints
> - This normalization ensures Bernstein polynomials remain **convex** (weights sum to 1, all non-negative)

#### Recursive Formulation 
The key insight: A Bezier curve of degree $n$ can be defined recursively using two curves of degree $n-1$:
$$C_r^n = C_{r-1}^{n-1} + C_r^{n-1}$$

#### Derivation using the recursive formula

Using the substitution $r' = r - 1$ (i.e. $r = r' + 1$), the curve is split into two sums:
$$P(t) = \sum_{r=0}^{n} C_{r-1}^{n-1} . t^r (1-t)^{n-r} P_{r+1} + \sum_{r=0}^{n} C_r^{n-1} . t^r (1-t)^{n-r} P_{r+1}$$

After re-indexing with $r' = r - 1$, this becomes:
$$= t \sum_{r'=0}^{n-1} C_{r'}^{n-1} . t^{r'} (1-t)^{n-1-r'} P_{r'+2} + (1-t)\sum_{r=0}^{n-1} C_r^{n-1} . t^r (1-t)^{n-1-r} P_{r+1}$$

##### What this means ?

The key idea is that a degree-$n$ Bézier curve can be expressed **recursively** as a linear blend of two degree-$(n-1)$ Bézier curves:

$$P^n(t) = (1-t) \cdot P^{n-1}_{\text{first } n \text{ points}}(t) + t \cdot P^{n-1}_{\text{last } n \text{ points}}(t)$$

This is the foundation of the **de Casteljau algorithm** — instead of evaluating the full polynomial, you repeatedly interpolate between control points until you get a single point on the curve. It's numerically stable and the standard way to actually compute Bézier curves in practice.

---
$$B_n(P_1...P_{n+1}, t) = t \cdot B_{n-1}(P_2...P_{n+1}, t) + (1-t) \cdot B_{n-1}(P_1...P_n, t)$$

**Base case**: $B_0(P_i) = P_i$ (single point)

```c
// Recursive implementation
Point B(Point P[], int i, int j, float t) {
    if (i == j) return P[i];  // Single control point
    
    // Linear interpolation between sub-curves
    return t * B(P, i+1, j, t) + (1-t) * B(P, i, j-1, t);
}
```



**Example for Cubic Bezier** $B(P_1, P_2, P_3, P_4)$:
![[FACL/ComputerGraphics/Chapters/LEC5/Attachments/image.png]]
- First level: $B(P_2,P_3,P_4)$ and $B(P_1,P_2,P_3)$
- Second level: Further subdivides until reaching individual points
- Final interpolation gives the point on the curve

> [!example] **Worked Example: Finding the point at t=0.5**
> Given control points: $P_1=(0,0)$, $P_2=(1,2)$, $P_3=(2,1)$, $P_4=(3,3)$
> 
> **Step 1 - First level (t=0.5):**
> - $q_1 = 0.5 \cdot P_2 + 0.5 \cdot P_1 = 0.5 \cdot (1,2) + 0.5 \cdot (0,0) = (0.5, 1)$
> - $q_2 = 0.5 \cdot P_3 + 0.5 \cdot P_2 = 0.5 \cdot (2,1) + 0.5 \cdot (1,2) = (1.5, 1.5)$
> - $q_3 = 0.5 \cdot P_4 + 0.5 \cdot P_3 = 0.5 \cdot (3,3) + 0.5 \cdot (2,1) = (2.5, 2)$
> 
> **Step 2 - Second level:**
> - $r_1 = 0.5 \cdot q_2 + 0.5 \cdot q_1 = 0.5 \cdot (1.5,1.5) + 0.5 \cdot (0.5,1) = (1, 1.25)$
> - $r_2 = 0.5 \cdot q_3 + 0.5 \cdot q_2 = 0.5 \cdot (2.5,2) + 0.5 \cdot (1.5,1.5) = (2, 1.75)$
> 
> **Step 3 - Final point:**
> - $P(t=0.5) = 0.5 \cdot r_2 + 0.5 \cdot r_1 = 0.5 \cdot (2,1.75) + 0.5 \cdot (1,1.25) = (1.5, 1.5)$
> 
> The point on the curve at t=0.5 is **(1.5, 1.5)** — this is the midpoint of the Bezier curve defined by these control points.

#### Properties of Bezier Curves

| Property | Explanation |
|----------|-------------|
| **Endpoint Interpolation** | Curve passes exactly through $P_1$ (at $t=0$) and $P_{n+1}$ (at $t=1$) |
| **Convex Hull Property** | The entire curve lies within the convex hull of its control points |
| **Affine Invariance** | Transforming control points then computing the curve = computing the curve then transforming it |
| **Variation Diminishing** | A line crosses the curve no more times than it crosses the control polygon |
| **Global Control** | Moving any control point affects the **entire** curve (unlike B-splines) |
| **Symmetry** | The curve is symmetric: reversing control points produces the same curve traced backward |

> [!warning] **Global Control Limitation**
> In Bezier curves, every control point influences the entire curve. If you move $P_1$, even the region near $P_{n+1}$ changes slightly. This makes precise local editing impossible — which is why we introduce splines like B-splines that offer **local control**.

#### Practical Rendering: Midpoint Subdivision
**The Problem**: Evaluating the recursive formula for every pixel is expensive.

**The Solution**: The midpoint $t = 0.5$ of a Bezier curve of degree $n$ divides it into **two Bezier curves of the same degree**, each with new control points calculated through repeated midpoint interpolation.
this Cubic ==Bezier== Curve :
![[FACL/ComputerGraphics/Chapters/LEC5/Attachments/image-1.png]]

```c
void DrawBezier(Point P1, Point P2, Point P3, Point P4) {
    // Calculate midpoints (De Casteljau construction)
    Point q1 = (P1 + P2) / 2;
    Point q2 = (P2 + P3) / 2;
    Point q3 = (P3 + P4) / 2;
    
    Point r1 = (q1 + q2) / 2;
    Point r2 = (q2 + q3) / 2;
    
    Point midpoint = (r1 + r2) / 2;
    
    // Termination condition: curve is flat enough
    if (distance(P1, P4) < threshold) {
        drawLine(P1, P4);
        return;
    }
    
    // Recursively draw both halves
    DrawBezier(P1, q1, r1, midpoint);
    DrawBezier(midpoint, r2, q3, P4);
}
```

**Why this matters**: This converts the mathematical curve into a **polyline approximation** that can be rendered efficiently, while maintaining visual smoothness through adaptive subdivision.

> [!tip] Midpoint subdivision is the bridge between pure math and renderable graphics — it lets us approximate smooth curves with straight line segments while controlling error through adaptive depth.

---

## Splines

**The Limitation**: Individual Bezier curves connect with only $C^0$ continuity (position matching). For *smooth modeling*, we need higher-order continuity.

### Hermite Curves: The Building Block

> [!note] **Prerequisite for Cardinal Splines**
> Before understanding Cardinal splines, you must understand Hermite curves — a curve defined by two endpoints and their tangent vectors.

**Definition**: A Hermite curve is fully defined by:
- **Two endpoints**: $P_0$ and $P_1$
- **Two tangent vectors**: $T_0$ (at $P_0$) and $T_1$ (at $P_1$)

**Hermite Basis Functions**:
$$P(t) = (2t^3 - 3t^2 + 1)P_0 + (-2t^3 + 3t^2)P_1 + (t^3 - 2t^2 + t)T_0 + (t^3 - t^2)T_1$$

Where:
- $H_0(t) = 2t^3 - 3t^2 + 1$ — weight for $P_0$
- $H_1(t) = -2t^3 + 3t^2$ — weight for $P_1$
- $H_2(t) = t^3 - 2t^2 + t$ — weight for $T_0$
- $H_3(t) = t^3 - t^2$ — weight for $T_1$

**Why this matters**: If you stitch multiple Hermite curves together and ensure tangent continuity at joints, you achieve **$C^1$ continuity** — exactly what Cardinal splines do automatically.

### Cardinal Splines: $C^1$ Continuity via Hermite Curves
> Cardinal Splines are based on ==Hermite== Curves — they automatically compute tangent vectors from control points.

**Core Concept**: Cardinal splines ensure that curves pass through control points with **matching tangents** (first derivatives) at joints.
![[FACL/ComputerGraphics/Chapters/LEC5/Attachments/image-2.png]]
**Tangent Calculation**:
$$T_i = c \cdot (P_{i+1} - P_{i-1})$$

Where:
- $c$ is a  Constant for  tension parameter (controls "tightness" of the curve)
- $T_i$ is the tangent at point $P_i$

>[!danger] for curve to be smooth Tangent must be 1.

**Algorithm**:
```c
void CardinalSpline(Point P[], int n, float c) {
    Vector T[n];
    
    // Calculate tangents for interior points
    for (i = 1; i < n-1; i++) {
        T[i] = c * (P[i+1] - P[i-1]);
    }
    
    // Handle endpoints (mirror tangents or set to zero)
    T[0] = T[1];
    T[n-1] = T[n-2];
    
    // Draw Hermite segments between each pair
    for (i = 0; i < n-1; i++) {
        DrawHermite(P[i], T[i], P[i+1], T[i+1]);
    }
}
```

**Why Hermite?**: Hermite curves are defined by two points and two tangents — perfect for stitching together with guaranteed $C^1$ continuity (continuous first derivative).

### B-Splines: $C^2$ Continuity and Local Control
**Recall** :
$$
P(t) = \sum_{r=0}^{n} \underbrace{C_r^n \cdot t^r (1-t)^{n-r}}_{\text{Basis Function}} \cdot P_{r+1}
$$
Basis function of Bezier curves:
$$
B^{n}_{r} = C_r^n \cdot t^r (1-t)^{n-r} \quad \text{for } r = 1,2,3,...,n+1
$$
$$
B_r^{(n)} = t \, B_{r-1}^{(n-1)} + (1 - t)\, B_r^{(n-1)}
\quad ,\text{Recursive formula}
$$


**The Evolution**: 
- **Bezier**: Global control (moving one point changes entire curve), $C^0$ between segments
- **Cardinal**: Local control, $C^1$ continuity
- **B-Splines**: Local control, $C^2$ continuity (continuous second derivative = smoother curvature)

**Key Difference**: In Bezier curves, the number of control points determines the degree. In B-splines, **degree is independent** of control point count.

**Basis Functions**: B-splines use more complex basis functions $N_{i,k}(t)$ (where $k$ is degree and $i$ is the control point index) defined recursively.

> [!tip] **Intuition for $N_{i,k}(t)$**
> - $N_{i,k}(t)$ is the weight applied to control point $P_i$
> - It's non-zero only over $k+1$ knot intervals — this gives **local control**
> - Higher $k$ = smoother basis functions = smoother curve
> - Unlike Bezier's Bernstein polynomials, B-spline basis functions are **piecewise polynomials**  

**Base Case**: ($k=0$)
$$
N_{i,0}(t)=
\begin{cases}
1, & t \in [t_i, t_{i+1}] \\
0, & \text{otherwise}
\end{cases}
$$
This means: at degree 0, the basis function is just a "hat" over one interval — it's 1 near $P_i$ and 0 everywhere else.

**Recurrence Case**: ($k > 0$)
$$N_{i,k}(t) = \frac{t-t_i}{t_{i+k}-t_i} N_{i,k-1}(t) + \frac{t_{i+k+1}-t}{t_{i+k+1}-t_{i+1}} N_{i+1,k-1}(t)$$
This builds higher-degree basis functions by **linearly blending** lower-degree ones — similar in spirit to de Casteljau for Bezier.

**Case $k=1$** : 
$$
N_{i,1}(t) = \frac{t-i}{1} \underbrace {N_{i,0}(t)}_{1 \text{ in }[i,i+1]} + \frac{i+2-t}{1} \underbrace{N_{i+1,0}(t)}_{1 \text{ in }[i+1,i+2]}
$$
$$
=
\begin{cases}
t - i, & t \in [i, i+1] \\
2 + i - t, & t \in [i+1, i+2]
\end{cases}
$$
where:
- $t \in [i, i+1]$ for the first term
- $t \in [i+1, i+2]$ for the second term
	
**The Knot Vector**: Control points are defined at parameter values $(t_1, t_2, ..., t_n)$ called the **knot vector**. This non-uniform spacing allows for:
- Variable curve "tension"
- Exact control over where the curve passes near control points
- $C^2$ continuity across joints

> [!note] **Knot Vector Definition**: A knot vector is a non-decreasing sequence of parameter values $t_0, t_1, ..., t_m$ that determines where and how basis functions influence the B-spline curve. Each basis function $N_{i,k}(t)$ (where $k$ is degree) is non-zero only within $[t_i, t_{i+k+1}]$. The spacing of knots controls:
> - **Where** the curve passes near control points
> - **How sharply** the curve bends between joints
> - **Continuity** order at each knot (removing a knot reduces continuity by 1)

> [!tip] Catmull-Rom splines are just Cardinal splines with tension parameter $c = 0.5$. This is the most commonly used variant in practice.

**Continuity Hierarchy**:
- $C^0$: Connected (position matches)
- $C^1$: Smooth (tangent matches)
- $C^2$: Very smooth (curvature matches) — achieved by B-splines

### Comparison: Bezier vs Cardinal vs B-Splines

| Property | Bezier | Cardinal | B-Spline |
|----------|--------|----------|----------|
| **Continuity between segments** | $C^0$ (position only) | $C^1$ (tangent) | $C^2$ (curvature) |
| **Control type** | Global | Local | Local |
| **Degree vs control points** | Degree = $n-1$ for $n$ points | Independent | Independent |
| **Passes through points?** | Endpoints only | All control points | Generally no |
| **Intuitive editing?** | Difficult (global changes) | Easy (tension param) | Easy (local edit) |
| **Mathematical basis** | Bernstein polynomials | Hermite interpolation | Recursive basis $N_{i,k}$ |

> [!tip] **When to use each**
> - **Bezier**: Simple curves, fonts, vector graphics — when you need exact endpoint control
> - **Cardinal/Catmull-Rom**: Animation paths, camera movements — when curve must pass through waypoints
> - **B-Spline**: CAD, 3D modeling — when you need smooth local editing without affecting entire curve

---

## Summary: Key Takeaways

1. **Bezier curves** use control points weighted by Bernstein polynomials — recursive de Casteljau algorithm enables efficient evaluation and subdivision.

2. **Midpoint subdivision** converts mathematical Bezier curves into renderable polylines by recursively splitting the curve until "flat enough."

3. **Hermite curves** are defined by two endpoints and their tangents — the foundation for stitching smooth curves together.

4. **Cardinal splines** automatically compute tangent vectors from neighboring control points, ensuring $C^1$ continuity — Catmull-Rom is the common variant ($c=0.5$).

5. **B-splines** achieve $C^2$ continuity with **local control** — moving one control point affects only a local region, not the entire curve.

6. **Knot vectors** control where B-spline basis functions are non-zero, enabling precise control over curve behavior and continuity.

7. **Continuity hierarchy**: $C^0$ (position) → $C^1$ (tangent) → $C^2$ (curvature) — higher continuity = smoother appearance but more mathematical constraints.

---
