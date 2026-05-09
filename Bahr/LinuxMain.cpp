/*
 * Computer Graphics Term Project - Person 3 Implementation
 * Linux/GNOME Port using GTK3 + Cairo
 *
 * Responsibilities:
 * - Change background color (Preferences menu)
 * - Choose shape drawing color (Preferences menu)
 * - DDA Line algorithm
 * - Midpoint Line algorithm (Bresenham's)
 * - Modified Midpoint Circle algorithm (Faster Bresenham variant)
 * - Change mouse pointer (Preferences menu)
 *
 * Build:
 *   g++ LinuxMain.cpp -o cg_project $(pkg-config --cflags --libs gtk+-3.0) -lm
 *
 * All drawing uses mouse-only input (no keyboard)
 */

#include <gtk/gtk.h>
#include <cmath>
#include <vector>
#include <iostream>

using namespace std;

// ============================================================================
// COLOR STRUCTURE  (replaces Windows COLORREF; Cairo uses 0.0-1.0 range)
// ============================================================================
struct Color {
    double r, g, b;
};

// ============================================================================
// DRAW MODE
// ============================================================================
enum DrawMode {
    MODE_NONE,
    MODE_DDA_LINE,
    MODE_MIDPOINT_LINE,
    MODE_MODIFIED_MIDPOINT_CIRCLE
};

// ============================================================================
// SHAPE STRUCTURE  (stores each drawn shape for redraws)
// ============================================================================
struct Shape {
    DrawMode mode;
    Color    color;
    int      x1, y1, x2, y2;
};

// ============================================================================
// GLOBAL STATE
// ============================================================================
Color    g_backgroundColor = {0.0, 0.0, 0.0};   // Default: black
Color    g_drawColor       = {1.0, 1.0, 1.0};   // Default: white
DrawMode g_currentMode     = MODE_NONE;

bool g_firstClick = false;
int  g_startX = 0, g_startY = 0;

vector<Shape> g_shapes;

GtkWidget* g_drawingArea = nullptr;   // Global reference for queue_draw calls
GtkWidget* g_mainWindow  = nullptr;   // Global reference for dialog parenting

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static inline int Round(double x) {
    return (int)(x + 0.5);
}

static void swap_points(int& a, int& b, int& c, int& d) {
    int t;
    t = a; a = c; c = t;
    t = b; b = d; d = t;
}

// Set a single pixel using Cairo (1×1 filled rectangle)
static void SetPixel(cairo_t* cr, int x, int y, Color color) {
    cairo_set_source_rgb(cr, color.r, color.g, color.b);
    cairo_rectangle(cr, x, y, 1.0, 1.0);
    cairo_fill(cr);
}

// ============================================================================
// DRAW 8 SYMMETRIC POINTS FOR CIRCLE
// ============================================================================
static void Draw8Points(cairo_t* cr, int xc, int yc, int x, int y, Color color) {
    SetPixel(cr, xc + x, yc + y, color);   // Quadrant I
    SetPixel(cr, xc - x, yc + y, color);   // Quadrant II
    SetPixel(cr, xc - x, yc - y, color);   // Quadrant III
    SetPixel(cr, xc + x, yc - y, color);   // Quadrant IV
    SetPixel(cr, xc + y, yc + x, color);   // Octant I-II
    SetPixel(cr, xc - y, yc + x, color);   // Octant II-III
    SetPixel(cr, xc - y, yc - x, color);   // Octant III-IV
    SetPixel(cr, xc + y, yc - x, color);   // Octant IV-I
}

// ============================================================================
// ALGORITHM 1: DDA LINE
// ============================================================================
static void DrawLineDDA(cairo_t* cr,
                        int x1, int y1, int x2, int y2, Color c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (abs(dy) <= abs(dx)) {
        // Gentle slope – step along X
        double m = (dx != 0) ? (double)dy / dx : 0.0;
        if (x1 > x2) swap_points(x1, y1, x2, y2);
        SetPixel(cr, x1, y1, c);
        int    x = x1;
        double y = y1;
        while (x < x2) {
            x++;
            y += m;
            SetPixel(cr, x, Round(y), c);
        }
    } else {
        // Steep slope – step along Y
        double mi = (dy != 0) ? (double)dx / dy : 0.0;
        if (y1 > y2) swap_points(x1, y1, x2, y2);
        SetPixel(cr, x1, y1, c);
        int    y = y1;
        double x = x1;
        while (y < y2) {
            y++;
            x += mi;
            SetPixel(cr, Round(x), y, c);
        }
    }
}

