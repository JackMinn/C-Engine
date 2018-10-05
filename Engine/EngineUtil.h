#pragma once

// System includes
#include <windows.h>
#include <iostream>
#include <sstream> 
#include <string>
#include <stdarg.h>
#include <DirectXMath.h>
#include <cstddef>


#ifdef _WIN64
	typedef uint64_t DEBUGINT;
#else
	typedef uint32_t DEBUGINT;
#endif

//this shouldnt be here, but its a cool temporary hack until I use SimpleMath or build my own library
typedef __declspec(align(16))union { DirectX::XMVECTOR v; DirectX::XMFLOAT4A f; } VectorRegister;

#if _DEBUG
#	define DebugLog(fmt, ...) EngineDebugLog(fmt, __VA_ARGS__);
#else
#	define DebugLog(fmt, ...)
#endif

// Safely release a COM object.
template<typename T>
inline void SafeRelease(T& ptr)
{
	if (ptr != NULL)
	{
		ptr->Release();
		ptr = NULL;
	}
}

// Safely delete an object on the heap.
template<typename T>
inline void SafeDeleteHeap(T& ptr)
{
	if (ptr != NULL)
	{
		delete ptr;
		ptr = NULL;
	}
}

inline void EngineDebugLog(const char *szTypes, ...)
{
	va_list args;
	va_start(args, szTypes);

	std::stringstream ss;

	// Step through the list.  
	for (int i = 0; szTypes[i] != '\0'; ++i) {
		switch (szTypes[i]) {   // Type to expect.  
			case 'i':
				ss << va_arg(args, int);
				break;

			case 'f':
				ss << va_arg(args, double);
				break;

			case 'c':
				ss << va_arg(args, char);
				break;

			case 's':
				ss << va_arg(args, char *);
				break;

			default:
				break;
			}
	}
	ss << std::endl;
	va_end(args);

	OutputDebugStringA(ss.str().c_str());
}
