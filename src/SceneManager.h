#pragma once
#include "Common.h"
#include "Events.h"
#include "tree.h"
#include "Serialization.h"
#include "GameObject.h"


class SceneManager : public ISceneManager, public Serializable<ISceneManager>
{	
	ICamera *_pCam{nullptr};
	IResourceManager *_pResMan{nullptr};
	std::unique_ptr<GameObjectEvent> _gameObjectAddedEvent{new GameObjectEvent};
	
public:

	SceneManager();

	tree<IGameObject*> _gameobjects;
	std::unordered_map<IGameObject*, tree<IGameObject*>::iterator_base> _go_to_it; // IGameObject* -> iterator in tree

	void Init();
	void Free();

	API SaveScene(const char *name) override;
	API GetDefaultCamera(OUT ICamera **pCamera) override;
	API AddRootGameObject(IGameObject *pGameObject) override;
	API GetChilds(OUT uint *number, IGameObject *parent) override;
	API GetChild(OUT IGameObject **pGameObject, IGameObject *parent, uint idx) override;
	
	// Events
	API GetGameObjectAddedEvent(IGameObjectEvent** pEvent) override;

	// ISubSystem
	API GetName(OUT const char **pName) override;
};