// ============================================================================
// ALGORITHM 2: MIDPOINT LINE  (Bresenham's Line)
// ============================================================================
static void DrawLineMidpoint(cairo_t* cr,
                             int x1, int y1, int x2, int y2, Color c) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int x  = x1, y = y1;

    if (dx >= dy) {
        // Gentle slope (|m| <= 1)
        int d       = dx - 2 * dy;
        int delta1  = 2 * dx - 2 * dy;
        int delta2  = -2 * dy;
        SetPixel(cr, x, y, c);
        while (x != x2) {
            if (d < 0) { y += sy; d += delta1; }
            else         {          d += delta2; }
            x += sx;
            SetPixel(cr, x, y, c);
        }
    } else {
        // Steep slope (|m| > 1)
        int d       = dy - 2 * dx;
        int delta1  = 2 * dy - 2 * dx;
        int delta2  = -2 * dx;
        SetPixel(cr, x, y, c);
        while (y != y2) {
            if (d < 0) { x += sx; d += delta1; }
            else         {          d += delta2; }
            y += sy;
            SetPixel(cr, x, y, c);
        }
    }
}

// ============================================================================
// ALGORITHM 3: MODIFIED MIDPOINT CIRCLE  (Faster Bresenham variant)
// ============================================================================
static void DrawCircleModifiedMidpoint(cairo_t* cr,
                                       int xc, int yc, int R, Color color) {
    if (R <= 0) return;
    int x  = 0, y = R;
    int d  = 1 - R;
    int c1 = 3;
    int c2 = 5 - 2 * R;

    Draw8Points(cr, xc, yc, x, y, color);

    while (x < y) {
        if (d < 0) {
            d  += c1;
            c2 += 2;
        } else {
            d  += c2;
            c2 += 4;
            y--;
        }
        c1 += 2;
        x++;
        Draw8Points(cr, xc, yc, x, y, color);
    }
}

// ============================================================================
// SHAPE RENDERING
// ============================================================================
static void RenderShape(cairo_t* cr, const Shape& shape) {
    switch (shape.mode) {
        case MODE_DDA_LINE:
            DrawLineDDA(cr, shape.x1, shape.y1,
                            shape.x2, shape.y2, shape.color);
            break;
        case MODE_MIDPOINT_LINE:
            DrawLineMidpoint(cr, shape.x1, shape.y1,
                                 shape.x2, shape.y2, shape.color);
            break;
        case MODE_MODIFIED_MIDPOINT_CIRCLE: {
            int dx = shape.x2 - shape.x1;
            int dy = shape.y2 - shape.y1;
            int R  = (int)sqrt(dx * dx + dy * dy);
            DrawCircleModifiedMidpoint(cr, shape.x1, shape.y1, R, shape.color);
            break;
        }
        default:
            break;
    }
}

// ============================================================================
// GTK SIGNAL CALLBACKS
// ============================================================================

// "draw" signal – replaces WM_PAINT
static gboolean on_draw(GtkWidget* /*widget*/, cairo_t* cr, gpointer /*data*/) {
    // Fill background
    cairo_set_source_rgb(cr, g_backgroundColor.r,
                             g_backgroundColor.g,
                             g_backgroundColor.b);
    cairo_paint(cr);

    // Redraw all stored shapes
    for (const auto& shape : g_shapes)
        RenderShape(cr, shape);

    return FALSE;
}

// "button-press-event" – replaces WM_LBUTTONDOWN
static gboolean on_button_press(GtkWidget* widget,
                                GdkEventButton* event, gpointer /*data*/) {
    if (event->button != GDK_BUTTON_PRIMARY) return FALSE;  // left-click only

    int x = (int)event->x;
    int y = (int)event->y;

    if (g_currentMode == MODE_NONE) {
        cout << "Please select a drawing mode from the menu first." << endl;
        return TRUE;
    }

    if (!g_firstClick) {
        g_startX    = x;
        g_startY    = y;
        g_firstClick = true;
        cout << "  First point: (" << x << ", " << y << ")" << endl;
    } else {
        cout << "  Second point: (" << x << ", " << y << ")" << endl;

        Shape s;
        s.mode  = g_currentMode;
        s.color = g_drawColor;
        s.x1    = g_startX;
        s.y1    = g_startY;
        s.x2    = x;
        s.y2    = y;
        g_shapes.push_back(s);

        switch (g_currentMode) {
            case MODE_DDA_LINE:
                cout << "  -> DDA Line drawn from ("
                     << g_startX << ", " << g_startY << ") to ("
                     << x << ", " << y << ")" << endl;
                break;
            case MODE_MIDPOINT_LINE:
                cout << "  -> Midpoint Line drawn from ("
                     << g_startX << ", " << g_startY << ") to ("
                     << x << ", " << y << ")" << endl;
                break;
            case MODE_MODIFIED_MIDPOINT_CIRCLE: {
                int dx = x - g_startX, dy = y - g_startY;
                int R  = (int)sqrt(dx * dx + dy * dy);
                cout << "  -> Modified Midpoint Circle: center ("
                     << g_startX << ", " << g_startY
                     << "), radius " << R << endl;
                break;
            }
            default:
                break;
        }

        g_firstClick = false;
        gtk_widget_queue_draw(widget);   // trigger redraw
    }

    return TRUE;
}

