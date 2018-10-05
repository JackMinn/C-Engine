#include "stdafx.h"
#include "GameObject.h"
#include "Component.h"

void GameObject::Destroy()
{
	//Need to manually call destructors on components in order to deallocate their memory from the allocator pool.
	for (auto it = m_ComponentMap.begin(); it != m_ComponentMap.end(); ++it)
	{
		it->second->~Component();
	}
	m_ComponentMap.clear();
}

template<typename ComponentType>
std::weak_ptr<ComponentType> GameObject::GetComponent(const uint32_t& id)
{
	auto componentIterator = m_ComponentMap.find(id);
	if (componentIterator != m_ComponentMap.end())
	{
		std::weak_ptr<ComponentType> component = componentIterator->second;
		return component;
	}
	else
	{
		return std::weak_ptr<ComponentType>();
	}
}

void GameObject::AddComponent(Component *component)
{
	m_ComponentMap.insert({ component->GetComponentID(), component });
}



