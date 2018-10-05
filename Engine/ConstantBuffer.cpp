#include "stdafx.h"
#include "ConstantBuffer.h"
#include "EngineUtil.h"


HRESULT ConstantBuffer::Create(ID3D11Device* device, ID3D11Buffer** constantBuffer, unsigned int bufferSize)
{
	D3D11_BUFFER_DESC cBufferDesc;

	ZeroMemory(&cBufferDesc, sizeof(D3D11_BUFFER_DESC));
	cBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	cBufferDesc.ByteWidth = bufferSize;
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = 0;

	HRESULT result = device->CreateBuffer(&cBufferDesc, NULL, constantBuffer);
	if (FAILED(result)) 
	{
		DebugLog("s", "Failed to create the buffer");
	}
	else
	{
		DebugLog("s", "Buffer created.");
	}
	return result;
}
