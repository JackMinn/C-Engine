#include "stdafx.h"
#include "Material.h"


Material::~Material()
{
	SafeRelease(m_globalsCBuffer);
	free(m_materialPropertyBlock);
}

bool Material::Initialize()
{
	HRESULT result;
	uint32_t cbSize = m_shader->m_cbSize;

	if (cbSize > 0) 
	{
		result = ConstantBuffer::Create(m_device, &m_globalsCBuffer, cbSize);
		if (FAILED(result))
		{
			return false;
		}
	}

	//allocate a block of memory to the cbuffer variables, then make a hash map to point into into this block of memory with the variable names
	m_materialPropertyBlock = malloc(cbSize);
	uint32_t ptrOffset = 0;
	for (uint32_t i = 0; i < m_shader->m_cbVariables.size(); i++) 
	{
		m_propertyMap.insert({ m_shader->m_cbVariables[i].first,  (void*)((char*)m_materialPropertyBlock + ptrOffset) });
		ptrOffset += m_shader->m_cbVariables[i].second;
	}

	SetMaterialParameters();

	return true;
}

//this is temporary for debugging
void Material::SetMaterialParameters()
{
	DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	float scale = 1.0f;
	float scale2 = 3.0f;

	SetFloats("color1", (float*)&color, 4);
	SetFloat("scale", scale);
	SetFloat("scale2", scale2);
}

bool Material::Render(uint32_t indexCount, ID3D11ShaderResourceView* texture)
{
	//at the moment there is one $Globals buffer holding everything
	if (m_globalsCBuffer != nullptr)
	{
		m_deviceContext->VSSetConstantBuffers(0, 1, &m_globalsCBuffer);
		m_deviceContext->PSSetConstantBuffers(0, 1, &m_globalsCBuffer);
	}

	if (texture != nullptr)
	{
		m_deviceContext->PSSetShaderResources(0, 1, &texture);
	}

	m_deviceContext->IASetInputLayout(m_shader->GetInputLayout());

	m_deviceContext->VSSetShader(m_shader->GetVertexShader(), NULL, 0);
	m_deviceContext->PSSetShader(m_shader->GetPixelShader(), NULL, 0);

	m_deviceContext->PSSetSamplers(0, 1, &m_shader->GetSamplerState());

	m_deviceContext->DrawIndexed(indexCount, 0, 0);

	return true;
}

//calling UpdateSubresource every time a change is not good if multiple changes are being made in bulk
//perhaps implement a dirty flag which is checked before issuing a draw call with this material, and if dirty, then call updatesubresource
//however this might stall the pipeline as we may need to wait for the update to finish before issuing the draw call
void Material::SetFloat(LPCSTR name, float const& value)
{
	auto it = m_propertyMap.find(name);
	if (it != m_propertyMap.end()) 
	{
		memcpy(m_propertyMap[name], &value, sizeof(value));
		m_deviceContext->UpdateSubresource(m_globalsCBuffer, 0, nullptr, m_materialPropertyBlock, 0, 0);
	}
}

void Material::SetFloats(LPCSTR name, float* const& value, uint32_t elementCount)
{
	auto it = m_propertyMap.find(name);
	if (it != m_propertyMap.end())
	{
		memcpy(m_propertyMap[name], value, sizeof(float) * elementCount);
		m_deviceContext->UpdateSubresource(m_globalsCBuffer, 0, nullptr, m_materialPropertyBlock, 0, 0);
	}
}