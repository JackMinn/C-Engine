#pragma once

//[https://www.gamedev.net/forums/topic/610607-dx11-constant-buffers-per-material-or-per-shader/]

#include "Shader.h"
#include "ConstantBuffer.h"
#include "EngineUtil.h"
#include <d3d11.h>
#include <unordered_map>

class Material
{
public:
	ID3D11Device* const m_device;
	ID3D11DeviceContext* const m_deviceContext;
	const Shader* const m_shader;

	explicit Material(ID3D11Device* device, ID3D11DeviceContext* deviceContext, Shader* shader) : m_device(device), m_deviceContext(deviceContext), m_shader(shader) 
	{
		m_materialPropertyBlock = nullptr;
		m_globalsCBuffer = nullptr;
	}
	~Material();

	bool Initialize();
	void SetFloat(LPCSTR, float const&);
	void SetFloats(LPCSTR, float* const&, uint32_t);
	bool Render(uint32_t, ID3D11ShaderResourceView* = nullptr);

private:
	void* m_materialPropertyBlock;
	ID3D11Buffer* m_globalsCBuffer;
	std::unordered_map<std::string, void*> m_propertyMap;

	void SetMaterialParameters();
};

