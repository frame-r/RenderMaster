#pragma once
#include "Common.h"

class SceneManager : public ISceneManager
{
	std::vector<IGameObject*> _game_objects;
	ICamera *_pCam{ nullptr };

	IResourceManager *pResMan{nullptr};	

public:

	SceneManager();

	void Free();

	API GetDefaultCamera(ICamera *&pCamera) override;
	API AddGameObject(IGameObject *pGameObject) override;
	API GetGameObjectsNumber(uint& number) override;
	API GetGameObject(IGameObject *&pGameObject, uint idx) override;
	API GetName(const char *&pName) override;
};

