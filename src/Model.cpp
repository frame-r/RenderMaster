#include "Model.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

Model::Model(std::vector<ICoreMesh *>& meshes) : _meshes(meshes)
{
}

Model::~Model()
{
}

API Model::GetMesh(ICoreMesh*& pMesh, uint idx)
{
	pMesh = _meshes.at(idx);
	return S_OK;
}

API Model::GetNumberOfMesh(uint & number)
{
	number = (uint)_meshes.size();
	return S_OK;
}

API Model::Free()
{
	IResourceManager *pResMan;
	_pCore->GetSubSystem((ISubSystem*&)pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	for (ICoreMesh *pMesh : _meshes)
	{
		uint refNum;
		pResMan->GetRefNumber(this, refNum);

		if (refNum == 1)
		{
			pResMan->RemoveFromList(this);
			pMesh->Free();
		}
		else
			pResMan->DecrementRef(this);
	}

	return S_OK;
}

API Model::GetType(RES_TYPE & type)
{
	type = RES_TYPE::MODEL;
	return S_OK;
}


