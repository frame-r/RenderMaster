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

API SceneManager::AddGameObject(IGameObject* pGameObject)
{
	_game_objects.push_back(pGameObject);
	return S_OK;
}

API SceneManager::GetGameObjectsNumber(OUT uint *number)
{
	*number = (uint)_game_objects.size();
	return S_OK;
}

API SceneManager::GetGameObject(OUT IGameObject **pGameObject, uint idx)
{
	*pGameObject = _game_objects.at(idx);
	return S_OK;
}

SceneManager::SceneManager()
{
	_pCore->GetSubSystem((ISubSystem**)&pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
}

void SceneManager::Init()
{
	_pCam = new Camera();
	pResMan->AddToList(_pCam);
	AddGameObject(_pCam);
	LOG("Scene Manager initialized");
}

void SceneManager::Free()
{
	DEBUG_LOG("SceneManager::Free(): objects to delete=%i", LOG_TYPE::NORMAL, _game_objects.size());
	#ifdef _DEBUG
		uint res_before = 0;
		pResMan->GetNumberOfResources(&res_before);
	#endif

	for(IGameObject *go : _game_objects)
		go->Free();

	_game_objects.clear();

	#ifdef _DEBUG
		uint res_after = 0;
		pResMan->GetNumberOfResources(&res_after);
		DEBUG_LOG("SceneManager::Free(): objects deleted=%i", LOG_TYPE::NORMAL, res_before - res_after);
	#endif
}

API SceneManager::GetName(OUT const char **pName)
{
	*pName = "SceneManager";
	return S_OK;
}
