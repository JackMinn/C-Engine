#include "stdafx.h"
#include "EngineUtil.h"
#include "WindowsApplication.h"
#include "GameEngine.h"
#include "TupleStructs.h"
#include "Input.h"
#include "GraphicsClass.h"

#include "GlobalExterns.h"

WindowsApplication::WindowsApplication()
{
}

WindowsApplication::~WindowsApplication()
{
}

void WindowsApplication::CreateApplication(HINSTANCE const& instanceHandle)
{
	//m_InstanceHandle = GetModuleHandle(NULL);
	m_InstanceHandle = instanceHandle;
	RegisterWindow(instanceHandle, LoadIcon(NULL, IDI_WINLOGO), m_ApplicationName);

	// Determine the resolution of the clients desktop screen.
	DebugLog("sisi", "System says width: ", GetSystemMetrics(SM_CXSCREEN), " Height: ", GetSystemMetrics(SM_CYSCREEN));
	//uint32_t screenWidth = GetSystemMetrics(SM_CXSCREEN) / 2;
	//uint32_t screenHeight = GetSystemMetrics(SM_CYSCREEN) / 2;
	uint32_t screenWidth = GetSystemMetrics(SM_CXSCREEN);
	uint32_t screenHeight = GetSystemMetrics(SM_CYSCREEN);

	RECT windowRect = { 0, 0, static_cast<LONG>(screenWidth), static_cast<LONG>(screenHeight) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int32_t posX, posY;
	//posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	//posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth);
	posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight);

	m_WindowHandle = CreateWindowEx(NULL,
		m_ApplicationName,
		m_ApplicationName,
		WS_OVERLAPPEDWINDOW,
		posX,
		posY,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		m_InstanceHandle,
		NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_WindowHandle, SW_SHOW);
	SetForegroundWindow(m_WindowHandle);
	SetFocus(m_WindowHandle);
}

void WindowsApplication::ShutdownApplication()
{
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(m_WindowHandle);
	m_WindowHandle = NULL;

	UnregisterClass(m_ApplicationName, m_InstanceHandle);
	m_InstanceHandle = NULL;
}

void WindowsApplication::RegisterWindow(HINSTANCE const& instanceHandle, HICON const& icon, LPCWSTR windowClassName)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;// | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instanceHandle;
	wc.hIcon = icon;
	//wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClassName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);
}

void WindowsApplication::ProcessMessagesLoop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	bool done, result;

	done = false;
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			g_GameEngine.ProcessGameLoop();
			result = g_GameEngine.RenderFrame();
			if (!result)
			{
				done = true;
			}
		}
	}
}

LRESULT CALLBACK WindowsApplication::EngineMessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	POINT mouse;
	mouse.x = GET_X_LPARAM(lparam);
	mouse.y = GET_Y_LPARAM(lparam);

	Int32Pair temp = { mouse.x, mouse.y };

	switch (umsg)
	{
		case WM_RBUTTONDOWN:
		{
			SetCapture(hwnd);
			g_GameEngine.m_Input->UpdateInput(InputEvent::RBUTTONDOWN, wparam, temp);
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			g_GameEngine.m_Input->UpdateInput(InputEvent::MOUSEMOVE, wparam, temp);
			return 0;
		}

		case WM_RBUTTONUP:
		{
			g_GameEngine.m_Input->UpdateInput(InputEvent::RBUTTONUP, wparam, temp);
			ReleaseCapture();
			return 0;
		}

		case WM_KEYDOWN:
		{
			g_GameEngine.m_Input->KeyDown((uint32_t)wparam);
			return 0;
		}

		case WM_KEYUP:
		{
			g_GameEngine.m_Input->KeyUp((uint32_t)wparam);
			return 0;
		}

		default:
		{
			return DefWindowProc(hwnd, umsg, wparam, lparam);
		}
	}
}

LRESULT CALLBACK WindowsApplication::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		case WM_CREATE:
		{
			DebugLog("s", "Creating the window.");
			//SetWindowLongPtr(hwnd, 0, );
			return 0;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		default:
		{
			return EngineMessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}