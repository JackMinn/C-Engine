#include "stdafx.h"
#include "GameObjectFactory.h"
#include "GameObject.h"
#include "TinyXML2.h"
#include <assert.h>
#include "EngineUtil.h"
#include "Component.h"

std::shared_ptr<GameObject> GameObjectFactory::CreateGameObject(TinyXML2::XMLDocument* const& data)
{
	using namespace TinyXML2;

	XMLElement* root = data->RootElement();
	if (root == nullptr) 
	{
		return std::shared_ptr<GameObject>();
	}
	assert(std::strcmp(root->Name(), "Root") == 0);

	std::shared_ptr<GameObject> gameObject(new GameObject(GetNextGameObjectID()));

	for (XMLElement* componentData = root->FirstChildElement(); componentData != nullptr; componentData = componentData->NextSiblingElement())
	{
		Component* newComponent(CreateComponent(componentData));

		if (newComponent != nullptr)
		{
			gameObject->AddComponent(newComponent);
			newComponent->SetOwner(gameObject);
		}
		else
		{
			return std::shared_ptr<GameObject>();
		}
	}

	return gameObject;
}

Component* GameObjectFactory::CreateComponent(TinyXML2::XMLElement* const& data)
{
	std::string name = data->Name();
	Component* component = nullptr;

	auto factoryIterator = m_ComponentFactoryMap.find(name);
	if (factoryIterator != m_ComponentFactoryMap.end())
	{
		ComponentFactory factory = factoryIterator->second;
		//Perhaps rather than creating with a default constructor, we can create with "data" and thus construct the component with the correct values 
		component = factory(data);
	}
	else
	{
		DebugLog("ss", "Could not find factory for type: ", name);
		assert(component == nullptr);
		return component;
	}

	if (component != nullptr)
	{
		if (!component->Initialize(data))
		{
			DebugLog("ss", "Failed to initialize component of type: ", name);
			return nullptr;
		}
	}

	return component;
}