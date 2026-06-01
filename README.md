# ComputerGraphics-Project

**Project Description**

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