// ---- File Menu ----
static void on_file_clear(GtkMenuItem* /*item*/, gpointer /*data*/) {
    g_shapes.clear();
    g_firstClick = false;
    gtk_widget_queue_draw(g_drawingArea);
    cout << "Screen cleared." << endl;
}

// ---- Preferences – Background ----
static void on_bg_black(GtkMenuItem* /*item*/, gpointer /*data*/) {
    g_backgroundColor = {0.0, 0.0, 0.0};
    gtk_widget_queue_draw(g_drawingArea);
    cout << "Background color changed to Black." << endl;
}

static void on_bg_white(GtkMenuItem* /*item*/, gpointer /*data*/) {
    g_backgroundColor = {1.0, 1.0, 1.0};
    gtk_widget_queue_draw(g_drawingArea);
    cout << "Background color changed to White." << endl;
}

// ---- Preferences – Drawing Color ----
static void on_choose_color(GtkMenuItem* /*item*/, gpointer /*data*/) {
    GtkWidget* dialog = gtk_color_chooser_dialog_new(
        "Choose Drawing Color", GTK_WINDOW(g_mainWindow));

    GdkRGBA rgba = { g_drawColor.r, g_drawColor.g, g_drawColor.b, 1.0 };
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &rgba);
        g_drawColor = { rgba.red, rgba.green, rgba.blue };
        cout << "Drawing color changed to RGB("
             << (int)(rgba.red   * 255) << ", "
             << (int)(rgba.green * 255) << ", "
             << (int)(rgba.blue  * 255) << ")" << endl;
    }
    gtk_widget_destroy(dialog);
}

// ---- Preferences – Cursor ----
static void set_cursor(GdkCursorType type, const char* name) {
    GdkWindow*  win    = gtk_widget_get_window(g_drawingArea);
    GdkDisplay* disp   = gdk_display_get_default();
    GdkCursor*  cursor = gdk_cursor_new_for_display(disp, type);
    gdk_window_set_cursor(win, cursor);
    g_object_unref(cursor);
    cout << "Mouse cursor changed to " << name << "." << endl;
}

static void on_cursor_arrow(GtkMenuItem*, gpointer) { set_cursor(GDK_LEFT_PTR,  "Arrow");     }
static void on_cursor_cross(GtkMenuItem*, gpointer) { set_cursor(GDK_CROSSHAIR, "Crosshair"); }
static void on_cursor_hand (GtkMenuItem*, gpointer) { set_cursor(GDK_HAND2,     "Hand");      }

// ---- Lines Menu ----
static void on_lines_dda(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_DDA_LINE;
    g_firstClick  = false;
    cout << "\n[DDA Line Mode] Click two points to draw a line." << endl;
}
static void on_lines_midpoint(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_MIDPOINT_LINE;
    g_firstClick  = false;
    cout << "\n[Midpoint Line Mode] Click two points to draw a line." << endl;
}

// ---- Circles Menu ----
static void on_circles_modified_midpoint(GtkMenuItem*, gpointer) {
    g_currentMode = MODE_MODIFIED_MIDPOINT_CIRCLE;
    g_firstClick  = false;
    cout << "\n[Modified Midpoint Circle Mode] Click center, then a radius point." << endl;
}

