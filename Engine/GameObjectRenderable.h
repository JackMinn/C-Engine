#pragma once
#include "Model.h"
#include "Material.h"
#include <DirectXMath.h>
#include "EngineUtil.h"
#include "Transform.h"

class GameObjectRenderable
{
public:
	explicit GameObjectRenderable(Model* const& mesh, Material* const& material) : m_mesh(mesh), m_material(material), m_transform(Transform()) {}
	explicit GameObjectRenderable(Model* const& mesh, Material* const& material, const DirectX::SimpleMath::Vector3& v , const DirectX::SimpleMath::Quaternion& q) :
						m_mesh(mesh), m_material(material), m_transform(Transform(v, q, DirectX::SimpleMath::Vector3::One)) {}
	GameObjectRenderable(const GameObjectRenderable&);
	~GameObjectRenderable();

	inline Transform& GetTransform() { return m_transform; }
	inline Material* GetMaterial() const { return m_material; }
	inline Model* GetMesh() const { return m_mesh; }

	inline void SetMaterial(Material* const& m) { m_material = m; }
	inline void SetMesh(Model* const& m) { m_mesh = m; }

private:
	Model* m_mesh;
	Material* m_material;
	Transform m_transform;

};

