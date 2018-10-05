#pragma once

#include <stdint.h>

class WindowsApplication
{
public:
	WindowsApplication();
	~WindowsApplication();

	void CreateApplication(HINSTANCE const& instanceHandle);
	void ShutdownApplication();
	void ProcessMessagesLoop();

	HWND m_WindowHandle;

private:
	void RegisterWindow(HINSTANCE const& instanceHandle, HICON const& icon, LPCWSTR windowClassName);

	static LRESULT CALLBACK EngineMessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

	LPCWSTR m_ApplicationName = L"Jacks Engine";
	HINSTANCE m_InstanceHandle;
};

