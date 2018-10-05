#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include <vector>
#include "ConstantBuffer.h"

class Shader
{
public:
	bool m_depthEnable;
	D3D11_COMPARISON_FUNC m_depthFunc;
	D3D11_CULL_MODE m_cullMode;
	uint32_t m_cbSize;
	std::vector<std::pair<std::string, uint32_t>> m_cbVariables;

	Shader();
	Shader(const Shader&);
	~Shader();

	bool Initialize(ID3D11Device*, LPCWSTR, LPCWSTR);
	void Shutdown();
	//bool Render(ID3D11DeviceContext*, int, ID3D11ShaderResourceView* = nullptr);

	inline ID3D11SamplerState* const& GetSamplerState() const { return m_samplerState; }
	inline ID3D11VertexShader* const& GetVertexShader() const { return m_vertexShader; }
	inline ID3D11PixelShader* const& GetPixelShader() const { return m_pixelShader; }
	inline ID3D11InputLayout* const& GetInputLayout() const { return m_inputLayout; }

private:
	//void RenderShader(ID3D11DeviceContext*, int);
	//HRESULT GetConstantBuffersFromShader(ID3DBlob*);
	//static HRESULT GetInputLayoutFromVertexShaderSignature(ID3DBlob*, ID3D11Device*, ID3D11InputLayout**);

private:
	ID3D11SamplerState * m_samplerState;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_inputLayout;
};

namespace ShaderUtils {
	HRESULT GetInputLayoutFromVertexShaderSignature(ID3DBlob*, ID3D11Device*, ID3D11InputLayout**);
	HRESULT GetConstantBuffersFromShader(ID3DBlob* const& shaderBlob, uint32_t& cbSize, std::vector<std::pair<std::string, uint32_t>>& cbVariables);
}

