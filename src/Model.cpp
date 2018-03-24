#include "Model.h"
#include "Core.h"

extern Core *_pCore;

Model::Model() : _pMesh(nullptr)
{
}

Model::Model(ICoreMesh* pMesh) : _pMesh(pMesh)
{
}


Model::~Model()
{
}

API Model::GetMesh(ICoreMesh*& pMesh, uint idx)
{
	return S_OK;
}

API Model::GetMeshesNumber(uint & number)
{
	return S_OK;
}

API Model::Free()
{
	IResourceManager *pResMan;
	_pCore->GetSubSystem((ISubSystem*&)pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	uint refNum;
	pResMan->GetRefNumber(this, refNum);

	if (refNum == 1)
	{
		pResMan->RemoveFromList(this);
		_pMesh->Free();
	}
	else
		pResMan->DecrementRef(this);

	return S_OK;
}

API Model::GetType(RES_TYPE & type)
{
	type = RES_TYPE::MODEL;
	return S_OK;
}

