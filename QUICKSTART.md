# Quick Start Guide

Get up and running with Person 3's Computer Graphics implementation in 5 minutes!

---

## 📦 What You Get

- ✅ DDA Line Algorithm
- ✅ Midpoint Line Algorithm (Bresenham's)
- ✅ Modified Midpoint Circle (Faster Bresenham)
- ✅ Color picker for drawing colors
- ✅ Background color control
- ✅ Mouse cursor customization
- ✅ Clear screen functionality
- ✅ Shape persistence on window refresh

---

## 🚀 Quick Compilation

### Option 1: Windows with MinGW (Easiest)

```bash
# Just run the build script
build.bat
```

### Option 2: Manual Compilation

```bash
# With g++/MinGW
g++ main.cpp -o GraphicsProject.exe -lgdi32 -luser32 -lcomdlg32 -mwindows

# With Visual Studio (Developer Command Prompt)
cl /EHsc main.cpp user32.lib gdi32.lib comdlg32.lib /Fe:GraphicsProject.exe
```

---

## 🎮 How to Use

### Step 1: Launch
```bash
./GraphicsProject.exe
# or just double-click GraphicsProject.exe
```

Two windows appear:
- **Graphics Window** - for drawing
- **Console Window** - for messages/logs

### Step 2: Draw Your First Line (DDA)

1. **Menu → Lines → DDA**
2. Console says: `[DDA Line Mode] Click two points to draw a line.`
3. **Click** anywhere in the window (first point)
4. **Click** somewhere else (second point)
5. 🎉 Line appears!

### Step 3: Draw Your First Circle

1. **Menu → Circles → Modified Midpoint**
2. Console says: `[Modified Midpoint Circle Mode] Click center, then a point on the radius.`
3. **Click** where you want the center
4. **Click** how far out you want the circle (defines radius)
5. 🎉 Perfect circle appears with 8-way symmetry!

### Step 4: Change Colors

**Drawing Color:**
1. **Menu → Preferences → Choose Drawing Color...**
2. Pick any color from the dialog
3. All future shapes use this color

**Background:**
1. **Menu → Preferences → Background: White**
2. Background changes to white
3. (Can toggle between black and white)

### Step 5: Try Different Mouse Cursors

**Menu → Preferences → Mouse Cursor → Select:**
- **Crosshair** ← Great for precise drawing!
- **Arrow** ← Default
- **Hand** ← Just for fun

---

## 🎯 Quick Test

Try drawing these to test all features:

1. **Horizontal line:** Click (100, 100), then (400, 100)
2. **Vertical line:** Click (200, 50), then (200, 350)
3. **Diagonal line:** Click (50, 50), then (350, 350)
4. **Small circle:** Click center, then close point (R ≈ 30)
5. **Large circle:** Click center, then far point (R ≈ 150)

Change to white background and red color, then draw a few more shapes to see persistence.

---

## 📋 Menu Structure

```
File
 └─ Clear Screen

Preferences
 ├─ Background: White
 ├─ Choose Drawing Color...
 └─ Mouse Cursor
     ├─ Arrow
     ├─ Crosshair
     └─ Hand

Lines
 ├─ DDA
 └─ Midpoint (Bresenham)

Circles
 └─ Modified Midpoint
```

---

## 🐛 Troubleshooting

### "Program won't compile"
- **Missing g++?** Install MinGW: https://www.mingw-w64.org/
- **Linux?** Use Wine + cross-compiler (see README.md)

### "Nothing happens when I click"
- Did you select a drawing mode from the menu first?
- Check the console - it shows what mode you're in

### "Colors not changing"
- Drawing color only affects NEW shapes
- Already drawn shapes keep their original color

### "Window is blank after resize"
- This is normal - shapes automatically redraw
- If not, it's a WM_PAINT issue (check code)

---

## 🎨 Tips & Tricks

1. **Precise Drawing:** Use the Crosshair cursor
2. **Multiple Shapes:** Just keep clicking - no need to reselect mode
3. **Clear Screen:** Menu → File → Clear Screen removes everything
4. **Console Messages:** Keep an eye on console for helpful info
5. **Test Symmetry:** Draw large circles to see perfect 8-way symmetry
6. **Compare Algorithms:** Draw same line with DDA vs Midpoint - they should be identical!

---

## 📊 Performance Notes

- **DDA:** Uses floating-point (slightly slower)
- **Midpoint Line:** Integer-only (fast!)
- **Modified Circle:** Integer + symmetry (fastest!)

All three are fast enough you won't notice the difference for normal drawing.

---

## 🔗 Next Steps

- Read **README.md** for detailed documentation
- Read **ALGORITHM_NOTES.md** for mathematical foundations
- Check **main.cpp** to understand the implementation
- Integrate with team members' code for full project

---

## 📞 Need Help?

Check these files in order:
1. **This file** - Quick answers
2. **README.md** - Complete documentation
3. **ALGORITHM_NOTES.md** - Algorithm details
4. **main.cpp** - Source code with comments

---

## ✅ Checklist

Before submitting to team, verify:

- [ ] Program compiles without errors
- [ ] DDA line draws in all directions
- [ ] Midpoint line draws in all directions
- [ ] Modified Midpoint Circle is perfectly round
- [ ] Color picker works
- [ ] Background changes to white
- [ ] All 3 cursor types work
- [ ] Clear screen removes all shapes
- [ ] Shapes persist after window resize
- [ ] Console shows helpful messages

---

**Enjoy drawing! 🎨**
