#include "stdafx.h"
#include "GameObjectRenderable.h"


GameObjectRenderable::GameObjectRenderable(const GameObjectRenderable& copy)
{
	memcpy_s(this, sizeof(GameObjectRenderable), &copy, sizeof(GameObjectRenderable));
	DebugLog("s", "Rudimentary game object copy.");
}

GameObjectRenderable::~GameObjectRenderable()
{
	DebugLog("s", "Destroying game object.");
}


