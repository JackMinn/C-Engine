#pragma once

#include <d3d11.h>
#include "stdafx.h"
#include "d3dengine.h"
#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "GameObjectRenderable.h"
#include "Texture.h"
#include <vector>
#include <unordered_map>
#include "Material.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;

class GraphicsClass
{
public:
	Camera* m_mainCamera;
	Model* m_Model;
	Shader* m_ColorShader;
	std::vector<GameObjectRenderable> m_gameObjects;
	GameObjectRenderable* m_skyBox;
	Model* m_skyBoxModel;
	Shader* m_skyBoxShader;
	Texture* m_skyBoxTex;

	Material* m_skyBoxMaterial;
	Material* m_objectMaterial;

public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool RenderFrame();

private:
	bool Render();

private:
	D3DEngine* m_Direct3D;
	//this is being updated several times per frame, will cause stalling in reality, each object should have their own draw buffer 
	//do cbuffers reside in vram when they are not bound to a shader?
	ID3D11Buffer* m_drawCBuffer; 
	ID3D11Buffer* m_frameCBuffer;
	ID3D11Buffer* m_applicationCBuffer;
};
