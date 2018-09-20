#include "SceneManager.h"
#include "Core.h"
#include "Camera.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

SceneManager::SceneManager()
{
	_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	add_entry("gameobjects", &SceneManager::_gameobjects);
}

API SceneManager::SaveScene(const char *name)
{
	IFileSystem *fs;
	_pCore->GetSubSystem((ISubSystem**)&fs, SUBSYSTEM_TYPE::FILESYSTEM);

	std::ostringstream out;
	dynamic_cast<SceneManager*>(this)->serialize(out, 0);

	IFile *f = nullptr;
	fs->OpenFile(&f, name, FILE_OPEN_MODE::WRITE | FILE_OPEN_MODE::BINARY);

	f->WriteStr(out.str().c_str());

	f->CloseAndFree();

	LOG_FORMATTED("Scene saved to %s\n", name);

	return S_OK;
}

API SceneManager::GetDefaultCamera(OUT ICamera **pCamera)
{
	*pCamera = _pCam.get();
	return S_OK;
}

API SceneManager::AddRootGameObject(IResource* pGameObject)
{
	IGameObject *go = nullptr;
	pGameObject->GetPointer((void**)&go);
	tree<IGameObject*>::iterator top = _gameobjects.begin();
	auto it = _gameobjects.insert(top, go);
	_go_to_it[go] = it;
	_gameObjectAddedEvent->Fire(go);
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

void SceneManager::Init()
{
	_pCam = _pResMan->createCamera();
	AddRootGameObject(_pCam.getResource());
	LOG("Scene Manager initialized");
}

void SceneManager::Free()
{
	DEBUG_LOG("SceneManager::Free(): objects to delete=%i", LOG_TYPE::NORMAL, _gameobjects.size());
	#ifdef _DEBUG
		uint res_before = 0;
		_pResMan->GetNumberOfResources(&res_before);
	#endif

	_pCam.reset();
			

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
