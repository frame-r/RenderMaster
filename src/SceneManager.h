#pragma once
#include "Common.h"

class SceneManager : public ISceneManager
{
	std::vector<IGameObject*> _game_objects;

public:

	void Free();

	API AddGameObject(IGameObject *pGameObject);
	API GetGameObjectsNumber(uint& number);
	API GetGameObject(IGameObject *&pGameObject, uint idx);
	API GetName(const char *&pName);
};

