#pragma once
#include <stdint.h>
#include <memory>
#include <unordered_map>
#include <assert.h>
#include "EngineUtil.h"

class Component;

class GameObject
{
public:
	//Not a copyable type
	GameObject(const GameObject& c) = delete;
	GameObject& operator=(const GameObject&) = delete;
	~GameObject() { assert(m_ComponentMap.empty() && "Destroy was not called on GameObject before destructor."); DebugLog("s", "Calling destroy on gameobject."); }

	//This must be explicitly called before the destructor is, due to circular strong references. This will clear out the component map.
	void Destroy();

	inline uint32_t GetID() const { return m_GameObjectID; }

	template <typename ComponentType>
	std::weak_ptr<ComponentType> GetComponent(const uint32_t& id);

private:
	friend class GameObjectFactory;

	//For now only the GameObjectFactory can create GameObjects, otherwise IDs will not be properly created. 
	explicit GameObject(uint32_t ID) : m_GameObjectID(ID) {}

	//this can remain private for now, such that we cannot add new components from other components, but only from factories
	void AddComponent(Component* component);

	const uint32_t m_GameObjectID;
	std::unordered_map<uint32_t, Component*> m_ComponentMap;
};

