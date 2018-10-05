#pragma once

#include <d3d11.h>
#include "SimpleMath.h"
#include <directxmath.h>
#include <assimp\mesh.h>

class Model
{
private:
	__declspec(align(16)) struct VertexType
	{
		DirectX::SimpleMath::Vector4 position;
		DirectX::SimpleMath::Vector4 normal;
		DirectX::SimpleMath::Vector4 color;
	};

public:
	Model();
	Model(aiMesh* mesh);
	Model(const Model&);
	~Model();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	void Render(ID3D11DeviceContext*, ID3D11Buffer*);

	inline const int GetVertexCount() const { return m_vertexCount; }
	inline const int GetIndexCount() const { return m_indexCount; }

	static void BuildTriangle(VertexType**, unsigned long**, int&, int&);
	static void BuildCube(VertexType**, unsigned long**, int&, int&);

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void SetDrawBufferProperties(ID3D11DeviceContext*, ID3D11Buffer*);
	void SetMeshDrawProperties(ID3D11DeviceContext*);
	
private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	VertexType* m_vertices;
	unsigned long* m_indices;
};
