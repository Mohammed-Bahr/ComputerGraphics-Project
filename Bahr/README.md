# Computer Graphics Project - Person 3 Implementation

## Overview

This is **Person 3's contribution** to the Computer Graphics Term Project (3rd Year – Faculty of Computers and AI, Cairo University). This implementation provides a 2D drawing application with the following features:

### Implemented Features

✅ **Preferences Menu:**
- Change background color to white
- Choose custom drawing colors (full color picker)
- Change mouse cursor style (Arrow, Crosshair, Hand)

✅ **Lines Menu:**
- **DDA Line Algorithm** - Digital Differential Analyzer for line drawing
- **Midpoint Line Algorithm** - Bresenham's line drawing algorithm

✅ **Circles Menu:**
- **Modified Midpoint Circle** - Faster Bresenham variant using 2nd order differences

✅ **Additional Features:**
- Clear screen functionality
- Mouse-only drawing (no keyboard required)
- Shape persistence (redraws on window refresh)
- Console logging for all actions

---

## Algorithm Implementations

### 1. DDA Line Algorithm
**Reference:** `LEC2.md` - Digital Differential Analyzer

The DDA algorithm uses incremental calculation to draw lines efficiently:
- For gentle slopes (|m| ≤ 1): Steps along x-axis, calculates y
- For steep slopes (|m| > 1): Steps along y-axis, calculates x
- Uses floating-point arithmetic for accuracy

**Key Features:**
- Handles all line orientations (horizontal, vertical, diagonal)
- Smooth line rendering without gaps
- Implements proper rounding for pixel placement

### 2. Midpoint Line Algorithm (Bresenham's)
**Reference:** `LEC2.md` - MidPoint Algorithm

An optimized integer-only line drawing algorithm:
- Uses decision variable to choose pixels
- No floating-point arithmetic (faster than DDA)
- Handles all slopes and directions

**Key Features:**
- Pure integer operations for maximum speed
- Bidirectional drawing support
- Gap-free continuous lines

### 3. Modified Midpoint Circle (Faster Bresenham)
**Reference:** `Circle Drawing Algorithms.md` - Section 5.6

The fastest circle drawing algorithm using 2nd order differences:
- Exploits 8-way circle symmetry (computes 1/8, reflects to draw full circle)
- Uses incremental decision variables (c1 and c2)
- Integer-only arithmetic

**Key Features:**
- Only 2-4 integer operations per pixel
- Perfect circles without gaps or duplicates
- Stops at x < y to avoid redundant octant overlap

---

## Compilation Instructions

### Windows (Visual Studio)

1. **Using Visual Studio IDE:**
   ```
   1. Open Visual Studio
   2. Create New Project → Empty C++ Project
   3. Add main.cpp to the project
   4. Build → Build Solution (Ctrl+Shift+B)
   5. Run (F5)
   ```

2. **Using Command Line (with Visual Studio installed):**
   ```cmd
   # Open Developer Command Prompt for VS
   cl /EHsc main.cpp user32.lib gdi32.lib comdlg32.lib /Fe:GraphicsProject.exe
   ```

### Windows (MinGW/g++)

```bash
g++ main.cpp -o GraphicsProject.exe -lgdi32 -luser32 -lcomdlg32 -mwindows
```

### Linux (Wine + MinGW cross-compiler)

```bash
x86_64-w64-mingw32-g++ main.cpp -o GraphicsProject.exe -lgdi32 -luser32 -lcomdlg32 -static-libgcc -static-libstdc++
wine GraphicsProject.exe
```

---

## Usage Instructions

### Starting the Application

1. Run the compiled executable
2. A window titled **"Computer Graphics Project - Person 3"** will appear
3. A console window will open showing program messages

### Drawing Lines (DDA or Midpoint)

1. **Select Line Mode:**
   - Menu → **Lines** → **DDA** (or **Midpoint**)
   - Console message: `[DDA Line Mode] Click two points to draw a line.`

2. **Draw the Line:**
   - Click the **first point** (start) anywhere in the window
   - Console shows: `First point: (x, y)`
   - Click the **second point** (end)
   - Console shows: `Second point: (x, y)`
   - Line is drawn immediately

3. **Continue Drawing:**
   - Click two more points to draw another line
   - Repeat as many times as needed

### Drawing Circles (Modified Midpoint)

1. **Select Circle Mode:**
   - Menu → **Circles** → **Modified Midpoint**
   - Console message: `[Modified Midpoint Circle Mode] Click center, then a point on the radius.`

2. **Draw the Circle:**
   - Click the **center point**
   - Console shows: `First point: (x, y)`
   - Click any point to define the **radius** (distance from center to this point)
   - Console shows radius calculation
   - Circle is drawn immediately using 8-way symmetry

### Changing Colors

**Background Color:**
- Menu → **Preferences** → **Background: White**
- Sets background to white (default is black)

**Drawing Color:**
- Menu → **Preferences** → **Choose Drawing Color...**
- A color picker dialog appears
- Select any color you want
- All subsequent shapes will use this color

### Changing Mouse Cursor

- Menu → **Preferences** → **Mouse Cursor** → Select:
  - **Arrow** (default)
  - **Crosshair** (good for precise drawing)
  - **Hand**

### Clearing the Screen

- Menu → **File** → **Clear Screen**
- Removes all drawn shapes
- Resets to the current background color

---

## Technical Details

### Coordinate System
- Origin (0,0) is at the **top-left** corner of the window
- X increases to the right
- Y increases downward

### Shape Storage
All drawn shapes are stored in a `vector<Shape>` structure containing:
- Drawing mode (DDA, Midpoint Line, or Modified Midpoint Circle)
- Color (COLORREF)
- Start point (x1, y1)
- End point (x2, y2) - or radius point for circles

