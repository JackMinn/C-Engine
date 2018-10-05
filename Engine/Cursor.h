#pragma once
#include "TupleStructs.h"
#include "EngineUtil.h"

class Cursor
{
public:
	Cursor();
	~Cursor();

	inline Int32Pair GetPosition() const
	{
		POINT CursorPos;
		::GetCursorPos(&CursorPos);

		return { CursorPos.x, CursorPos.y };
	}

	inline void SetPosition(const int32_t& x, const int32_t& y)
	{
		::SetCursorPos(x, y);
	}

	inline void SetVisible(bool visible)
	{
		if (visible)
		{
			while (::ShowCursor(true) < 0);
		}
		else
		{		
			while (::ShowCursor(false) >= 0);
		}
	}

	inline void LockInPosition()
	{
		Int32Pair pos = GetPosition();
		RECT bounds = { pos.x, pos.y, pos.x, pos.y };
		::ClipCursor(&bounds);
	}

	inline bool GetCursorBounds(RECT* const pRect)
	{
		return ::GetClipCursor(pRect);
	}

	inline bool SetCursorBounds(const RECT* const pRect)
	{
		return ::ClipCursor(pRect);
	}
};

