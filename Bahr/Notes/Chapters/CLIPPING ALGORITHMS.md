---
Created: " 2026-05-07 11:29"
modified: 2026-05-07 11:29
tags:
  - FACL
  - ComputerGraphics
  - "#ComputerGraphics/ClippingAlgorithms"
Doc Lec order: LEC 7
Source: "https://onedrive.live.com/view.aspx?resid=530E6BF1AA159903%21s607f2ff402ba4f709ccbe35664b7971e&id=documents&wd=target%28Computer%20Graphics.one%7CBC6717AA-DCF2-EF4D-BAA9-DC46AB0FFF58%2FLEC7%7CD145887C-6D0D-B64C-9B77-D87F28EF4193%2F%29&wdpartid={BB3F51A4-9628-7E4F-9B22-F0297BBDED11}{1}&wdsectionfileid=530E6BF1AA159903!s35a2701b6f4d458b967156f6ece30219&end\r

  onenote:https://d.docs.live.net/530E6BF1AA159903/Documents/FACL/Computer%20Graphics.one#LEC7&section-id={BC6717AA-DCF2-EF4D-BAA9-DC46AB0FFF58}&page-id={D145887C-6D0D-B64C-9B77-D87F28EF4193}&end"
---
> [!quote] People who say it cannot be done should not interrupt those who are doing it.
> — Chinese proverb

**"اللهم إني أسألك فهم النبيين، وحفظ المرسلين، والملائكة المقربين، اللهم اجعل ألسنتنا عامرة بذكرك، وقلوبنا بخشيتك، إنك على كل شيء قدير"**

---


## PART 1: THE BIG PICTURE — WHY DO WE NEED CLIPPING?

### The Viewing Pipeline (World → Screen)
![[Screenshot_20260507-113917.png]]
The lecture starts with the fundamental problem: We have objects described in the real world (==world space==), but we need to display them on a ==computer screen==.

**Key Concepts:**

1. **World Space**: Where objects exist with real units (meters, centimeters, inches). Objects can be anywhere, infinitely large.
2. **Clipping Window**: A rectangular region in world space that defines "what the camera sees"
3. **Viewport**: The corresponding rectangular region on the actual screen (measured in pixels)
4. **Window-to-Viewport Mapping**: The transformation that maps content from the clipping window to the viewport
 
**The Pipeline Flow:**
```
World Objects → Clipping (remove what's outside window) → Window-to-Viewport Mapping → Screen Display
```

**Critical Point from Lecture**: The clipping operation is **performance-critical** because:
- World scenes contain MANY objects (potentially thousands or millions of polygons)
- Calculating lighting, shading, and rendering for objects that won't appear on screen is wasted computation
- Clipping eliminates unnecessary computations early in the pipeline, speeding up the entire rendering process

---

## PART 2: $2D$ vs $3D$ CLIPPING

### $2D$ Clipping
- The clipping window is a **rectangle** (flat)
- We test points, lines, or polygons against this rectangle
- Four edges to test against: left, right, top, bottom

### $3D$ Clipping
![[image.png]]
- The clipping volume is a *volume* (not flat) — could be a box, pyramid (frustum), or other shape
- The lecture mentions that $3D$ clipping volumes are often approximated as *pyramids* or *frustums* for easier computation
- After $3D$ clipping, we do *projection* to convert $3D$ to $2D$, then *viewport mapping*

> [!Important] The focus primarily on $2D$ ==clipping algorithms==, noting that $3D$ extensions are "straightforward in many cases."

---

## PART 3: TYPES OF CLIPPING ALGORITHMS

| Type | What It Does | Complexity |
|------|-------------|------------|
| **Point Clipping** | Tests if a single point is inside the window | Trivial |
| **Line Clipping** | Tests line segments, clips them to window edges | Moderate |
| **Polygon Clipping** | Tests entire polygons, may create new edges | Complex |

---

## PART 4: POINT CLIPPING (The Simplest Case)

### The Concept
A point (x, y) is inside the clipping window if and only if:
- `x` is between `xleft` and `xright` (inclusive)
- `y` is between `ybottom` and `ytop` (inclusive)

### The Algorithm :
```c
bool IsInside(x, y, xleft, ybottom, xright, ytop)
    return (x >= xleft) && (x <= xright) && (y >= ybottom) && (y <= ytop) 

```

### The C++ Implementation :
```cpp
void PointClipping(int x, int y, int xleft, int ytop, int xright, int ybottom, COLORREF color)
{
    if(x >= xleft && x <= xright && y >= ytop && y <= ybottom)
        SetPixel(hdc, x, y, color);
}
```

**Important Note from Lecture**: In the PDF's implementation, `ytop` and `ybottom` are **swapped** compared to the mathematical definition. This is because:
- In **world coordinates**: y-axis points UP, so `ybottom < ytop`
- In **screen coordinates**: y-axis points DOWN, so `ytop < ybottom`
- The implementation assumes screen coordinates for simplicity (before learning window-to-viewport mapping)

