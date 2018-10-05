#pragma once
#include <mutex>

class Camera;

class Scene
{
public:
	Scene();
	~Scene();

private:
	std::shared_ptr<Camera> m_Camera;
};

