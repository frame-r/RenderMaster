#include "ResourceManager.h"
#include "Core.h"

extern Core *_pCore;

ResourceManager::ResourceManager()
{
	InitializeCriticalSection(&_cs);

	_pCore->Log("ResourceManager initalized");
}

ResourceManager::~ResourceManager()
{
	for each(Mesh *m in _mesh_vec)
		m->Free();
}

API ResourceManager::GetName(const char *& pTxt)
{
	pTxt = "ResourceManager";

	return S_OK;
}

API ResourceManager::LoadMesh(IMesh *& pMesh, const char * pFileName, IProgressSubscriber *pPregress)
{
	for (int i = 0; i < 10; i++)
	{
		Sleep(200);

		if (pPregress != nullptr)
			pPregress->ProgressChanged(i * 10);
	}	

	Mesh *mesh = new Mesh;
	pMesh = mesh;


	EnterCriticalSection(&_cs);

	_mesh_vec.push_back(mesh);

	LeaveCriticalSection(&_cs);

	return S_OK;
}