---

## PART 5: LINE CLIPPING — THE COHEN-SUTHERLAND ALGORITHM

This is the **main algorithm** . ==Pay close attention==.
### The Problem
![[image-1.png]]
Given a line segment with two endpoints, determine:
1. Is it completely *inside*? → Draw it all
2. Is it completely *outside*? → Draw nothing
3. Does it cross the window boundary? → Calculate intersection points and draw only the visible portion

### The 9-Region Division

The algorithm divides the 2D plane into **9 regions** based on the clipping window:

```
          |         |  	        |
 Left-Top |   Top   | Right-Top |
----------|---------|-----------|-----y-top
   Left   | INSIDE  |   Right   |
----------|---------|-----------|-----y-bottom
LeftBottom| Bottom  |RightBottom|
	      |         |           |  	
		x-left     x-right
```

Each region is described by which "outsides" it has relative to the window:
```horizontal
- Inside: `{}` (empty set)
- Left: `{left}`
- Left-Top: `{left, top}`
- Top: `{top}`
- Right-Top: `{right, top}`
  ---
- Right: `{right}`
- Right-Bottom: `{right, bottom}`
- Bottom: `{bottom}`
- Left-Bottom: `{left, bottom}`
```

### OutCode Data Structure

This is **critical** to understanding the implementation. The lecture uses a **bit field** approach:
**Bit Assignment (4 bits total):**
- Bit 0 (value 1): ==LEFT==
- Bit 1 (value 2): ==RIGHT==  
- Bit 2 (value 4): ==BOTTOM==
- Bit 3 (value 8): ==TOP==

Wait, let me clarify . Looking at the actual implementation:

```cpp
union OutCode {
    unsigned All:4;  // All 4 bits together
    struct {
        unsigned left:1, top:1, right:1, bottom:1;
    };
};
```
	
So the bit order in the struct is: **left, top, right, bottom** (each 1 bit).
Actually, looking at the `GetOutCode` function in the PDF:
	
```cpp
OutCode GetOutCode(double x, double y, double xleft, double ytop, double xright, double ybottom)
{
    OutCode out;
    out.All = 0;
    if(x < xleft) out.left = 1; 
    else if(x > xright) out.right = 1;
	    
    if(y < ytop) out.top = 1; 
    else if(y > ybottom) out.bottom = 1;
	    
    return out;
}
```
So:
- `left = 1` if `x < xleft`
- `right = 1` if `x > xright`
- `top = 1` if `y < ytop` (remember screen coordinates: smaller y is higher up)
- `bottom = 1` if `y > ybottom`

In memory (typically), the first field is the least significant bit. So:
```horizontal
- `left` = bit 0 (value 1)
- `top` = bit 1 (value 2)
- `right` = bit 2 (value 4)
- `bottom` = bit 3 (value 8)
---
- inside : `0000`
- Left only: `0001` = 1
- Top only: `0010` = 2
- Left+Top: `0011` = 3
- Right only: `0100` = 4
- Bottom only: `1000` = 8
```


### The Three Tests (The Core of the Algorithm)

The **Cohen-Sutherland algorithm**, a standard method for line clipping against a rectangular window, categorises line segments into **three fundamental cases** based on the region "OutCodes" of their endpoints. These cases determine whether a line can be immediately processed or if it requires mathematical intersection calculations.

#### 1. Trivial Accept (Completely Inside)
![[image-2.png|300]]
A line is **trivially accepted** if both of its endpoints lie entirely within the clipping window.

- **Identification:** This is determined when the *OutCodes of both endpoints are empty sets* (or `0000` in bit representation).
- **Action:** The line is drawn in its entirety without any clipping.

#### 2. Trivial Reject (Completely Outside)
![[image-3.png|300]]
A line is **trivially rejected** if it is guaranteed to be completely outside the clipping window.
	
- **Identification:** This occurs if both endpoints share a *common outside region*. Mathematically, this is detected by performing a *bitwise AND operation* on the OutCodes of the two endpoints; if the result is non-zero, ==they share a common "outside" bit== (e.g., both are to the left of the window).
- **Action:** The line is completely discarded and not drawn at all.

#### 3. Must Clip (Non-Trivial/Partial Case)
![[image-4.png]]
If a line is neither trivially accepted nor trivially rejected, it is considered a **clipping case**.

- **Identification:** This happens when at least one endpoint is outside the window, but the two endpoints do **not** *share a common outside side*. This means the line segment may cross one or more boundaries of the clipping window.
- **Action:** The algorithm must pick an endpoint that is outside, calculate its **intersection** with a window boundary (priority order: `left`, `top`, `right`, `bottom`), and replace the old endpoint with this new intersection point. The algorithm then recalculates the OutCode for the new point and repeats the tests until the line is either accepted or rejected.
	
### Intersection Calculation
The core idea behind both functions is simply using the **parametric equation of a line**.

