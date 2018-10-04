#pragma once
#include "Common.h"
#include "Events.h"
#include "Serialization.h"

class SceneManager : public ISceneManager, public Serializable<ISceneManager>
{	
	ResourcePtr<ICamera> _pCam;
	IResourceManager *_pResMan{nullptr};
	std::unique_ptr<ResourceEvent> _gameObjectAddedEvent{new ResourceEvent};

	tree<IResource*>::iterator find_(IResource *pGameObject);

public:

	tree<IResource*> _gameobjects;

public:

	SceneManager();

	void Init();
	void Free();

	API SaveScene(const char *name) override;
	API GetDefaultCamera(OUT ICamera **pCamera) override;
	API AddRootGameObject(IResource *pGameObject) override;
	API GetChilds(OUT uint *number, IResource *parent) override;
	API GetChild(OUT IResource **pGameObject, IResource *parent, uint idx) override;
	
	// Events
	API GetGameObjectAddedEvent(IResourceEvent** pEvent) override;

	// ISubSystem
	API GetName(OUT const char **pName) override;
};

