#pragma once
#include "Common.h"
#include "Events.h"
#include "Serialization.h"

class SceneManager : public ISceneManager
{
	bool _sceneLoaded = false;

	std::unique_ptr<ResourceEvent> _gameObjectAddedEvent{new ResourceEvent};
	std::unique_ptr<ResourceEvent> _gameObjectDeleteEvent{new ResourceEvent};

	tree<IResource*>::iterator gameobject_to_iterator(IResource *pGameObject);

	friend void loadSceneManager(YAML::Node& n, SceneManager &sm);
	friend YAML::Emitter& operator<<(YAML::Emitter& out, const SceneManager& sm);

public:

	tree<IResource*> _gameobjects;

public:

	SceneManager();

	void Init();
	void Free();

	API SaveScene(const char *pRelativeScenePath) override;
	API LoadScene(const char *pRelativeScenePath) override;
	API CloseScene() override;
	API GetDefaultCamera(OUT ICamera **pCamera) override;
	API AddRootGameObject(IResource *pGameObject) override;
	API GetNumberOfChilds(OUT uint *number, IResource *parent) override;
	API GetChild(OUT IResource **pGameObject, IResource *parent, uint idx) override;
	
	// Events
	API GetGameObjectAddedEvent(IResourceEvent** pEvent) override;
	API GetDeleteGameObjectEvent(IResourceEvent** pEvent) override;

	// ISubSystem
	API GetName(OUT const char **pName) override;
};

