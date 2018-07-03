#include "SceneManager.h"
#include "Core.h"
#include "Camera.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

API SceneManager::SaveScene(const char *name)
{
	return S_OK;
}

API SceneManager::GetDefaultCamera(OUT ICamera **pCamera)
{
	*pCamera = _pCam;
	return S_OK;
}

API SceneManager::AddRootGameObject(IGameObject* pGameObject)
{
	tree<IGameObject*>::iterator top = _gameobjects.begin();
	auto it = _gameobjects.insert(top, pGameObject);
	_go_to_it[pGameObject] = it;
	_gameObjectAddedEvent->Fire(pGameObject);
	return S_OK;
}

API SceneManager::GetChilds(OUT uint *number, IGameObject *parent)
{
	if (parent)
	{
		tree<IGameObject*>::iterator_base it = _go_to_it[parent];
		*number = (uint)_gameobjects.number_of_children(it);
	}
	else
		*number = (uint)_gameobjects.number_of_siblings(_gameobjects.begin()) + 1;
	
	return S_OK;
}

API SceneManager::GetChild(OUT IGameObject **pGameObject, IGameObject *parent, uint idx)
{
	uint number;
	GetChilds(&number, parent);
	assert(idx < number && "SceneManager::GetChild() idx is out of range");

	if (parent)
	{
		auto it = _go_to_it[parent];
		*pGameObject = *_gameobjects.child(it, idx);
	}else
	{
		*pGameObject = *_gameobjects.sibling(_gameobjects.begin(), idx);
	}

	return S_OK;
}

SceneManager::SceneManager()
{
	_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	add_entry("gameobjects", &SceneManager::_gameobjects);
}

void SceneManager::Init()
{
	_pCam = new Camera();
	_pResMan->AddToList(_pCam);
	AddRootGameObject(_pCam);
	LOG("Scene Manager initialized");
}

void SceneManager::Free()
{
	IFileSystem *fs;
	_pCore->GetSubSystem((ISubSystem**)&fs, SUBSYSTEM_TYPE::FILESYSTEM);
	IFile *f;
	fs->OpenFile(&f, "scene.yaml", FILE_OPEN_MODE::WRITE);
	dynamic_cast<SceneManager*>(this)->print(f, 0);
	f->CloseAndFree();

	DEBUG_LOG("SceneManager::Free(): objects to delete=%i", LOG_TYPE::NORMAL, _gameobjects.size());
	#ifdef _DEBUG
		uint res_before = 0;
		_pResMan->GetNumberOfResources(&res_before);
	#endif

	for (tree<IGameObject*>::sibling_iterator sl_it = _gameobjects.begin(); sl_it != _gameobjects.end(); sl_it++)
	{
		(*sl_it)->Free();
	}

	#ifdef _DEBUG
		uint res_after = 0;
		_pResMan->GetNumberOfResources(&res_after);
		DEBUG_LOG("SceneManager::Free(): objects deleted=%i", LOG_TYPE::NORMAL, res_before - res_after);
	#endif
}

API SceneManager::GetName(OUT const char **pName)
{
	*pName = "SceneManager";
	return S_OK;
}

API SceneManager::GetGameObjectAddedEvent(IGameObjectEvent** pEvent)
{
	*pEvent = _gameObjectAddedEvent.get();
	return S_OK;
}
