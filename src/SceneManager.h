#pragma once
#include "Common.h"
#include "Events.h"
#include "tree.h"
#include "Serialization.h"
#include "GameObject.h"
#include "ResourceManager.h"


class SceneManager : public ISceneManager, public Serializable<ISceneManager>
{	
	TResource<ICamera> *_pCam = nullptr;
	IResourceManager *_pResMan{nullptr};
	std::unique_ptr<GameObjectEvent> _gameObjectAddedEvent{new GameObjectEvent};

public:

	tree<IGameObject*> _gameobjects;

	//
	// IGameObject* -> iterator
	// For fast searching childs, parents through tree...
	std::unordered_map<IGameObject*, tree<IGameObject*>::iterator_base> _go_to_it;

public:

	SceneManager();

	void Init();
	void Free();

	API SaveScene(const char *name) override;
	API GetDefaultCamera(OUT ICamera **pCamera) override;
	API AddRootGameObject(IResource *pGameObject) override;
	API GetChilds(OUT uint *number, IGameObject *parent) override;
	API GetChild(OUT IGameObject **pGameObject, IGameObject *parent, uint idx) override;
	
	// Events
	API GetGameObjectAddedEvent(IGameObjectEvent** pEvent) override;

	// ISubSystem
	API GetName(OUT const char **pName) override;
};

