#include "stdafx.h"
#include "Shader.h"
#include "EngineUtil.h"

LPCSTR g_vertexEntry = "VSMain";
LPCSTR g_pixelEntry = "PSMain";


Shader::Shader()
{
	m_depthEnable = true;
	m_depthFunc = D3D11_COMPARISON_LESS;
	m_cullMode = D3D11_CULL_BACK;

	m_samplerState = nullptr;
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_inputLayout = nullptr;
}

Shader::Shader(const Shader&)
{

}

Shader::~Shader()
{
	DebugLog("s", "Destroying Shader.");
}

bool Shader::Initialize(ID3D11Device* device, LPCWSTR vsFilename, LPCWSTR psFilename)
{
	// Initialize the vertex and pixel shaders.
	HRESULT result;
	ID3DBlob* errorMessage;
	ID3DBlob* vertexShaderBlob;
	ID3DBlob* pixelShaderBlob;
	D3D11_BUFFER_DESC cBufferDesc;
	ZeroMemory(&cBufferDesc, sizeof(D3D11_BUFFER_DESC));
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));


	// Initialize the pointers this function will use to null.
	errorMessage = nullptr;
	vertexShaderBlob = nullptr;
	pixelShaderBlob = nullptr;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, g_vertexEntry, "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBlob, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			DebugLog("ssss", "Error compiling ", vsFilename, ". ", reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			DebugLog("ss", "Missing shader file: ", vsFilename);
		}

		return false;
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, NULL, g_pixelEntry, "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBlob, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			DebugLog("ssss", "Error compiling ", psFilename, ". ", reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			DebugLog("ss", "Missing shader file: ", psFilename); //doesnt currently print the full name
		}

		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		DebugLog("s", "Unable to create vertex shader from blob.");
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		DebugLog("s", "Unable to create pixel shader from blob.");
		return false;
	}

	result = ShaderUtils::GetInputLayoutFromVertexShaderSignature(vertexShaderBlob, device, &m_inputLayout);
	if (FAILED(result))
	{
		DebugLog("s", "Unable to create input layout from vertex blob.");
		return false;
	}

	result = ShaderUtils::GetConstantBuffersFromShader(vertexShaderBlob, m_cbSize, m_cbVariables);
	if (FAILED(result))
	{
		DebugLog("s", "Unable to create globals constant buffer from vertex blob.");
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	SafeRelease(vertexShaderBlob);
	SafeRelease(pixelShaderBlob);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void Shader::Shutdown()
{
	SafeRelease(m_samplerState);
	SafeRelease(m_inputLayout);
	SafeRelease(m_pixelShader);
	SafeRelease(m_vertexShader);
}

//bool Shader::Render(ID3D11DeviceContext* deviceContext, int indexCount, ID3D11ShaderResourceView* texture)
//{
//	// Set the shader parameters that it will use for rendering.
//	bool result = SetShaderParameters(deviceContext, texture);
//	if (!result)
//	{
//		return false;
//	}
//
//	// Now render the prepared buffers with the shader.
//	RenderShader(deviceContext, indexCount);
//
//	return true;
//}

//[https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0]
//bool Shader::SetShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture)
//{
//	HRESULT result;
//
//	DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
//	float scale = 1.0f;
//	//float scale = 0.01f;
//	float scale2 = 3.0f;
//
//	D3D11_BUFFER_DESC bufferDesc;
//	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
//	m_globalsCBuffer->GetDesc(&bufferDesc);
//
//	void* globalsBuffer = malloc(bufferDesc.ByteWidth);
//	memcpy(globalsBuffer, &color, sizeof(color));
//	void* b = (void*)((char*)globalsBuffer + sizeof(color));
//	memcpy(b, &scale, sizeof(scale));
//	void* c = (void*)((char*)b + sizeof(scale));
//	memcpy(c, &scale2, sizeof(scale2));
//
//	deviceContext->UpdateSubresource(m_globalsCBuffer, 0, nullptr, globalsBuffer, 0, 0);
//
//	free(globalsBuffer);
//
//	//at the moment there is one $Globals buffer holding everything
//	deviceContext->VSSetConstantBuffers(0, 1, &m_globalsCBuffer);
//	deviceContext->PSSetConstantBuffers(0, 1, &m_globalsCBuffer); 
//
//	// Set shader texture resource in the pixel shader.
//	if (texture != nullptr) 
//	{
//		deviceContext->PSSetShaderResources(0, 1, &texture);
//	}
//
//	return true;
//}

//void Shader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
//{
//	// Set the vertex input layout.
//	deviceContext->IASetInputLayout(m_inputLayout);
//
//	// Set the vertex and pixel shaders that will be used to render this triangle.
//	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
//	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
//
//	// Set the sampler state in the pixel shader.
//	deviceContext->PSSetSamplers(0, 1, &m_samplerState);
//
//	// Render the triangle.
//	deviceContext->DrawIndexed(indexCount, 0, 0);
//
//	return;
//}

//DEFINE FUNCTIONS FROM SHADERUTILS NAMESPACE

HRESULT ShaderUtils::GetInputLayoutFromVertexShaderSignature(ID3DBlob* shaderBlob, ID3D11Device* device, ID3D11InputLayout** inputLayout)
{
	HRESULT result; 

	// Reflect shader info
	ID3D11ShaderReflection* vertexShaderReflection = nullptr;
	result = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)&vertexShaderReflection);
	if (FAILED(result))
	{
		return result;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	vertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		vertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlot = 0;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format (UINT/SINT/FLOAT R/RG/RGB/RGB
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	HRESULT hr = device->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), inputLayout);

	//Free allocation shader reflection memory
	vertexShaderReflection->Release();
	return hr;
}

HRESULT ShaderUtils::GetConstantBuffersFromShader(ID3DBlob* const& shaderBlob, uint32_t& cbSize, std::vector<std::pair<std::string, uint32_t>>& cbVariables)
{
	HRESULT result;

	// Reflect shader info
	ID3D11ShaderReflection* shaderReflection = nullptr;
	result = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)&shaderReflection);
	if (FAILED(result))
	{
		return result;
	}

	D3D11_SHADER_BUFFER_DESC bufferDesc;
	ID3D11ShaderReflectionConstantBuffer* cBuffer;
	ID3D11ShaderReflectionVariable* cBufferVariable;
	D3D11_SHADER_VARIABLE_DESC varDesc;

	cBuffer = shaderReflection->GetConstantBufferByIndex(0);
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	result = cBuffer->GetDesc(&bufferDesc);
	if (FAILED(result))
	{
		DebugLog("s", "No globals constant buffer exists");
		cbSize = 0;
		return S_OK;
	}
	DebugLog("sssisi", "Buffer name is: ", bufferDesc.Name, " Number of variables: ", bufferDesc.Variables, " Buffer size: ", bufferDesc.Size);
	cbSize = bufferDesc.Size;

	for (int varIndex = 0; varIndex < bufferDesc.Variables; varIndex++) {
		cBufferVariable = cBuffer->GetVariableByIndex(varIndex);
		ZeroMemory(&varDesc, sizeof(varDesc));
		result = cBufferVariable->GetDesc(&varDesc);
		if (FAILED(result))
		{
			DebugLog("sisi", "No variable exists at index: ", varIndex, " exists in cbuffer", bufferDesc.Name);
			return result;
		}
		cbVariables.emplace_back(std::make_pair(varDesc.Name, varDesc.Size));
	}

	return result;
}
		