#include "stdafx.h"
#include "Texture.h"


Texture::Texture()
{
	m_texture = nullptr;
	m_textureResourceView = nullptr;
	m_forceSRGB = false;
}

Texture::Texture(const Texture& other)
{

}

Texture::~Texture()
{
	assert(m_texture == nullptr);
	assert(m_textureResourceView == nullptr);
}

bool Texture::Initialize(ID3D11Device* device, LPCWSTR filename, bool forceSRGB)
{
	HRESULT result;
	m_forceSRGB = forceSRGB;

	result = DirectX::CreateDDSTextureFromFile(device, filename, (ID3D11Resource**)&m_texture, &m_textureResourceView, m_forceSRGB);
	if(FAILED(result))
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	m_texture->GetDesc(&texDesc);
	DebugLog("si", "Array size is: ", texDesc.ArraySize);
	DebugLog("si", "Mip chain size is: ", texDesc.MipLevels);
	DebugLog("si", "Format enum is: ", (unsigned int)texDesc.Format);

	D3D11_SHADER_RESOURCE_VIEW_DESC resDesc;
	ZeroMemory(&resDesc, sizeof(resDesc));
	m_textureResourceView->GetDesc(&resDesc);
	DebugLog("si", "Resource forman enum is: ", resDesc.Format);

	return true;
}

void Texture::Shutdown()
{
	SafeRelease(m_texture);
	SafeRelease(m_textureResourceView);
}

ID3D11ShaderResourceView* Texture::GetTexture()
{
	assert(m_textureResourceView != nullptr);
	return m_textureResourceView;
}

