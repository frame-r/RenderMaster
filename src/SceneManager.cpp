#include "SceneManager.h"
#include "Core.h"
#include "Camera.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

API SceneManager::GetDefaultCamera(ICamera*& pCamera)
{
	pCamera = _pCam;
	return S_OK;
}

API SceneManager::AddGameObject(IGameObject* pGameObject)
{
	_game_objects.push_back(pGameObject);
	return S_OK;
}

API SceneManager::GetGameObjectsNumber(uint& number)
{
	number = (uint)_game_objects.size();
	return S_OK;
}

API SceneManager::GetGameObject(IGameObject *&pGameObject, uint idx)
{
	pGameObject = _game_objects.at(idx);
	return S_OK;
}

SceneManager::SceneManager()
{
	_pCore->GetSubSystem((ISubSystem*&)pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
	
	_pCam = new Camera();
	pResMan->AddToList(_pCam);

}

void SceneManager::Free()
{
	DEBUG_LOG("SceneManager::Free(): objects total=%i", LOG_TYPE::NORMAL, _game_objects.size() + 1); // +1 camera
	#ifdef _DEBUG
		uint res_before = 0;
		pResMan->GetNumberOfResources(res_before);
	#endif

	for(IGameObject *go : _game_objects)
		go->Free();

	_pCam->Free();

	#ifdef _DEBUG
		uint res_after = 0;
		pResMan->GetNumberOfResources(res_after);
		DEBUG_LOG("SceneManager::Free(): resources deleted=%i", LOG_TYPE::NORMAL, res_before - res_after);
	#endif
}

API SceneManager::GetName(const char*& pName)
{
	pName = "SceneManager";
	return S_OK;
}
