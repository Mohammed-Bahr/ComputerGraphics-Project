/*
 * Computer Graphics Term Project - Person 3 Implementation
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

#define IDM_LINES_DDA                  201
#define IDM_LINES_MIDPOINT             202

#define IDM_CIRCLES_MODIFIED_MIDPOINT  301

#define IDM_FILE_CLEAR                 401

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
COLORREF g_backgroundColor = RGB(0, 0, 0);      // Default black background
COLORREF g_drawColor = RGB(255, 255, 255);      // Default white drawing color

enum DrawMode { 
    MODE_NONE, 
    MODE_DDA_LINE, 
    MODE_MIDPOINT_LINE, 
    MODE_MODIFIED_MIDPOINT_CIRCLE 
};

DrawMode g_currentMode = MODE_NONE;

// Mouse click state
bool g_firstClick = false;
int g_startX = 0, g_startY = 0;

// Structure to store drawn shapes for redrawing
struct Shape {
    DrawMode mode;
    COLORREF color;
    int x1, y1, x2, y2;
};

vector<Shape> g_shapes;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

int Round(double x) {
    return (int)(x + 0.5);
}

void swap(int& a, int& b, int& c, int& d) {
    int temp;
    temp = a; a = c; c = temp;
    temp = b; b = d; d = temp;
}

// ============================================================================
// DRAW 8 SYMMETRIC POINTS FOR CIRCLE
// ============================================================================
void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF color) {
    SetPixel(hdc, xc + x, yc + y, color);    // Quadrant I
    SetPixel(hdc, xc - x, yc + y, color);    // Quadrant II
    SetPixel(hdc, xc - x, yc - y, color);    // Quadrant III
    SetPixel(hdc, xc + x, yc - y, color);    // Quadrant IV
    SetPixel(hdc, xc + y, yc + x, color);    // Octant between I and II
    SetPixel(hdc, xc - y, yc + x, color);    // Octant between II and III
    SetPixel(hdc, xc - y, yc - x, color);    // Octant between III and IV
    SetPixel(hdc, xc + y, yc - x, color);    // Octant between IV and I
}

// ============================================================================
// ALGORITHM 1: DDA LINE
// Reference: LEC2.md - Digital Differential Analyzer
// ============================================================================
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

// ============================================================================
// ALGORITHM 2: MIDPOINT LINE (Bresenham's Line)
// Reference: LEC2.md - MidPoint Algorithm
// ============================================================================
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

// ============================================================================
// ALGORITHM 3: MODIFIED MIDPOINT CIRCLE (Faster Bresenham variant)
// Reference: Circle Drawing Algorithms.md - Section 5.6
// Uses 2nd order differences for maximum speed
// ============================================================================
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

// ============================================================================
// SHAPE RENDERING
// ============================================================================
void RenderShape(HDC hdc, const Shape& shape) {
    switch (shape.mode) {
        case MODE_DDA_LINE:
            DrawLineDDA(hdc, shape.x1, shape.y1, shape.x2, shape.y2, shape.color);
            break;
        case MODE_MIDPOINT_LINE:
            DrawLineMidpoint(hdc, shape.x1, shape.y1, shape.x2, shape.y2, shape.color);
            break;
        case MODE_MODIFIED_MIDPOINT_CIRCLE: {
            int R = (int)sqrt((shape.x2 - shape.x1) * (shape.x2 - shape.x1) + 
                             (shape.y2 - shape.y1) * (shape.y2 - shape.y1));
            DrawCircleModifiedMidpoint(hdc, shape.x1, shape.y1, R, shape.color);
            break;
        }
    }
}

void RedrawAllShapes(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    
    // Clear background
    RECT rect;
    GetClientRect(hwnd, &rect);
    HBRUSH brush = CreateSolidBrush(g_backgroundColor);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
    
    // Redraw all shapes
    for (const auto& shape : g_shapes) {
        RenderShape(hdc, shape);
    }
    
    ReleaseDC(hwnd, hdc);
}

// ============================================================================
// CHOOSE COLOR DIALOG
// ============================================================================
void ChooseDrawingColor(HWND hwnd) {
    CHOOSECOLOR cc;
    static COLORREF customColors[16];
    
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.lpCustColors = customColors;
    cc.rgbResult = g_drawColor;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    
    if (ChooseColor(&cc)) {
        g_drawColor = cc.rgbResult;
        cout << "Drawing color changed to RGB(" 
             << (int)GetRValue(g_drawColor) << ", "
             << (int)GetGValue(g_drawColor) << ", "
             << (int)GetBValue(g_drawColor) << ")" << endl;
    }
}

// ============================================================================
// MENU CREATION
// ============================================================================
HMENU CreateMainMenu() {
    HMENU hMenuBar = CreateMenu();
    
    // File Menu
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_CLEAR, L"Clear Screen");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
    
    // Preferences Menu
    HMENU hPrefMenu = CreatePopupMenu();
    AppendMenu(hPrefMenu, MF_STRING, IDM_PREFERENCES_BG_WHITE, L"Background: White");
    AppendMenu(hPrefMenu, MF_STRING, IDM_PREFERENCES_CHOOSE_COLOR, L"Choose Drawing Color...");
    AppendMenu(hPrefMenu, MF_SEPARATOR, 0, NULL);
    
    HMENU hCursorMenu = CreatePopupMenu();
    AppendMenu(hCursorMenu, MF_STRING, IDM_PREFERENCES_CURSOR_ARROW, L"Arrow");
    AppendMenu(hCursorMenu, MF_STRING, IDM_PREFERENCES_CURSOR_CROSS, L"Crosshair");
    AppendMenu(hCursorMenu, MF_STRING, IDM_PREFERENCES_CURSOR_HAND, L"Hand");
    AppendMenu(hPrefMenu, MF_POPUP, (UINT_PTR)hCursorMenu, L"Mouse Cursor");
    
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hPrefMenu, L"Preferences");
    
    // Lines Menu
    HMENU hLinesMenu = CreatePopupMenu();
    AppendMenu(hLinesMenu, MF_STRING, IDM_LINES_DDA, L"DDA");
    AppendMenu(hLinesMenu, MF_STRING, IDM_LINES_MIDPOINT, L"Midpoint (Bresenham)");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hLinesMenu, L"Lines");
    
    // Circles Menu
    HMENU hCirclesMenu = CreatePopupMenu();
    AppendMenu(hCirclesMenu, MF_STRING, IDM_CIRCLES_MODIFIED_MIDPOINT, L"Modified Midpoint");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hCirclesMenu, L"Circles");
    
    return hMenuBar;
}

// ============================================================================
// WINDOW PROCEDURE
// ============================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HCURSOR hCurrentCursor = LoadCursor(NULL, IDC_ARROW);
    
    switch (msg) {
        case WM_CREATE: {
            // Attach menu to window
            SetMenu(hwnd, CreateMainMenu());
            
            // Open console for messages
            AllocConsole();
            FILE* fp;
            freopen_s(&fp, "CONOUT$", "w", stdout);
            
            cout << "=== Computer Graphics Project - Person 3 ===" << endl;
            cout << "Select a drawing mode from the menu, then use the mouse to draw." << endl;
            cout << endl;
            
            break;
        }
        
        case WM_COMMAND: {
            int menuId = LOWORD(wParam);
            
            switch (menuId) {
                // File Menu
                case IDM_FILE_CLEAR:
                    g_shapes.clear();
                    g_firstClick = false;
                    InvalidateRect(hwnd, NULL, TRUE);
                    cout << "Screen cleared." << endl;
                    break;
                
                // Preferences - Background
                case IDM_PREFERENCES_BG_WHITE:
                    g_backgroundColor = RGB(255, 255, 255);
                    InvalidateRect(hwnd, NULL, TRUE);
                    cout << "Background color changed to White." << endl;
                    break;
                
                // Preferences - Drawing Color
                case IDM_PREFERENCES_CHOOSE_COLOR:
                    ChooseDrawingColor(hwnd);
                    break;
                
                // Preferences - Cursor
                case IDM_PREFERENCES_CURSOR_ARROW:
                    hCurrentCursor = LoadCursor(NULL, IDC_ARROW);
                    SetCursor(hCurrentCursor);
                    cout << "Mouse cursor changed to Arrow." << endl;
                    break;
                case IDM_PREFERENCES_CURSOR_CROSS:
                    hCurrentCursor = LoadCursor(NULL, IDC_CROSS);
                    SetCursor(hCurrentCursor);
                    cout << "Mouse cursor changed to Crosshair." << endl;
                    break;
                case IDM_PREFERENCES_CURSOR_HAND:
                    hCurrentCursor = LoadCursor(NULL, IDC_HAND);
                    SetCursor(hCurrentCursor);
                    cout << "Mouse cursor changed to Hand." << endl;
                    break;
                
                // Lines Menu
                case IDM_LINES_DDA:
                    g_currentMode = MODE_DDA_LINE;
                    g_firstClick = false;
                    cout << "\n[DDA Line Mode] Click two points to draw a line." << endl;
                    break;
                case IDM_LINES_MIDPOINT:
                    g_currentMode = MODE_MIDPOINT_LINE;
                    g_firstClick = false;
                    cout << "\n[Midpoint Line Mode] Click two points to draw a line." << endl;
                    break;
                
                // Circles Menu
                case IDM_CIRCLES_MODIFIED_MIDPOINT:
                    g_currentMode = MODE_MODIFIED_MIDPOINT_CIRCLE;
                    g_firstClick = false;
                    cout << "\n[Modified Midpoint Circle Mode] Click center, then a point on the radius." << endl;
                    break;
            }
            break;
        }
        
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            if (g_currentMode == MODE_NONE) {
                cout << "Please select a drawing mode from the menu first." << endl;
                break;
            }
            
            if (!g_firstClick) {
                // First click: store starting point
                g_startX = x;
                g_startY = y;
                g_firstClick = true;
                cout << "  First point: (" << x << ", " << y << ")" << endl;
            } else {
                // Second click: draw the shape
                cout << "  Second point: (" << x << ", " << y << ")" << endl;
                
                HDC hdc = GetDC(hwnd);
                
                switch (g_currentMode) {
                    case MODE_DDA_LINE:
                        DrawLineDDA(hdc, g_startX, g_startY, x, y, g_drawColor);
                        cout << "  -> DDA Line drawn from (" << g_startX << ", " << g_startY 
                             << ") to (" << x << ", " << y << ")" << endl;
                        break;
                    case MODE_MIDPOINT_LINE:
                        DrawLineMidpoint(hdc, g_startX, g_startY, x, y, g_drawColor);
                        cout << "  -> Midpoint Line drawn from (" << g_startX << ", " << g_startY 
                             << ") to (" << x << ", " << y << ")" << endl;
                        break;
                    case MODE_MODIFIED_MIDPOINT_CIRCLE: {
                        int R = (int)sqrt((x - g_startX) * (x - g_startX) + 
                                         (y - g_startY) * (y - g_startY));
                        DrawCircleModifiedMidpoint(hdc, g_startX, g_startY, R, g_drawColor);
                        cout << "  -> Modified Midpoint Circle drawn with center (" 
                             << g_startX << ", " << g_startY << ") and radius " << R << endl;
                        break;
                    }
                }
                
                ReleaseDC(hwnd, hdc);
                
                // Store the shape for redrawing
                Shape newShape;
                newShape.mode = g_currentMode;
                newShape.color = g_drawColor;
                newShape.x1 = g_startX;
                newShape.y1 = g_startY;
                newShape.x2 = x;
                newShape.y2 = y;
                g_shapes.push_back(newShape);
                
                // Reset for next shape
                g_firstClick = false;
            }
            break;
        }
        
        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT) {
                SetCursor(hCurrentCursor);
                return TRUE;
            }
            break;
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Fill background
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH brush = CreateSolidBrush(g_backgroundColor);
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
            
            // Redraw all shapes
            for (const auto& shape : g_shapes) {
                RenderShape(hdc, shape);
            }
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    return 0;
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"CGProject_Person3";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error", MB_ICONERROR);
        return 0;
    }
    
    // Create window
    HWND hwnd = CreateWindow(
        L"CGProject_Person3",
        L"Computer Graphics Project - Person 3",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hwnd) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_ICONERROR);
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
