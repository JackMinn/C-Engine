#pragma once

#pragma comment(lib, "d3d11.lib")

#include <d3d11.h>
#include "DDSTextureLoader.h"
#include <assert.h>
#include "EngineUtil.h"

class Texture
{
public:
	Texture();
	Texture(const Texture&);
	~Texture();

	bool Initialize(ID3D11Device*, LPCWSTR, bool = false);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

private:
	ID3D11Texture2D * m_texture;
	ID3D11ShaderResourceView* m_textureResourceView;
	bool m_forceSRGB;

};

