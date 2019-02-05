#include "Pch.h"
#include "SceneManager.h"
#include "Core.h"
#include "Camera.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

tree<IGameObject*>::iterator SceneManager::gameobject_to_iterator(IGameObject *pGameObject)
{
	for (auto it = _gameobjects.begin(); it != _gameobjects.end(); ++it)
	{
		IGameObject* res = *it;
		if (res == pGameObject)
			return it;
	}
	return _gameobjects.end();
}

API_RESULT SceneManager::SaveScene(const char *pRelativeScenePath)
{
	IFileSystem *fs;
	_pCore->GetSubSystem((ISubSystem**)&fs, SUBSYSTEM_TYPE::FILESYSTEM);

	// TODO

	//YAML::Emitter out;
	//out << YAML::Block << *this;
	//
	//IFile *f = nullptr;
	//fs->OpenFile(&f, pRelativeScenePath, FILE_OPEN_MODE::WRITE | FILE_OPEN_MODE::BINARY);
	//
	//f->WriteStr(out.c_str());
	//
	//f->CloseAndFree();
	
	LOG_FORMATTED("Scene saved to: %s\n", pRelativeScenePath);

	return S_OK;
}

API_RESULT SceneManager::LoadScene(const char *pRelativeScenePath)
{
	// TODO

	//if (_sceneLoaded)
	//{
	//	LOG_WARNING("Closing scene...");
	//	CloseScene();
	//}
	//IFileSystem *fs;
	//_pCore->GetSubSystem((ISubSystem**)&fs, SUBSYSTEM_TYPE::FILESYSTEM);
	//
	//YAML::Emitter out;
	//out << YAML::Block << *this;
	//
	//IFile *f = nullptr;
	//fs->OpenFile(&f, pRelativeScenePath, FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
	//
	//uint fileSize;
	//f->FileSize(&fileSize);
	//
	//char *tmp = new char[fileSize + 1];
	//tmp[fileSize] = '\0';
	//
	//f->Read((uint8 *)tmp, fileSize);
	//f->CloseAndFree();
	//
	//YAML::Node model_yaml = YAML::Load(tmp);
	//auto t = model_yaml.Type();	
	//
	//loadSceneManager(model_yaml, *this);
	//
	//_sceneLoaded = true;
	//
	//delete tmp;

	return S_OK;
}

API_RESULT SceneManager::CloseScene()
{
	for (IGameObject *obj : _gameobjects)
		_gameObjectDeleteEvent->Fire(obj);

	_gameobjects.clear();

	_sceneLoaded = false;

	LOG("Scene closed");

	return S_OK;
}

API_RESULT SceneManager::GetDefaultCamera(OUT ICamera **pCamera)
{
	for (IGameObject *obj : _gameobjects)
	{
		ICamera *cam = dynamic_cast<ICamera*>(obj);
		if (cam)
		{
			*pCamera = cam;
			return S_OK;
		}
	}
	*pCamera = nullptr;
	return S_OK;
}

API_RESULT SceneManager::GetNumberOfChilds(OUT uint *number, IGameObject *parent)
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

API_RESULT SceneManager::GetChild(OUT IGameObject **pGameObject, IGameObject *parent, uint idx)
{
	uint number;
	GetNumberOfChilds(&number, parent);
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

API_RESULT SceneManager::FindChildById(OUT IGameObject **objectOut, uint idIn)
{
	for (auto it = _gameobjects.begin(); it != _gameobjects.end(); ++it)
	{
		IGameObject* res = *it;
		uint id;

		res->GetID(&id);

		if (id == idIn)
		{
			*objectOut = res;
			return S_OK;
		}
	}

	*objectOut = nullptr;

	return S_OK;
}

void SceneManager::Init()
{
	IResourceManager *rm = getResourceManager(_pCore);
	camera = rm->CreateCamera();
	LOG("Scene Manager initialized");
}

void SceneManager::Free()
{
	camera.Reset();

	for (auto it = _gameobjects.begin(); it != _gameobjects.end(); ++it)
	{
		IGameObject* res = *it;
		res->Release();
	}
	_gameobjects.clear();
}

void SceneManager::addGameObject(IGameObject *go)
{
	go->AddRef();
	tree<IGameObject*>::iterator top = _gameobjects.begin();
	auto it = _gameobjects.insert(top, go);
	_gameObjectAddedEvent->Fire(go);
}

API_RESULT SceneManager::GetName(OUT const char **pName)
{
	*pName = "SceneManager";
	return S_OK;
}

API_RESULT SceneManager::GetGameObjectAddedEvent(IGameObjectEvent** pEvent)
{
	*pEvent = _gameObjectAddedEvent.get();
	return S_OK;
}

API_RESULT SceneManager::GetDeleteGameObjectEvent(IGameObjectEvent ** pEvent)
{
	*pEvent = _gameObjectDeleteEvent.get();
	return S_OK;
}
