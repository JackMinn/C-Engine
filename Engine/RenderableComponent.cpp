#include "stdafx.h"
#include "RenderableComponent.h"
#include "GameObject.h"
#include "ComponentAllocator.h"
#include "TinyXML2.h"

std::unique_ptr<ComponentAllocator<RenderableComponent>> RenderableComponent::m_RenderableComponentAllocator = std::make_unique<ComponentAllocator<RenderableComponent>>(MAX_COUNT);
//new ComponentAllocator<RenderableComponent>(MAX_COUNT);

RenderableComponent::~RenderableComponent()
{
	//Deallocate this objects memory from the allocator of its type
	m_RenderableComponentAllocator->Deallocate(this);
	DebugLog("s", "Rendering component was deallocated.");
}

Component* RenderableComponent::Create(TinyXML2::XMLElement* data)
{
	const TinyXML2::XMLAttribute* material = data->FirstAttribute();
	DebugLog("ss", "Material name is: ", material->Name());
	DebugLog("ss", "Mesh name is: ", material->Next()->Name());
	//Run logic to get the variables for construction
	Material* temp = nullptr;
	Model* temp2 = nullptr;

	RenderableComponent* address = m_RenderableComponentAllocator->Allocate();
	new(address) RenderableComponent(temp2, temp);

	return address;
}

void RenderableComponent::UpdateAll()
{
	RenderableComponent* component = m_RenderableComponentAllocator->m_StartingAddress;
	for (uint32_t i = 0; i < m_RenderableComponentAllocator->m_CurrentCount; i++)
	{
		component->Update();
		component++;
	}
}

void RenderableComponent::Update() 
{
	DebugLog("si", "RenderableComponent is being updated. Owning object ID is: ", m_Owner->GetID());
}