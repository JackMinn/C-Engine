#pragma once

#include <assert.h>
#include <stdint.h>
#include "EngineUtil.h"

template<typename ComponentType>
class ComponentAllocator
{
public:
	ComponentAllocator(uint32_t maxCount) : m_MaxCount(maxCount), m_CurrentCount(0), m_StartingAddress(reinterpret_cast<ComponentType*>(malloc(sizeof(ComponentType) * maxCount))), m_LastAddress(m_StartingAddress)
	{ 
		//m_StartingAddress = reinterpret_cast<ComponentType*>(malloc(sizeof(ComponentType) * maxCount)); 
		//m_LastAddress = m_StartingAddress;
	}
	//Not a copyable type
	ComponentAllocator(const ComponentAllocator& c) = delete;
	ComponentAllocator& operator=(const ComponentAllocator&) = delete;
	~ComponentAllocator() { assert(m_CurrentCount == 0 && "Component allocator being freed with a non-zero count."); DebugLog("s", "Allocator is being destroyed"); free(m_StartingAddress); }

	inline ComponentType* Allocate()
	{
		assert(m_CurrentCount <= m_MaxCount && "Current component count exceeds maximum possible components of this type.");

		if (m_CurrentCount >= m_MaxCount) 
		{
			return nullptr;
		}

		ComponentType* address = m_LastAddress;
		m_LastAddress++;
		m_CurrentCount++;
		return address;
	}

	//what happens to objects which had a pointer to the component that just got relocated? we now have a serious problem
	//this will corrupt the component map of the gameobject, as the pointer owned now points to invalid memory
	//perhaps keeping these components contiguous in memory is not the best solution, but rather it should be a managed list or byte padding to specify free/inuse 
	//(1 byte might cause alignemnt problems)
	//consider a stack of pointers to all the free memory addresses avaiable in your allocator... however iterating over them might not be tedious
	inline void Deallocate(ComponentType* address)
	{
		memcpy(address, --m_LastAddress, sizeof(ComponentType));
		m_CurrentCount--;
	}

private:
	friend ComponentType;

	uint32_t m_CurrentCount;
	uint32_t m_MaxCount;
	ComponentType* const m_StartingAddress;
	ComponentType* m_LastAddress;

};

