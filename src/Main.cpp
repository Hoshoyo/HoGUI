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

#define CLEAR_COLOR 0.4f, 0.4f, 0.4f, 1.0f
#define INITIAL_WIN_WIDTH 800
#define INITIAL_WIN_HEIGHT 600

struct WindowState
{
	HWND window_handle;
	LONG win_width, win_height;
	WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };
};

WindowState win_state = {};

void ToggleFullScreen(WindowState* win_state)
{
	DWORD dwStyle = GetWindowLong(win_state->window_handle, GWL_STYLE);
	if (dwStyle & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(win_state->window_handle, &win_state->g_wpPrev) &&
			GetMonitorInfo(MonitorFromWindow(win_state->window_handle, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(win_state->window_handle, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(win_state->window_handle, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(win_state->window_handle, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(win_state->window_handle, &win_state->g_wpPrev);
		SetWindowPos(win_state->window_handle, NULL, 0, 0, 0, 0,
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
		case 'F': ToggleFullScreen(&win_state); break;
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
		win_state.win_width = r.right - r.left;
		win_state.win_height = r.bottom - r.top;
		glViewport(0, 0, win_state.win_width, win_state.win_height);
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

	glClearColor(CLEAR_COLOR);

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

	win_state.win_width = INITIAL_WIN_WIDTH;
	win_state.win_height = INITIAL_WIN_HEIGHT;

	win_state.window_handle = CreateWindowEx(
		NULL,
		window_class.lpszClassName,
		"HoGraphics",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		win_state.win_width, win_state.win_height, NULL, NULL, hInstance, NULL
	);

	if (!win_state.window_handle)
		OutputDebugStringA("Error criating window context.\n");

	ShowWindow(win_state.window_handle, nCmdShow);
	UpdateWindow(win_state.window_handle);

	HDC device_context;
	HGLRC rendering_context;
	InitOpenGL(win_state.window_handle, device_context, rendering_context);

	wglSwapIntervalEXT(1);		// Enable Vsync

	bool running = true;
	MSG msg;

	// Track mouse events
	TRACKMOUSEEVENT mouse_event = {};
	mouse_event.cbSize = sizeof(mouse_event);
	mouse_event.dwFlags = TME_LEAVE;
	mouse_event.dwHoverTime = HOVER_DEFAULT;
	mouse_event.hwndTrack = win_state.window_handle;

	/*
	*	NOTE: Temporary init code
	*/
	FontShader shader = {};
	shader.id = LoadShader("res/shaders/normal.vs", "res/shaders/normal.fs");
	shader.uni_color = glGetUniformLocation(shader.id, "font_color");

	FontInfo font_info;
	LoadFreetype("res/LiberationMono-Regular.ttf", 96, &font_info);

	glm::vec4 font_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

	// -

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

		// NOTE: temp render
		SetOrthoProjection(&shader.projection_matrix, (float)win_state.win_width, (float)win_state.win_height);
		RenderText(&font_info, &shader, "Xãoñ Zé", 1920/2.0f, 1080/2.0f, 1, &font_color);
	
		SwapBuffers(device_context);
	}

#if _DEBUG
	fclose(pCout);
#endif
	return (int)msg.wParam;
}

