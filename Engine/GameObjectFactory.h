#pragma once

#include <unordered_map>
#include <memory>
#include "RenderableComponent.h"

namespace TinyXML2 
{
	class XMLDocument;
	class XMLElement;
}
class GameObject;
class Component;

//alias for function pointer which takes no arguments and returns a Component pointer, the alias type name is ComponentFactory
typedef Component* (*ComponentFactory)(TinyXML2::XMLElement* data);
typedef void (*ComponentUpdateAll)();

typedef std::unordered_map<std::string, ComponentFactory> ComponentFactoryMap;

//consider making this a singleton
class GameObjectFactory
{
public:
	//Need to update this system so that the function map and vector can be updated more elegantly than just hardcoding it
	GameObjectFactory() : m_LastGameObjectID(0) 
	{ 
		m_ComponentFactoryMap.insert({"RenderableComponent", RenderableComponent::Create});
		m_ComponentUpdateFunctions.emplace_back(RenderableComponent::UpdateAll);
	}
	//Not a copyable type
	GameObjectFactory(const GameObjectFactory& c) = delete;
	GameObjectFactory& operator=(const GameObjectFactory&) = delete;
	~GameObjectFactory() {}

	std::shared_ptr<GameObject> CreateGameObject(TinyXML2::XMLDocument* const& data);
	inline void GlobalUpdate()
	{
		for (uint32_t i = 0; i < m_ComponentUpdateFunctions.size(); i++)
		{
			m_ComponentUpdateFunctions[i]();
		}
	}

private:
	uint32_t m_LastGameObjectID;
	ComponentFactoryMap m_ComponentFactoryMap;
	std::vector<ComponentUpdateAll> m_ComponentUpdateFunctions;

	Component* CreateComponent(TinyXML2::XMLElement* const& data);
	inline uint32_t GetNextGameObjectID(void) { ++m_LastGameObjectID; return m_LastGameObjectID; }
};

