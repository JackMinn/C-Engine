#pragma once

#include <windowsx.h>
#include <memory>
#include "EngineUtil.h"
#include "GameObjectFactory.h"

class GameObjectFactory;
class Input;
class GraphicsClass;

class GameEngine
{
public:
	GameEngine() : m_Graphics(nullptr), m_Input(nullptr), m_GameObjectFactory(std::unique_ptr<GameObjectFactory>()) {}
	GameEngine(const GameEngine&) = delete;
	~GameEngine() { DebugLog("s", "ENGINE DESTRUCTOR WAS CALLED"); }

	void operator=(const GameEngine&) = delete;

	bool Initialize(const uint32_t& screenWidth, const uint32_t& screenHeight, HWND const& windowHandle);
	void Shutdown();

private:
	friend class WindowsApplication;

	bool RenderFrame();
	void ProcessGameLoop();

	LPCWSTR m_ApplicationName;
	HINSTANCE m_InstanceHandle;
	HWND m_WindowHandle;
	Input* m_Input;
	GraphicsClass* m_Graphics;
	std::unique_ptr<GameObjectFactory> m_GameObjectFactory;
};

