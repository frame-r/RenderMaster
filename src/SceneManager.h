#pragma once
#include "Common.h"
#include "Serialization.h"

class SceneManager : public ISceneManager
{
	bool _sceneLoaded = true;

	std::unique_ptr<GameObjectEvent> _gameObjectAddedEvent{new GameObjectEvent};
	std::unique_ptr<GameObjectEvent> _gameObjectDeleteEvent{new GameObjectEvent};

	tree<IGameObject*>::iterator gameobject_to_iterator(IGameObject *pGameObject);

	WRL::ComPtr<ICamera> camera;

	friend void loadSceneManager(YAML::Node& n, SceneManager &sm);
	friend YAML::Emitter& operator<<(YAML::Emitter& out, const SceneManager& sm);

public:

	tree<IGameObject*> _gameobjects;

public:

	void Init();
	void Free();
	void addGameObject(IGameObject *go);

	// Scene
	API SaveScene(const char *pRelativeScenePath) override;
	API LoadScene(const char *pRelativeScenePath) override;
	API CloseScene() override;

	// GameObjects manipulations
	API GetNumberOfChilds(OUT uint *number, IGameObject *parent) override;
	API GetChild(OUT IGameObject **pChildOut, IGameObject *parent, uint idx) override;
	API FindChildById(OUT IGameObject **objectOut, uint id) override;

	// Default camera
	API GetDefaultCamera(OUT ICamera **pCamera) override;
	
	// Events
	API GetGameObjectAddedEvent(IGameObjectEvent** pEvent) override;
	API GetDeleteGameObjectEvent(IGameObjectEvent** pEvent) override;

	// ISubSystem
	API GetName(OUT const char **pName) override;
};

