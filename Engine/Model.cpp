#include "stdafx.h"
#include "Model.h"
#include "EngineUtil.h"


Model::Model()
{
	m_vertices = nullptr;
	m_indices = nullptr;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
}

Model::Model(aiMesh* mesh) 
{
	using namespace DirectX::SimpleMath;

	m_vertexCount = mesh->mNumVertices;
	m_indexCount = mesh->mNumFaces * 3;
	m_vertices = new VertexType[m_vertexCount];
	m_indices = new unsigned long[m_indexCount];
	for (unsigned int i = 0; i < m_vertexCount; i++) {
		Vector4 v = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1 };
		Vector4 n = mesh->HasNormals() ? Vector4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 1.0f)
									   : Vector4::One;
		//Vector4 c = { mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a };
		Vector4 c = mesh->HasVertexColors(0) ? Vector4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a)
											 : Vector4::One;
		m_vertices[i] = { v, n, c };
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < 3; j++) {
			m_indices[i*3 + j] = face.mIndices[j];
		}		
	}

	DebugLog("sisi", "Number of vertices is: ", m_vertexCount, ". Number of primitives is: ", (m_indexCount / 3));
}

Model::Model(const Model& other)
{
}

Model::~Model()
{
}

bool Model::Initialize(ID3D11Device* device)
{
	// Initialize the vertex and index buffers.
	bool result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	return true;
}

void Model::Shutdown()
{
	// Shutdown the vertex and index buffers.
	ShutdownBuffers();
}

void Model::Render(ID3D11DeviceContext* deviceContext, ID3D11Buffer* drawCBuffer)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	SetMeshDrawProperties(deviceContext);
	//SetDrawBufferProperties(deviceContext, drawCBuffer);
}

bool Model::InitializeBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	if (m_vertices == nullptr && m_indices == nullptr) {
		//BuildTriangle(&vertices, &indices, m_vertexCount, m_indexCount);
		BuildCube(&m_vertices, &m_indices, m_vertexCount, m_indexCount);
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = m_indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] m_vertices;
	m_vertices = nullptr;

	delete[] m_indices;
	m_indices = nullptr;

	return true;
}

void Model::ShutdownBuffers()
{
	// Release the index buffer.
	SafeRelease(m_indexBuffer);
	SafeRelease(m_vertexBuffer);
}

void Model::SetDrawBufferProperties(ID3D11DeviceContext* deviceContext, ID3D11Buffer* drawCBuffer)
{
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
	worldMatrix = XMMatrixTranspose(worldMatrix);

	deviceContext->UpdateSubresource(drawCBuffer, 0, nullptr, &worldMatrix, 0, 0);
}

void Model::SetMeshDrawProperties(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride, offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	assert(m_vertexBuffer != nullptr);
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	assert(m_indexBuffer != nullptr);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

void Model::BuildTriangle(VertexType** verticesPtr, unsigned long** indicesPtr, int& vertexCount, int& indexCount) 
{
	using namespace DirectX::SimpleMath;
	VertexType* vertices = new VertexType[3];
	unsigned long* indices = new unsigned long[3];

	vertices[0].position = DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f);  // Bottom left.
	vertices[0].normal = Vector4::One;
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	vertices[1].position = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);  // Top middle.
	vertices[1].normal = Vector4::One;
	vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT4(1.0f, -1.0f, 0.0f, 1.0f);  // Bottom right.
	vertices[2].normal = Vector4::One;
	vertices[2].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	indices[0] = 0;  // Bottom left.
	indices[1] = 1;  // Top middle.
	indices[2] = 2;  // Bottom right.

	vertexCount = 3;
	indexCount = 3;
	*verticesPtr = vertices;
	*indicesPtr = indices;
}

void Model::BuildCube(VertexType** verticesPtr, unsigned long** indicesPtr, int& vertexCount, int& indexCount)
{
	using namespace DirectX::SimpleMath;

	VertexType* vertices = new VertexType[8];
	vertices[0].position = DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f);
	vertices[0].color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	vertices[1].position = DirectX::XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f);
	vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	vertices[2].position = DirectX::XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f);
	vertices[2].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	vertices[3].position = DirectX::XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f);
	vertices[3].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	vertices[4].position = DirectX::XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f);
	vertices[4].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	vertices[5].position = DirectX::XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f);
	vertices[5].color = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	vertices[6].position = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertices[6].color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertices[7].position = DirectX::XMFLOAT4(1.0f, -1.0f, 1.0, 1.0f);
	vertices[7].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);

	vertices[0].normal = Vector4::One;
	vertices[1].normal = Vector4::One;
	vertices[2].normal = Vector4::One;
	vertices[3].normal = Vector4::One;
	vertices[4].normal = Vector4::One;
	vertices[5].normal = Vector4::One;
	vertices[6].normal = Vector4::One;
	vertices[7].normal = Vector4::One;

	unsigned long* indices = new unsigned long[36] 
	{
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};

	vertexCount = 8;
	indexCount = 36;
	*verticesPtr = vertices;
	*indicesPtr = indices;
}
