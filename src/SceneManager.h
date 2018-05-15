#pragma once
#include "Common.h"

class SceneManager final : public ISceneManager
{
	std::vector<IGameObject*> _game_objects;
	ICamera *_pCam{ nullptr };

	IResourceManager *pResMan{nullptr};	

public:

	SceneManager();

	void Init();
	void Free();

	API GetDefaultCamera(OUT ICamera **pCamera) override;
	API AddGameObject(IGameObject *pGameObject) override;
	API GetGameObjectsNumber(OUT uint *number) override;
	API GetGameObject(OUT IGameObject **pGameObject, uint idx) override;
	API GetName(OUT const char **pName) override;
};

