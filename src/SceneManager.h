#pragma once
#include "Common.h"

class SceneManager : public ISceneManager
{
	std::vector<IGameObject*> _game_objects;
	IResourceManager *pResMan{nullptr};
	ICamera *_pCam{nullptr};

public:

	SceneManager();

	void Free();

	API AddGameObject(IGameObject *pGameObject) override;
	API GetGameObjectsNumber(uint& number) override;
	API GetGameObject(IGameObject *&pGameObject, uint idx) override;
	API GetCamera(ICamera *&pCamera) override;
	API GetName(const char *&pName) override;
};

