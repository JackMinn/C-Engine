#include "stdafx.h"
#include "GameEngine.h"
#include "Input.h"
#include "GraphicsClass.h"
#include "GameObject.h"
#include "TinyXML2.h"

#include "GlobalExterns.h"


bool GameEngine::Initialize(const uint32_t& screenWidth, const uint32_t& screenHeight, HWND const& windowHandle)
{
	bool result;

	m_Input = Input::Get();
	if (!m_Input)
	{
		return false;
	}

	m_Input->Initialize();

	m_Graphics = new GraphicsClass;
	if (!m_Graphics)
	{
		return false;
	}

	result = m_Graphics->Initialize(screenWidth, screenHeight, windowHandle);
	if (!result)
	{
		return false;
	}

	m_GameObjectFactory = std::unique_ptr<GameObjectFactory>(new GameObjectFactory());

	//https://shilohjames.wordpress.com/2014/04/27/tinyxml2-tutorial/
	using namespace TinyXML2;
	using TinyXML2::XMLDocument;
	{
		XMLDocument xmlDoc;
		XMLNode* pRoot = xmlDoc.NewElement("Root");
		xmlDoc.InsertFirstChild(pRoot);
		XMLElement* pElement = xmlDoc.NewElement("RenderableComponent");
		pRoot->InsertEndChild(pElement);
		pElement->SetAttribute("Material", "MyMaterial");
		pElement->SetAttribute("Model", "MyModel");
		xmlDoc.SaveFile("../Assets/GameObjects/GO0.xml");

		std::shared_ptr<GameObject> gameObject = m_GameObjectFactory->CreateGameObject(&xmlDoc);
		m_GameObjectFactory->GlobalUpdate();
		gameObject->Destroy();
	}

	XMLDocument xmlDoc;
	XMLNode* pRoot = xmlDoc.NewElement("Root");
	xmlDoc.InsertFirstChild(pRoot);
	XMLElement* pElement = xmlDoc.NewElement("IntValue");
	pElement->SetText(10);
	pRoot->InsertEndChild(pElement);
	pElement->SetAttribute("day", 26);
	pElement->SetAttribute("month", "April");
	pElement->SetAttribute("year", 2014);
	pElement->SetAttribute("dateFormat", "26/04/2014");
	xmlDoc.SaveFile("../Assets/Materials/test.xml");

	XMLDocument xmlDoc2;
	xmlDoc2.LoadFile("../Assets/Materials/test.xml");
	XMLNode* newRoot = xmlDoc2.FirstChild();
	int32_t intOut;
	XMLElement* newElement = newRoot->FirstChildElement("IntValue");
	XMLError error = newElement->QueryIntText(&intOut);
	XMLCheckResult(error);
	DebugLog("si", "Int value from XML is: ", intOut);

	return true;
}

void GameEngine::Shutdown()
{
	DebugLog("s", "System is gracefully shutting down.");

	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = nullptr;
	}

	if (m_Input)
	{
		delete m_Input;
		m_Input = nullptr;
	}
}

void GameEngine::ProcessGameLoop() 
{
	m_Graphics->m_mainCamera->Update();
}

bool GameEngine::RenderFrame()
{
	bool result;
	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	result = m_Graphics->RenderFrame();
	if (!result)
	{
		return false;
	}

	return true;
}
