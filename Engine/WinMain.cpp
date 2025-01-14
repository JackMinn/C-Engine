#include "stdafx.h"
#include "WindowsApplication.h"
#include "GameEngine.h"

GameEngine g_GameEngine;

constexpr uint32_t g_LaunchWindowWidth = 1920;
constexpr uint32_t g_LaunchWindowHeight = 1080;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	bool result;
	WindowsApplication mainApplication;
	mainApplication.CreateApplication(hInstance);

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

	result = g_GameEngine.Initialize(g_LaunchWindowWidth, g_LaunchWindowHeight, mainApplication.m_WindowHandle);
	if (!result)
	{
		return 0;
	}

	mainApplication.ProcessMessagesLoop();

	// Shutdown and release the system object.
	g_GameEngine.Shutdown();
	mainApplication.ShutdownApplication();

	return 0;
}
