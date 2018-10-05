#pragma once
#include <windowsx.h>
#include "TupleStructs.h"
#include "Cursor.h"
#include "EngineUtil.h"

namespace InputEvent {
	enum Event {
		RBUTTONDOWN,
		LBUTTONDOWN,
		RBUTTONUP,
		LBUTTONUP,
		MOUSEMOVE,
		KEYDOWN,
		KEYUP
	};
}

class Input
{
public:
	void Initialize();

	static bool IsKeyDown(uint32_t);
	static Int32Pair GetMouseDelta();
	static Int32Pair GetMouseScreenSpacePosition();

private:
	friend class GameEngine;
	friend class WindowsApplication;

	Input();
	Input(const Input&);
	~Input();

	static Input* Get();
	void UpdateInput(const InputEvent::Event&, WPARAM, Int32Pair = { 0, 0 });
	void KeyDown(uint32_t);
	void KeyUp(uint32_t);
	void ResetMouseDelta();

	static Input* m_Instance; //must be defined in .cpp file as it is static
	Cursor m_Cursor;
	InputEvent::Event m_InputEvent;
	Int32Pair m_MouseDelta;
	Int32Pair m_MouseScreenSpacePosition;
	bool m_Keys[256];

};

