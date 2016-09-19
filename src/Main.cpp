#include <windows.h>
#include <windowsx.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include "font_render.h"
#include "shader_loader.h"
#include "graphics/shader.h"

#ifdef _DEBUG
#include <iostream>
#endif

HWND window_handle;
WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

void ToggleFullScreen(HWND hwnd)
{
	DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	if (dwStyle & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(hwnd, &g_wpPrev) &&
			GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(hwnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(hwnd, &g_wpPrev);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE: {
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
	} break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		SetCapture(GetActiveWindow());
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case 'F': ToggleFullScreen(hWnd); break;
		case VK_ESCAPE: PostQuitMessage(0); break;
		}
		break;
	case WM_KEYUP:
		break;
	case WM_SYSKEYDOWN:
		break;
	case WM_SYSKEYUP:
		break;
	case WM_CHAR:
		break;
	case WM_SIZE: {
		RECT r;
		GetClientRect(hWnd, &r);
		glViewport(0, 0, r.right - r.left, r.bottom - r.top);
	} break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
void InitOpenGL(HWND window_handle, HDC& device_context, HGLRC& rendering_context)
{
	int PixelFormat;
	device_context = GetDC(window_handle);

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.cDepthBits = 32;
	pfd.cColorBits = 24;
	pfd.iPixelType = PFD_TYPE_RGBA;

	PixelFormat = ChoosePixelFormat(device_context, &pfd);
	if (!SetPixelFormat(device_context, PixelFormat, &pfd))
		OutputDebugStringA("Error creating context pixel descriptor.\n");

	rendering_context = wglCreateContext(device_context);
	wglMakeCurrent(device_context, rendering_context);

	glewExperimental = true;
	GLenum err = glewInit();
	if (err != GLEW_OK)
		OutputDebugStringA("Error initializing glew.\n");

	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{

#if _DEBUG
	AllocConsole();
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
#endif

	WNDCLASSEX window_class;
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = WndProc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = hInstance;
	window_class.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(NULL));
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = "HoGraphicClass";
	window_class.hIconSm = NULL;

	if (!RegisterClassEx(&window_class))
		OutputDebugStringA("Error creating window class.\n");

	int window_width = 800;
	int window_height = 600;

	window_handle = CreateWindowEx(
		NULL,
		window_class.lpszClassName,
		"HoGraphics",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		window_width, window_height, NULL, NULL, hInstance, NULL
	);

	if (!window_handle)
		OutputDebugStringA("Error criating window context.\n");

	ShowWindow(window_handle, nCmdShow);
	UpdateWindow(window_handle);

	HDC device_context;
	HGLRC rendering_context;
	InitOpenGL(window_handle, device_context, rendering_context);

	wglSwapIntervalEXT(1);		// Enable Vsync

	bool running = true;
	MSG msg;

	// Track mouse events
	TRACKMOUSEEVENT mouse_event = {};
	mouse_event.cbSize = sizeof(mouse_event);
	mouse_event.dwFlags = TME_LEAVE;
	mouse_event.dwHoverTime = HOVER_DEFAULT;
	mouse_event.hwndTrack = window_handle;

	// TEMP
	FontInfo font_info;
	LoadFreetype("res/LiberationMono-Regular.ttf", 96, &font_info);
	FontShader shader = {};
	shader.id = LoadShader("res/shaders/normal.vs", "res/shaders/normal.fs");
	SetOrthoProjection(&shader.projection_matrix, 800.0f, 600.0f);

	while (running)
	{
		TrackMouseEvent(&mouse_event);

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			if (msg.message == WM_QUIT)
				running = false;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render
		RenderText(&shader, "Xãoñ Zé", 0, 0, 1);
	
		SwapBuffers(device_context);
	}

#if _DEBUG
	fclose(pCout);
#endif
	return (int)msg.wParam;
}

