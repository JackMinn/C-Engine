#include "stdafx.h"
#include "graphicsclass.h"
#include "EngineUtil.h"
#include <DirectXMath.h>
#include "SimpleMath.h"
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include "DDSReader.h"
#include "IrradianceMapGen.h"

#define DEBUG_GO_COUNT 1
#define MESH_INDEX 0


GraphicsClass::GraphicsClass()
{
	m_mainCamera = nullptr;
	m_Direct3D = nullptr;
	m_Model = nullptr;
	m_ColorShader = nullptr;
	m_skyBox = nullptr;
	m_skyBoxModel = nullptr;
	m_skyBoxShader = nullptr;
	m_skyBoxTex = nullptr;
	m_gameObjects.reserve(DEBUG_GO_COUNT);
	m_skyBoxMaterial = nullptr;
	m_objectMaterial = nullptr;
}

GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}

GraphicsClass::~GraphicsClass()
{
	assert(m_mainCamera == nullptr);
	assert(m_Direct3D == nullptr);
	assert(m_Model == nullptr);
	assert(m_ColorShader == nullptr);
	assert(m_skyBox == nullptr);
	assert(m_skyBoxModel == nullptr);
	assert(m_skyBoxShader == nullptr);
	assert(m_skyBoxTex == nullptr);
	assert(m_skyBoxMaterial == nullptr);
	assert(m_objectMaterial == nullptr);
}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
	HRESULT hresult;
	Assimp::Importer importer;

	//we make camera first to get viewport for D3DEngine
	m_mainCamera = new Camera(screenWidth, screenHeight);
	if (!m_mainCamera)
	{
		return false;
	}

	// Create the Direct3D object.
	m_Direct3D = new D3DEngine();
	if (!m_Direct3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, &m_mainCamera->m_ViewPort);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	//Initialize the draw constant buffer
	hresult = ConstantBuffer::Create(m_Direct3D->GetDevice(), &m_drawCBuffer, CB_DRAW_SIZE);
	if (FAILED(hresult))
	{
		MessageBox(hwnd, L"Could not create draw constant buffer", L"Error", MB_OK);
		return false;
	}

	//Initialize the frame constant buffer
	hresult = ConstantBuffer::Create(m_Direct3D->GetDevice(), &m_frameCBuffer, CB_FRAME_SIZE);
	if (FAILED(hresult))
	{
		MessageBox(hwnd, L"Could not create frame constant buffer", L"Error", MB_OK);
		return false;
	}

	//Initialize the application constant buffer
	hresult = ConstantBuffer::Create(m_Direct3D->GetDevice(), &m_applicationCBuffer, CB_APPLICATION_SIZE);
	if (FAILED(hresult))
	{
		MessageBox(hwnd, L"Could not create application constant buffer", L"Error", MB_OK);
		return false;
	}

	DirectX::SimpleMath::Matrix projectionMatrix;
	DirectX::SimpleMath::Matrix invProjectionMatrix;
	m_mainCamera->BuildProjectionMatrix();
	m_mainCamera->GetGPUProjectionMatrix(projectionMatrix);
	m_mainCamera->GetGPUInvProjectionMatrix(invProjectionMatrix);

	void* applicationBuffer = malloc(CB_APPLICATION_SIZE);
	memcpy(applicationBuffer, &projectionMatrix, sizeof(projectionMatrix));
	memcpy((void*)((char*)applicationBuffer + sizeof(projectionMatrix)), &invProjectionMatrix, sizeof(invProjectionMatrix));

	m_Direct3D->GetDeviceContext()->UpdateSubresource(m_applicationCBuffer, 0, nullptr, applicationBuffer, 0, 0);
	free(applicationBuffer);

	m_Direct3D->GetDeviceContext()->VSSetConstantBuffers(CB_DRAW, 1, &m_drawCBuffer);
	m_Direct3D->GetDeviceContext()->PSSetConstantBuffers(CB_DRAW, 1, &m_drawCBuffer);
	m_Direct3D->GetDeviceContext()->VSSetConstantBuffers(CB_FRAME, 1, &m_frameCBuffer);
	m_Direct3D->GetDeviceContext()->PSSetConstantBuffers(CB_FRAME, 1, &m_frameCBuffer);
	m_Direct3D->GetDeviceContext()->VSSetConstantBuffers(CB_APPLICATION, 1, &m_applicationCBuffer);
	m_Direct3D->GetDeviceContext()->PSSetConstantBuffers(CB_APPLICATION, 1, &m_applicationCBuffer);

	//load fbx scene files
	const aiScene* skyBox = importer.ReadFile("sphere.fbx", aiProcessPreset_TargetRealtime_Quality | aiProcess_ConvertToLeftHanded);
	aiMesh* skyBoxMesh = skyBox->mMeshes[MESH_INDEX];
	m_skyBoxModel = new Model(skyBoxMesh);

	const aiScene* scene = importer.ReadFile("stanford-dragon.fbx", aiProcessPreset_TargetRealtime_Quality | aiProcess_ConvertToLeftHanded);

	if (scene == nullptr) {
		DebugLog("s", "Failed to import. Creating box meshes instead.");

		m_Model = new Model();
		if (!m_Model)
		{
			return false;
		}
	}
	else {
		aiMesh* mesh = scene->mMeshes[MESH_INDEX];

		m_Model = new Model(mesh);
		if (!m_Model)
		{
			return false;
		}		
	}

	// Initialize the skybox.
	result = m_skyBoxModel->Initialize(m_Direct3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the skybox object.", L"Error", MB_OK);
		return false;
	}

	// Initialize the model object.
	result = m_Model->Initialize(m_Direct3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// Create the color shader object.
	m_skyBoxShader = new Shader;
	if (!m_skyBoxShader)
	{
		return false;
	}

	// Initialize the color shader object.
	result = m_skyBoxShader->Initialize(m_Direct3D->GetDevice(), L"Skybox.hlsl", L"Skybox.hlsl");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the skybox shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the color shader object.
	m_ColorShader = new Shader;
	if (!m_ColorShader)
	{
		return false;
	}

	// Initialize the color shader object.
	result = m_ColorShader->Initialize(m_Direct3D->GetDevice(), L"shader.hlsl", L"shader.hlsl");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

	m_skyBoxMaterial = new Material(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), m_skyBoxShader);
	m_skyBoxMaterial->Initialize();
	m_objectMaterial = new Material(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), m_ColorShader);
	m_objectMaterial->Initialize();

	m_objectMaterial->SetFloat("scale", 3.0f);

	//make skybox object
	//m_skyBox = new GameObjectRenderable(m_skyBoxModel, m_skyBoxMaterial, m_mainCamera->m_position, DirectX::SimpleMath::Quaternion::Identity);
	m_skyBox = new GameObjectRenderable(m_skyBoxModel, m_skyBoxMaterial, m_mainCamera->m_Transform.GetPosition(), DirectX::SimpleMath::Quaternion::Identity);

	//make game objects
	for (int i = 0; i < DEBUG_GO_COUNT; i++) 
	{
		float x = (((float)rand() / (RAND_MAX + 1)) * 20) - 10;
		float y = (((float)rand() / (RAND_MAX + 1)) * 14) - 7;
		y = 0;
		float z = (((float)rand() / (RAND_MAX + 1)) * 40) - 20;
		DirectX::SimpleMath::Vector3 pos = { x,y,z };

		float eulerX = ((float)rand() / RAND_MAX + 1) * 180;
		eulerX = 0;
		float eulerY = ((float)rand() / RAND_MAX + 1) * 180;
		float eulerZ = ((float)rand() / RAND_MAX + 1) * 180;
		eulerZ = 0;
		DirectX::SimpleMath::Quaternion rot = DirectX::SimpleMath::Quaternion::CreateFromPitchYawRoll(eulerX, eulerY, eulerZ);

		pos = DirectX::SimpleMath::Vector3::Zero;
		rot = DirectX::SimpleMath::Quaternion::Identity;

		m_gameObjects.emplace_back(m_Model, m_objectMaterial, pos, rot);
		//m_gameObjects.push_back(GameObjectRenderable(m_Model, m_ColorShader, DirectX::SimpleMath::Vector3::One, DirectX::SimpleMath::Quaternion::Identity));
	}

	m_skyBoxTex = new Texture();
	result = m_skyBoxTex->Initialize(m_Direct3D->GetDevice(), L"skybox.dds", true);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the sky box texture.", L"Error", MB_OK);
		return false;
	}

	//building irradiance map
	std::vector<uint8_t*> cubeFacePtrs;
	DDSReader::LoadDDSCubeMap(L"skybox.dds", cubeFacePtrs);
	IrradianceMapGen::GenerateIrradianceSH(cubeFacePtrs, 1024, 4);

	delete[] cubeFacePtrs[0];
	delete[] cubeFacePtrs[1];
	delete[] cubeFacePtrs[2];
	delete[] cubeFacePtrs[3];
	delete[] cubeFacePtrs[4];
	delete[] cubeFacePtrs[5];

	return true;
}