// ============================================================================
// MENU BUILDER
// ============================================================================
static GtkWidget* build_menu_bar() {
    GtkWidget* bar = gtk_menu_bar_new();

    // ── File ────────────────────────────────────────────
    GtkWidget* file_menu = gtk_menu_new();
    GtkWidget* file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

        GtkWidget* clear_item = gtk_menu_item_new_with_label("Clear Screen");
        g_signal_connect(clear_item, "activate", G_CALLBACK(on_file_clear), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), clear_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), file_item);

    // ── Preferences ─────────────────────────────────────
    GtkWidget* pref_menu = gtk_menu_new();
    GtkWidget* pref_item = gtk_menu_item_new_with_label("Preferences");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pref_item), pref_menu);

        GtkWidget* bg_black_item = gtk_menu_item_new_with_label("Background: Black");
        g_signal_connect(bg_black_item, "activate", G_CALLBACK(on_bg_black), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu), bg_black_item);

        GtkWidget* bg_white_item = gtk_menu_item_new_with_label("Background: White");
        g_signal_connect(bg_white_item, "activate", G_CALLBACK(on_bg_white), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu), bg_white_item);

        GtkWidget* color_item = gtk_menu_item_new_with_label("Choose Drawing Color...");
        g_signal_connect(color_item, "activate", G_CALLBACK(on_choose_color), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu), color_item);

        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu),
                              gtk_separator_menu_item_new());

        // Cursor sub-menu
        GtkWidget* cursor_menu = gtk_menu_new();
        GtkWidget* cursor_item = gtk_menu_item_new_with_label("Mouse Cursor");
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(cursor_item), cursor_menu);

            GtkWidget* arrow_item = gtk_menu_item_new_with_label("Arrow");
            g_signal_connect(arrow_item, "activate", G_CALLBACK(on_cursor_arrow), nullptr);
            gtk_menu_shell_append(GTK_MENU_SHELL(cursor_menu), arrow_item);

            GtkWidget* cross_item = gtk_menu_item_new_with_label("Crosshair");
            g_signal_connect(cross_item, "activate", G_CALLBACK(on_cursor_cross), nullptr);
            gtk_menu_shell_append(GTK_MENU_SHELL(cursor_menu), cross_item);

            GtkWidget* hand_item = gtk_menu_item_new_with_label("Hand");
            g_signal_connect(hand_item, "activate", G_CALLBACK(on_cursor_hand), nullptr);
            gtk_menu_shell_append(GTK_MENU_SHELL(cursor_menu), hand_item);

        gtk_menu_shell_append(GTK_MENU_SHELL(pref_menu), cursor_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), pref_item);

    // ── Lines ────────────────────────────────────────────
    GtkWidget* lines_menu = gtk_menu_new();
    GtkWidget* lines_item = gtk_menu_item_new_with_label("Lines");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(lines_item), lines_menu);

        GtkWidget* dda_item = gtk_menu_item_new_with_label("DDA");
        g_signal_connect(dda_item, "activate", G_CALLBACK(on_lines_dda), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(lines_menu), dda_item);

        GtkWidget* mp_item = gtk_menu_item_new_with_label("Midpoint (Bresenham)");
        g_signal_connect(mp_item, "activate", G_CALLBACK(on_lines_midpoint), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(lines_menu), mp_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), lines_item);

    // ── Circles ──────────────────────────────────────────
    GtkWidget* circles_menu = gtk_menu_new();
    GtkWidget* circles_item = gtk_menu_item_new_with_label("Circles");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(circles_item), circles_menu);

        GtkWidget* mm_item = gtk_menu_item_new_with_label("Modified Midpoint");
        g_signal_connect(mm_item, "activate",
                         G_CALLBACK(on_circles_modified_midpoint), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(circles_menu), mm_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(bar), circles_item);

    return bar;
}

// ============================================================================
// MAIN
// ============================================================================
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    // ── Window ───────────────────────────────────────────
    g_mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(g_mainWindow),
                         "Computer Graphics Project - Person 3");
    gtk_window_set_default_size(GTK_WINDOW(g_mainWindow), 800, 600);
    g_signal_connect(g_mainWindow, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    // ── Layout ───────────────────────────────────────────
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(g_mainWindow), vbox);

    // Menu bar
    GtkWidget* menu_bar = build_menu_bar();
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    // Drawing area
    g_drawingArea = gtk_drawing_area_new();
    gtk_widget_set_hexpand(g_drawingArea, TRUE);
    gtk_widget_set_vexpand(g_drawingArea, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), g_drawingArea, TRUE, TRUE, 0);

    // Enable mouse click events on the drawing area
    gtk_widget_add_events(g_drawingArea, GDK_BUTTON_PRESS_MASK);

    // Connect drawing signals
    g_signal_connect(g_drawingArea, "draw",
                     G_CALLBACK(on_draw), nullptr);
    g_signal_connect(g_drawingArea, "button-press-event",
                     G_CALLBACK(on_button_press), nullptr);

    // ── Start ────────────────────────────────────────────
    cout << "=== Computer Graphics Project - Person 3 ===" << endl;
    cout << "Select a drawing mode from the menu, then click two points." << endl;
    cout << endl;

    gtk_widget_show_all(g_mainWindow);
    gtk_main();

    return 0;
}
