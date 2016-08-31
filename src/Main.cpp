#include <windows.h>
#include <windowsx.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include "font_render.h"

#ifdef _DEBUG
#include <iostream>
#endif

bool is_down[512] = {};

HWND window_handle;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto getLowerCase = [](int c) -> int
	{
		if (c > 0x40 && c < 0x5A)	// A - Z
			c += 32;
		return c;
	};
	auto isExtendedKey = [](int key) -> bool
	{
		// if 24th bit is 1 then the key is extended
		// Alt or ctrl key
		if ((key & 0x1000000) != 0)
			return true;
		return false;
	};

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOUSELEAVE:
		//std::cout << "mouse left\n";
		break;
	case WM_MOUSEMOVE:
		std::cout << LOWORD(lParam) << " " << HIWORD(lParam) << "\n";
		break;
	case WM_LBUTTONDOWN:
		SetCapture(window_handle);
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_RBUTTONUP:
		break;
	case WM_KEYDOWN:
		is_down[wParam] = true;
		std::cout << (char)wParam;
		break;
	case WM_KEYUP:
		is_down[wParam] = false;
		break;
	case WM_MOUSEWHEEL:
		break;
	case WM_CHAR:
		break;
	case WM_SIZE:
		std::cout << "deu resize\n";
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
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

	FontRenderer* fr = new FontRenderer(512, 60, glm::vec4(1, 1, 1, 1));

	while (running)
	{
		TrackMouseEvent(&mouse_event);

		// input

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			if (msg.message == WM_QUIT)
				running = false;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render
	
		char text[] = "Hello World!";									 // TEMP
		fr->RenderText(text, sizeof(text), 0, 0, 200, true);			 // TEMP

		SwapBuffers(device_context);
	}

#if _DEBUG
	fclose(pCout);
#endif
	return (int)msg.wParam;
}