void GraphicsClass::Shutdown()
{
	SafeDeleteHeap(m_mainCamera);
	SafeDeleteHeap(m_skyBox);
	SafeRelease(m_drawCBuffer);
	SafeRelease(m_frameCBuffer);
	SafeRelease(m_applicationCBuffer);

	SafeDeleteHeap(m_skyBoxMaterial);
	SafeDeleteHeap(m_objectMaterial);

	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = nullptr;
	}
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = nullptr;
	}
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = nullptr;
	}
	if (m_skyBoxModel)
	{
		m_skyBoxModel->Shutdown();
		delete m_skyBoxModel;
		m_skyBoxModel = nullptr;
	}
	if (m_skyBoxShader)
	{
		m_skyBoxShader->Shutdown();
		delete m_skyBoxShader;
		m_skyBoxShader = nullptr;
	}
	if (m_skyBoxTex)
	{
		m_skyBoxTex->Shutdown();
		delete m_skyBoxTex;
		m_skyBoxTex = nullptr;
	}
}

bool GraphicsClass::RenderFrame()
{
	bool result = Render();
	if (!result)
		return false;

	return true;
}

bool GraphicsClass::Render()
{
	using namespace DirectX::SimpleMath;
	ID3D11DeviceContext* deviceContext = m_Direct3D->GetDeviceContext();
	Matrix viewMatrix;

	// Clear the buffers to begin the scene.
	m_Direct3D->BeginFrame(m_mainCamera->m_ClearColor);

	void* frameBuffer = malloc(CB_FRAME_SIZE);
	m_mainCamera->BuildViewMatrix();
	m_mainCamera->GetGPUViewMatrix(viewMatrix);
	DirectX::SimpleMath::Vector4 homogeneousPos = { m_mainCamera->m_Transform.GetPosition().x, m_mainCamera->m_Transform.GetPosition().y, m_mainCamera->m_Transform.GetPosition().z, 1.0f };
	memcpy(frameBuffer, &viewMatrix, sizeof(viewMatrix));
	void* frameBufferIndex1 = (void*)((char*)frameBuffer + sizeof(viewMatrix));
	memcpy(frameBufferIndex1, &homogeneousPos, sizeof(homogeneousPos));
	deviceContext->UpdateSubresource(m_frameCBuffer, 0, nullptr, frameBuffer, 0, 0);
	free(frameBuffer);


	void* drawBuffer = malloc(CB_DRAW_SIZE);
	void* drawBufferIndex1 = (void*)((char*)drawBuffer + sizeof(Matrix));
	m_skyBox->GetTransform().SetPosition(m_mainCamera->m_Transform.GetPosition());
	Matrix localToWorldMatrix = m_skyBox->GetTransform().GetLocalToWorldMatrix();
	localToWorldMatrix = XMMatrixTranspose(localToWorldMatrix);
	Matrix worldToLocalMatrix = m_skyBox->GetTransform().GetWorldToLocalMatrix();
	worldToLocalMatrix = XMMatrixTranspose(worldToLocalMatrix);

	memcpy(drawBuffer, &localToWorldMatrix, sizeof(localToWorldMatrix));
	memcpy(drawBufferIndex1, &worldToLocalMatrix, sizeof(worldToLocalMatrix));

	deviceContext->UpdateSubresource(m_drawCBuffer, 0, nullptr, drawBuffer, 0, 0);

	m_skyBox->GetMesh()->Render(deviceContext, m_drawCBuffer); //should update this function, it does not need this information anymore
	//bool result = m_skyBox->GetShader()->Render(deviceContext, m_skyBox->GetMesh()->GetIndexCount(), m_skyBoxTex->GetTexture());
	bool result = m_skyBox->GetMaterial()->Render(m_skyBox->GetMesh()->GetIndexCount(), m_skyBoxTex->GetTexture());
	if (!result)
	{
		return false;
	}

	for (unsigned int i = 0; i < m_gameObjects.size(); i++) {
		DirectX::SimpleMath::Matrix localToWorldMatrix = m_gameObjects[i].GetTransform().GetLocalToWorldMatrix();
		localToWorldMatrix = XMMatrixTranspose(localToWorldMatrix);
		DirectX::SimpleMath::Matrix worldToLocalMatrix = m_gameObjects[i].GetTransform().GetWorldToLocalMatrix();
		worldToLocalMatrix = XMMatrixTranspose(worldToLocalMatrix);

		memcpy(drawBuffer, &localToWorldMatrix, sizeof(localToWorldMatrix));
		memcpy(drawBufferIndex1, &worldToLocalMatrix, sizeof(worldToLocalMatrix));

		deviceContext->UpdateSubresource(m_drawCBuffer, 0, nullptr, drawBuffer, 0, 0);

		m_gameObjects[i].GetMesh()->Render(deviceContext, m_drawCBuffer); //should update this function, it does not need this information anymore
		//bool result = m_gameObjects[i].GetShader()->Render(deviceContext, m_gameObjects[i].GetMesh()->GetIndexCount(), m_skyBoxTex->GetTexture());
		bool result = m_gameObjects[i].GetMaterial()->Render(m_gameObjects[i].GetMesh()->GetIndexCount(), m_skyBoxTex->GetTexture());
		if (!result)
		{
			return false;
		}
	}
	free(drawBuffer);

	m_Direct3D->EndFrame();
	return true;
}