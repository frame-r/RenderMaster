#include "pch.h"
#include "SceneManager.h"
#include "Core.h"
#include "Camera.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

tree<IResource*>::iterator SceneManager::gameobject_to_iterator(IResource * pGameObject)
{
	for (auto it = _gameobjects.begin(); it != _gameobjects.end(); ++it)
	{
		IResource* res = *it;
		if (res == pGameObject)
		{
			return it;
		}
	}
	return _gameobjects.end();
}

SceneManager::SceneManager()
{
}

API SceneManager::SaveScene(const char *name)
{
	IFileSystem *fs;
	_pCore->GetSubSystem((ISubSystem**)&fs, SUBSYSTEM_TYPE::FILESYSTEM);
	
	YAML::Emitter out;
	out << YAML::Block << *this;
	
	IFile *f = nullptr;
	fs->OpenFile(&f, name, FILE_OPEN_MODE::WRITE | FILE_OPEN_MODE::BINARY);
	
	f->WriteStr(out.c_str());
	
	f->CloseAndFree();
	
	_sceneLoaded = true;
	
	LOG_FORMATTED("Scene saved to: %s\n", name);

	return S_OK;
}

API SceneManager::CloseScene()
{
	for (IResource *obj : _gameobjects)
	{
		_gameObjectDeleteEvent->Fire(obj);
		obj->Release();
	}

	_gameobjects.clear();

	LOG("Scene closed");

	return S_OK;
}

API SceneManager::GetDefaultCamera(OUT ICamera **pCamera)
{
	for (IResource *obj : _gameobjects)
	{
		RES_TYPE type;
		obj->GetType(&type);
		if (type == RES_TYPE::CAMERA)
		{
			obj->GetPointer((void**)pCamera);
			return S_OK;
		}
	}
	*pCamera = nullptr;
	return S_OK;
}

API SceneManager::AddRootGameObject(IResource* pGameObject)
{
	tree<IResource*>::iterator top = _gameobjects.begin();
	pGameObject->AddRef();
	auto it = _gameobjects.insert(top, pGameObject);
	_gameObjectAddedEvent->Fire(pGameObject);
	return S_OK;
}

API SceneManager::GetChilds(OUT uint *number, IResource *parent)
{
	if (parent)
	{
		auto it = gameobject_to_iterator(parent);

		if (it == _gameobjects.end())
		{
			*number = 0;
			return E_FAIL;
		}

		*number = (uint)_gameobjects.number_of_children(it);
		return S_OK;
	}
	else
		*number = (uint)_gameobjects.number_of_siblings(_gameobjects.begin()) + 1;
	
	return S_OK;
}

API SceneManager::GetChild(OUT IResource **pGameObject, IResource *parent, uint idx)
{
	uint number;
	GetChilds(&number, parent);
	assert(idx < number && "SceneManager::GetChild() idx is out of range");

	if (parent)
	{
		auto it = gameobject_to_iterator(parent);

		if (it == _gameobjects.end())
		{
			*pGameObject = nullptr;
			return E_FAIL;
		}

		*pGameObject = *_gameobjects.child(it, idx);
	}else
	{
		*pGameObject = *_gameobjects.sibling(_gameobjects.begin(), idx);
	}

	return S_OK;
}

void SceneManager::Init()
{
	IResourceManager *_pResMan = getResourceManager(_pCore);
	ResourcePtr<ICamera> camera = _pResMan->createCamera();
	AddRootGameObject(camera.getResource());
	LOG("Scene Manager initialized");
}

void SceneManager::Free()
{
	IResourceManager *_pResMan = getResourceManager(_pCore);

	DEBUG_LOG("SceneManager::Free(): objects to delete=%i", LOG_TYPE::NORMAL, _gameobjects.size());
	#ifdef _DEBUG
		uint res_before = 0;
		_pResMan->GetNumberOfResources(&res_before);
	#endif

	CloseScene();

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

API SceneManager::GetGameObjectAddedEvent(IResourceEvent** pEvent)
{
	*pEvent = _gameObjectAddedEvent.get();
	return S_OK;
}

API SceneManager::GetDeleteGameObjectEvent(IResourceEvent ** pEvent)
{
	*pEvent = _gameObjectDeleteEvent.get();
	return S_OK;
}
