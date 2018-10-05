#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

#define CB_DRAW_SIZE 128
#define CB_FRAME_SIZE 80
#define CB_APPLICATION_SIZE 128

enum ConstanBuffer
{
	CB_GLOBALS,
	CB_DRAW,
	CB_FRAME,
	CB_APPLICATION,
	CB_COUNT
};

namespace ConstantBuffer {
	HRESULT Create(ID3D11Device*, ID3D11Buffer**, unsigned int);
}

