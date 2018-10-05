#pragma once
#include <memory>

namespace TinyXML2
{
	class XMLElement;
}
class GameObject;

class Component
{
public:
	Component() = default;
	//Not a copyable type
	Component(const Component& c) = delete;
	Component& operator=(const Component&) = delete;

	virtual ~Component() { m_Owner.reset(); }

	virtual void Update() {}
	virtual void LateUpdate() {}

	//Every component must implement these functions
	virtual bool Initialize(TinyXML2::XMLElement* data) = 0;
	virtual uint32_t GetComponentID() const = 0;

protected:
	std::shared_ptr<GameObject> m_Owner;

private:
	friend class GameObjectFactory;

	//Private because this function is only called on the Component base class by the Factory, the component itself should not be allowed to change owner
	inline void SetOwner(std::shared_ptr<GameObject> owner) { m_Owner = owner; }
};