This allows:
- Automatic redrawing when window is resized/minimized/restored
- Future integration with save/load functionality (Person 2's responsibility)

### Console Logging
The program prints helpful messages to the console:
- Mode changes
- Color changes
- Cursor changes
- Each point clicked
- Shape completion with coordinates

---

## Code Structure

```Projects/ComputerGraphics_Person3/main.cpp#L1-30
/*
 * Computer Graphics Project - Person 3 Implementation
 * 
 * Responsibilities:
 * - Change background color (Preferences menu)
 * - Choose shape drawing color (Preferences menu)
 * - DDA Line algorithm
 * - Midpoint Line algorithm (Bresenham's)
 * - Modified Midpoint Circle algorithm (Faster Bresenham variant)
 * - Change mouse pointer (Preferences menu)
 * 
 * All drawing uses mouse-only input (no keyboard)
 */

#include <windows.h>
#include <cmath>
#include <vector>
#include <iostream>

using namespace std;

// ============================================================================
// MENU IDS
// ============================================================================
#define IDM_PREFERENCES_BG_WHITE       101
#define IDM_PREFERENCES_CHOOSE_COLOR   102
#define IDM_PREFERENCES_CURSOR_ARROW   103
#define IDM_PREFERENCES_CURSOR_CROSS   104
#define IDM_PREFERENCES_CURSOR_HAND    105
```

### Key Functions:

```Projects/ComputerGraphics_Person3/main.cpp#L98-136
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Check if the line is more horizontal than vertical (|slope| <= 1)
    if (abs(dy) <= abs(dx)) {
        double m = (double)dy / dx;

        // Ensure we always draw from left to right
        if (x1 > x2) swap(x1, y1, x2, y2);

        SetPixel(hdc, x1, y1, c);

        int x = x1;
        double y = y1;

        while (x < x2) {
            x++;
            y += m;
            SetPixel(hdc, x, Round(y), c);
        }
    }
    else {
        // Steep slope: step along y-axis
        double mi = (double)dx / dy;
        
        if (y1 > y2) swap(x1, y1, x2, y2);

        SetPixel(hdc, x1, y1, c);

        int y = y1;
        double x = x1;

        while (y < y2) {
            y++;
            x += mi;
            SetPixel(hdc, Round(x), y, c);
        }
    }
}
```

```Projects/ComputerGraphics_Person3/main.cpp#L142-191
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    
    // Determine direction of line
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    
    int x = x1;
    int y = y1;
    
    // Case 1: Gentle slope (|m| <= 1)
    if (dx >= dy) {
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
    // Case 2: Steep slope (|m| > 1)
    else {
        int d = dy - 2 * dx;
        int delta_d1 = 2 * dy - 2 * dx;
        int delta_d2 = -2 * dx;
        
        SetPixel(hdc, x, y, c);
        
        while (y != y2) {
            if (d < 0) {
                x += sx;
                d += delta_d1;
            } else {
                d += delta_d2;
            }
            y += sy;
            SetPixel(hdc, x, y, c);
        }
    }
}
```

```Projects/ComputerGraphics_Person3/main.cpp#L197-218
void DrawCircleModifiedMidpoint(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R;
    int d = 1 - R;                    // Initial decision variable
    int c1 = 3;                       // Initial increment value (2*x + 3)
    int c2 = 5 - 2 * R;               // Initial increment value (2*(x-y) + 5)
    
    Draw8Points(hdc, xc, yc, x, y, color);
    
    while (x < y) {
        if (d < 0) {
            // Move right only (midpoint inside circle)
            d += c1;
            c2 += 2;
        } else {
            // Move right and down (midpoint outside circle)
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

---

## Integration Notes for Team

This implementation is designed for easy integration with team members:

### For Person 2 (File System):
- The `g_shapes` vector can be serialized to save all drawn shapes
- Each `Shape` struct contains all necessary data (mode, color, coordinates)
- Use the `RenderShape()` function to redraw loaded shapes

### For Person 1, 4, 5 (Other Algorithms):
- Add new menu items and `#define` constants for your algorithms
- Add new drawing modes to the `DrawMode` enum
- Implement your algorithms as separate functions (e.g., `DrawEllipseDirect()`)
- Add cases to the `WM_COMMAND` switch for menu handling
- Add cases to the `RenderShape()` function for redrawing

### Shared Components:
- `Draw8Points()` - Can be reused for all circle/ellipse algorithms
- `Round()` helper function
- Shape storage system
- Menu structure is modular and extensible

---

## Testing Checklist

- [x] DDA Line draws correctly in all directions
- [x] Midpoint Line draws correctly in all directions
- [x] Modified Midpoint Circle draws perfect circles
- [x] Background color changes to white
- [x] Color picker works and updates drawing color
- [x] Mouse cursor changes (Arrow, Crosshair, Hand)
- [x] Clear screen removes all shapes
- [x] Shapes persist after window resize/minimize
- [x] Console shows helpful messages
- [x] Window compiles without warnings

---

## Known Limitations

1. **Background Color**: Currently only supports changing to white (can be extended to full color picker)
2. **Platform**: Windows-only (uses Win32 API)
3. **No Undo**: Shapes can only be removed by clearing all
4. **No Save/Load**: This is Person 2's responsibility

---

## References

- **LEC2.md**: DDA and Midpoint Line algorithms
- **Circle Drawing Algorithms.md**: Modified Midpoint Circle (Section 5.6)
- Win32 API Documentation
- Course lecture materials from `/home/mohammed_bahr/Projects/Obsidian/FACL/ComputerGraphics`

---

## Author

**Person 3** - UI & Interaction + Line/Circle Algorithms

For questions or integration help, contact through the project team communication channels.
