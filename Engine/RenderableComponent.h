#pragma once
#include "Component.h"
#include <memory>
#include "EngineUtil.h"

constexpr uint32_t MAX_COUNT = 1000;

class GameObject;
class Model;
class Material;

template<typename = RenderableComponent>
class ComponentAllocator;

//typedef std::unique_ptr<ComponentAllocator<RenderableComponent>> UniqueAllocatorPtr;

//A future improvement could involve having a special renderable allocator which groups components by their material to batch things together 
class RenderableComponent : public Component
{
public:
	//Not a copyable type
	RenderableComponent(const RenderableComponent& c) = delete;
	RenderableComponent& operator=(const RenderableComponent&) = delete;
	virtual ~RenderableComponent();

	static Component* Create(TinyXML2::XMLElement* data);
	static void UpdateAll();

	//Not yet any implementation for Initialize
	virtual bool Initialize(TinyXML2::XMLElement* data) { return true; }
	//Later on there should be some managed way to allocate IDs to ComponentTypes, preferably by hashing the component name
	virtual uint32_t GetComponentID() const override { return 1; }

	inline Material* GetMaterial() const { return m_material; }
	inline Model* GetMesh() const { return m_mesh; }

	inline void SetMaterial(Material* const& m) { m_material = m; }
	inline void SetMesh(Model* const& m) { m_mesh = m; }

	//For testing purposes, renderablecomponent doesnt actually need an update function
	virtual void Update() override;

private:
	explicit RenderableComponent(Model* const& mesh, Material* const& material) : m_mesh(mesh), m_material(material) {}

	//should we have shared_ptrs and unique_ptrs for this? who cleans up the component allocator memory if it is static?
	//static ComponentAllocator<RenderableComponent>* m_RenderableComponentAllocator;
	static std::unique_ptr<ComponentAllocator<RenderableComponent>> m_RenderableComponentAllocator;
	Model* m_mesh;
	Material* m_material;
};

