#include "stdafx.h"
#include "Input.h"
#include <assert.h>

Input* Input::m_Instance;

Input::Input()
{
	m_MouseDelta = { 0, 0 };
}

Input::Input(const Input& other)
{
}

Input::~Input()
{
}

void Input::Initialize()
{
	// Initialize all the keys to being released and not pressed.
	for (int i = 0; i<256; i++)
	{
		m_Keys[i] = false;
	}
}

Input* Input::Get()
{
	if (!m_Instance)
	{
		m_Instance = new Input();
	}
	return m_Instance;
}

void Input::UpdateInput(const InputEvent::Event& currentEvent, WPARAM wparam, Int32Pair screenSpaceMousePosition)
{
	static Int32Pair mouseStart;        // static means this persists through function calls, and it also forces initialization
	Int32Pair mousePosition = m_Cursor.GetPosition();
	
	m_InputEvent = currentEvent;
	ResetMouseDelta();

	switch (currentEvent)
	{
	case InputEvent::RBUTTONDOWN:
		{
			m_Cursor.SetVisible(false);
			mouseStart = mousePosition;
			break;
		}

	case InputEvent::RBUTTONUP:
		{
			m_Cursor.SetVisible(true);
			m_Cursor.SetPosition(mouseStart.x, mouseStart.y);
			break;
		}

		case InputEvent::MOUSEMOVE:
		{
			if (wparam & MK_RBUTTON)
			{
				int32_t deltaX = mousePosition.x - mouseStart.x;
				int32_t deltaY = mousePosition.y - mouseStart.y;
				m_MouseDelta = { deltaX, deltaY };
				m_Cursor.SetPosition(mouseStart.x, mouseStart.y);
			}
			break;
		}

		case InputEvent::KEYDOWN:
		{
			KeyDown((uint32_t)wparam);
			break;
		}

		case InputEvent::KEYUP:
		{
			KeyUp((uint32_t)wparam);
			break;
		}
	}
}

void Input::KeyDown(uint32_t input)
{
	m_Keys[input] = true;
}

void Input::KeyUp(uint32_t input)
{
	m_Keys[input] = false;
}

void Input::ResetMouseDelta()
{
	m_MouseDelta.x = 0;
	m_MouseDelta.y = 0;
}


/*
	Public Functions
*/
bool Input::IsKeyDown(uint32_t key)
{
	if (m_Instance) {
		return m_Instance->m_Keys[key];
	}
	else {
		DebugLog("s", "Tried to read input key with no instance of Input");
		assert(0);
	}
}

Int32Pair Input::GetMouseDelta()
{
	if (m_Instance) {
		return m_Instance->m_MouseDelta;
	}
	else {
		DebugLog("s", "Tried to read mouse delta with no instance of Input");
		assert(0);
	}
}

Int32Pair Input::GetMouseScreenSpacePosition()
{
	if (m_Instance) {
		return m_Instance->m_MouseScreenSpacePosition;
	}
	else {
		DebugLog("s", "Tried to read mouse screen space position with no instance of Input");
		assert(0);
	}
}

