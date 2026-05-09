#include <windows.h>
#include <assert.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
void AdjustPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	int pf = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pf, &pfd);
}
static LRESULT CALLBACK WndProc(HWND hwnd, UINT m, WPARAM wp, LPARAM lp)
{
	static HDC hdc;
	static HGLRC  hglrc;
	switch (m)
	{
	case WM_CREATE:
		hdc = GetDC(hwnd);
		AdjustPixelFormat(hdc);
		hglrc = wglCreateContext(hdc);
		wglMakeCurrent(hdc, hglrc);
		break;
	case WM_SIZE:
		glViewport(0, 0, LOWORD(lp), HIWORD(lp));
		break;
	case WM_LBUTTONDOWN:
		glClearColor(0.0F, 0.0F, 0.4F, 1.0F);
		glShadeModel(GL_SMOOTH);
		glClear(GL_COLOR_BUFFER_BIT);
		glBegin(GL_TRIANGLES);
		glColor3f(1.0F, 0.0F, 0.0F);
		glVertex3f(0.0F, 0.0F, 0.0F);
		glColor3f(0.0F, 1.0F, 0.0F);
		glVertex3f(1.0F, 0.5F, 0.0F);
		glColor3f(0.0F, 0.0F, 1.0F);
		glVertex3f(-1.0F, 1.F, 0.0F);
		glEnd();
		SwapBuffers(hdc);
		break;
	case WM_DESTROY:
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hglrc);
		ReleaseDC(hwnd, hdc);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, m, wp, lp);
	}
	return 0;
}


int APIENTRY WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"GLEX";
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wc);
	HWND hwnd = CreateWindow(L"GLEX", L"OpenGL Demo", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
		NULL, hInstance, 0);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
