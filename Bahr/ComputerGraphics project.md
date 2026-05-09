---
created: 2026-05-08 12:04
modified: 2026-05-08 12:18
tags:
  - Projects
  - ComputerGraphics
source:
---


**✅ Project Description**

This is a **Computer Graphics Term Project** (3rd Year – Faculty of Computers and AI, Cairo University) that requires you to build a **complete 2D Drawing Package** (like a simplified Paint program) from scratch. 

The main goal is to **implement classic computer graphics algorithms** (line, circle, ellipse, curve, filling, and clipping) and integrate them into a user-friendly interactive application using **menus** and **mouse interaction only**.

---

### **Core Requirements (10 Marks)**

You must implement a program with a graphical window that has the following **menus**:

#### **1. File Menu**
- Clear the screen (remove all shapes)
- Save all drawn shapes/data to a file
- Load previously saved file

#### **2. Preferences Menu**
- Change background color to **white**
- Change the shape/style of the mouse cursor
- Choose drawing **color** before drawing any shape

#### **3. Lines Menu**
Implement **3 line drawing algorithms**:
- DDA
- Midpoint
- Parametric

#### **4. Circles Menu**
Implement **5 circle drawing algorithms**:
- Direct (Cartesian)
- Polar
- Iterative Polar
- Midpoint
- Modified Midpoint

#### **5. Ellipse Menu**
Implement **3 ellipse algorithms**:
- Direct
- Polar
- Midpoint

#### **6. Curves Menu**
- **Cardinal Spline Curve**

#### **7. Filling Menu**
- Fill Circle using **lines** (user chooses quarter)
- Fill Circle using **smaller circles** (user chooses quarter)
- Fill Square using **Hermite Curve** (Vertical)
- Fill Rectangle using **Bezier Curve** (Horizontal)
- Convex & Non-Convex Polygon Filling
- **Recursive** and **Non-Recursive** Flood Fill

#### **8. Clipping Menu**
- **Rectangle** as clipping window → Clip **Point**, **Line**, and **Polygon**
- **Square** as clipping window → Clip **Point** and **Line**

#### **General Requirements**
- User must draw **everything using the mouse only** (no keyboard for drawing)
- Use the **console** to show logs, messages, and take some inputs when needed

---

### **Bonus Marks (4 Marks)**

1. **Circle Clipping** – Use a circle as a clipping window for Point & Line.
2. **Smiley Faces**:
   - **Happy Face**
   - **Sad Face**

   Using:
   - Circles (face + eyes)
   - Curves (mouth)
   - Lines (nose)


---
# project roles for each one of the teams :
## 👤 Person 1 — Core + Filling

- Clear screen
- Line Parametric
- Ellipse Direct
- Convex Filling
- Non-Convex Filling
- Smiley Faces (Bonus)

### Why?

Balanced with:

- geometry
- filling
- bonus

---

## 👤 Person 2 — File System + Circle + Rectangle Clipping

- Save screen to file
- Load screen from file
- Direct Circle
- Circle filling with circles
- Rectangular window clipping:
    - Point
    - Line
    - Polygon

### Why?

Owns serialization + major clipping system.

---

## 👤 Person 3 — UI + Lines (My part)

- Change background color
- Choose shape color
- DDA Line 
- Midpoint Line
- Modified Midpoint Circle
- Change mouse pointer

### Why?

Good mix of:

- UI
- interaction
- algorithms

---

## 👤 Person 4 — Curves + Flood Fill 

- Ellipse Polar
- Rectangle filling with Bezier
- Square filling with Hermite
- Recursive Flood Fill
- Non-Recursive Flood Fill
- Cardinal Spline Curve

### Why?

All curve-related work grouped together logically.

---

## 👤 Person 5 — Advanced Circle + Clipping

- Ellipse Midpoint
- Circle Polar
- Circle Iterative Polar
- Circle filling with lines
- Circular window clipping (Bonus)

### Why?

Heavy mathematical algorithms grouped together.

---

# What to do?

I am **Person 3**, so the AI/developer must implement only my assigned part unless integration with the rest of the team requires small shared helpers.

## My Required Tasks

- Change background color from the **Preferences** menu.
- Choose the current drawing color before drawing shapes.
- Draw a line using **DDA Line**.
  - Reference: [[FACL/ComputerGraphics/Chapters/LEC2/LEC2#Line Digital Differential analyser ($DDA$)|DDA algorithm]]
- Draw a line using **Midpoint Line**.
  - Reference: [[FACL/ComputerGraphics/Chapters/LEC2/LEC2#MidPoint Algorithm|MidPoint algorithm]]
- Draw a circle using **Modified Midpoint Circle**.
  - Reference: [[FACL/ComputerGraphics/Chapters/LEC3/Circle Drawing Algorithms#5.6 The Faster Bresenham Variant, Based on $2^{nd}$ order difference .|Modified/Faster Midpoint Circle]]
- Change the mouse pointer from the **Preferences** menu.

## Implementation Instructions for the AI

- The project must be implemented in **C++**.
- Before writing code, inspect and reuse the style/ideas from the existing course files under:
  - `/home/mohammed_bahr/Projects/Obsidian/FACL/ComputerGraphics`
  - Especially:
    - `Chapters/LEC2/LEC2.md` for DDA and Midpoint Line.
    - `Chapters/LEC3/Circle Drawing Algorithms.md` for Modified Midpoint Circle.
    - `Chapters/LEC7/openGL1.cpp` only as a possible window/OpenGL reference, not as an algorithm replacement.
- Implement every required drawing algorithm **from scratch** using the mathematical steps in the notes.
- Do **not** use built-in drawing algorithms for the required shapes.
  - Do not use `LineTo`, `Polyline`, `Ellipse`, `Arc`, `Circle`, `Rectangle`, OpenGL line/circle primitives, or library shape functions to draw DDA lines, Midpoint lines, or Modified Midpoint circles.
  - It is allowed to use `SetPixel`/pixel plotting for the algorithms and Win32/OpenGL only for creating the window, menus, mouse events, cursor changes, colors, and screen refresh.
- Drawing input must use the **mouse only**:
  - For DDA Line and Midpoint Line: first click selects start point, second click selects end point.
  - For Modified Midpoint Circle: first click selects center, second click selects a point on the radius.
- Menu choices should set the current mode:
  - Current algorithm: DDA Line, Midpoint Line, or Modified Midpoint Circle.
  - Current drawing color.
  - Current background color.
  - Current mouse cursor style.
- Print helpful messages to the console when the user changes mode/color/cursor/background or completes a shape.
- Keep the code ready for team integration:
  - Use clear function names such as `DrawLineDDA`, `DrawLineMidpoint`, `DrawCircleModifiedMidpoint`, and `Draw8Points`.
  - Keep menu IDs/constants organized.
  - Store enough shape data to redraw my shapes if the window repaints or when the team integrates save/load later.

## Acceptance Criteria

- The program opens a graphical window with menus for my assigned tasks.
- User can change background color.
- User can choose shape color before drawing.
- User can change mouse pointer.
- User can select DDA Line and draw using two mouse clicks.
- User can select Midpoint Line and draw using two mouse clicks.
- User can select Modified Midpoint Circle and draw using two mouse clicks.
- The three required algorithms are implemented manually from the lecture math, not by built-in graphics functions.