Given two points$(x_1, y_s)$ and $(x_2, y_e)$, any point on the line between them satisfies:
$$
(x - x_1) / (x_2 - x_1) = (y - ys) / (ye - ys)  =  t
$$
This gives you a parameter `t`. If you know one coordinate, you solve for the other.
>[!note] **So in short**: `xi` and `yi` are just the coordinates of the intersection point you're trying to find. One of them is always known (the edge coordinate), and the other is what you calculate.

**For `VIntersect` — vertical clipping edge (e.g. left or right boundary)**

The clipping edge is a vertical line $x = x_{edge}$ . You already know `xi`, so you solve for `yi`:

From the line equation:
$$
t = (x_{edge} - x_1) / (x_2 - x_1)
$$


Get $y_i$ :
$$
y_i = y_1 + t * (y_2 - y_1)
= y_1 + (x_{edge} - x_1) * (y_2 - y_1) / (x_2 - x_1)
$$
So `yi` is just `y1` _plus_ how far along the segment you are (in the x direction) _scaled to the y direction_. That's the entire formula.

```cpp
void VIntersect(double xs, double ys, double xe, double ye, int x_Edge, double *xi, double *yi)
{
    *xi = x_Edge;
    *yi = y1 + (x_Edge - x1) * (y2 - y1) / (x2 - x1);
}
```

**For `HIntersect` — horizontal clipping edge (e.g. top or bottom boundary)**

Same idea, but now you know $y_i = y_{edge}$ and you solve for `xi`:
$$
t = (y_{edge} - y_1) / (y_2 - y_1)
$$


Get $y_i$ :
$$
x_i = x_1 + t * (x_2 - x_1)
= x_1 + (y_{edge} - y_1) * (x_2 - x_1) / (y_2 - y_1)
$$
	
The two functions are mirrors of each other — just swap the roles of x and y.
```cpp
void HIntersect(double xs, double ys, double xe, double ye, int y_Edge, double *xi, double *yi)
{
    *yi = y_Edge;
    *xi = x1 + (y_Edge - y1) * (x2 - x1) / (y2 - y1);
}
```

Here's a diagram showing what's happening geometrically:**One important edge case** — notice the denominator in each function:
![[image-5.png]]
- `VIntersect` divides by `(x_2 - x_1)` — this blows up if the line is **perfectly vertical** (same x on both ends). But that can't intersect a vertical edge at a single point anyway, so it's not a real case.
- `HIntersect` divides by `(ye - ys)` — same story for **perfectly horizontal** lines and horizontal edges.


### Complete Cohen-Sutherland Implementation :

```cpp
void CohenSuth(HDC hdc, int xs, int ys, int xe, int ye,
               int xleft, int ytop, int xright, int ybottom)
{
    double x1 = xs, y1 = ys, x2 = xe, y2 = ye;
    
    OutCode out1 = GetOutCode(x1, y1, xleft, ytop, xright, ybottom);
    OutCode out2 = GetOutCode(x2, y2, xleft, ytop, xright, ybottom);
    
    while((out1.All || out2.All) && !(out1.All & out2.All))
    {
        double xi, yi;
        
        if(out1.All)  // First point is outside
        {
            if(out1.left)      VIntersect(x1, y1, x2, y2, xleft, &xi, &yi);
            else if(out1.top)  HIntersect(x1, y1, x2, y2, ytop, &xi, &yi);
            else if(out1.right)VIntersect(x1, y1, x2, y2, xright, &xi, &yi);
            else               HIntersect(x1, y1, x2, y2, ybottom, &xi, &yi);
            
            x1 = xi; y1 = yi;
            out1 = GetOutCode(x1, y1, xleft, ytop, xright, ybottom);
        }
        else  // Second point is outside
        {
            if(out2.left)      VIntersect(x1, y1, x2, y2, xleft, &xi, &yi);
            else if(out2.top)  HIntersect(x1, y1, x2, y2, ytop, &xi, &yi);
            else if(out2.right)VIntersect(x1, y1, x2, y2, xright, &xi, &yi);
            else               HIntersect(x1, y1, x2, y2, ybottom, &xi, &yi);
            
            x2 = xi; y2 = yi;
            out2 = GetOutCode(x2, y2, xleft, ytop, xright, ybottom);
        }
    }
    
    if(!out1.All && !out2.All)  // Both inside after clipping
    {
        MoveToEx(hdc, Round(x1), Round(y1), NULL);
        LineTo(hdc, Round(x2), Round(y2));
    }
}
```

### Priority Order for Edge Selection

The algorithm picks edges in this order: **LEFT → TOP → RIGHT → BOTTOM**

This order is arbitrary — you can change it without affecting correctness, but it may affect:
- **Number of iterations** needed (worst case: 4 iterations)
- **Performance** (fewer intersections = faster)

**Worst Case**: A line that needs to be clipped against all 4 edges (4 intersection calculations)
**Best Case**: Trivial accept or trivial reject (0 intersections)

---
