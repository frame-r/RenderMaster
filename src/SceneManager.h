#pragma once
#include "Common.h"
#include "Serialization.h"

class SceneManager : public ISceneManager
{
	bool _sceneLoaded = true;

	std::unique_ptr<GameObjectEvent> _gameObjectAddedEvent{new GameObjectEvent};
	std::unique_ptr<GameObjectEvent> _gameObjectDeleteEvent{new GameObjectEvent};

	tree<IGameObject*>::iterator gameobject_to_iterator(IGameObject *pGameObject);

	intrusive_ptr<ICamera> camera;

	friend void loadSceneManager(YAML::Node& n, SceneManager &sm);
	friend YAML::Emitter& operator<<(YAML::Emitter& out, const SceneManager& sm);

public:

	tree<IGameObject*> _gameobjects;

public:

	void Init();
	void Free();
	void addGameObject(IGameObject *go);

	// Scene
	API_RESULT SaveScene(const char *pRelativeScenePath) override;
	API_RESULT LoadScene(const char *pRelativeScenePath) override;
	API_RESULT CloseScene() override;

	// GameObjects manipulations
	API_RESULT GetNumberOfChilds(OUT uint *number, IGameObject *parent) override;
	API_RESULT GetChild(OUT IGameObject **pChildOut, IGameObject *parent, uint idx) override;
	API_RESULT FindChildById(OUT IGameObject **objectOut, uint id) override;

	// Default camera
	API_RESULT GetDefaultCamera(OUT ICamera **pCamera) override;
	
	// Events
	API_RESULT GetGameObjectAddedEvent(IGameObjectEvent** pEvent) override;
	API_RESULT GetDeleteGameObjectEvent(IGameObjectEvent** pEvent) override;

	// ISubSystem
	API_RESULT GetName(OUT const char **pName) override;
};

