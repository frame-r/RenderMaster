#include "SceneManager.h"
#include "Core.h"

extern Core *_pCore;

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

void SceneManager::Free()
{
	IResourceManager *pResMan;
	_pCore->GetSubSystem((ISubSystem*&)pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	for(IGameObject *go : _game_objects)
	{
		uint refNum;
		pResMan->GetRefNumber(go, refNum);

		if (refNum == 1)
		{
			pResMan->RemoveFromList(go);
			go->Free();
		}
		else
			pResMan->DecrementRef(go);
	}
}

API SceneManager::GetName(const char*& pName)
{
	pName = "SceneManager";
	return S_OK;
}