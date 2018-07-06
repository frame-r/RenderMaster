#include "Model.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

void Model::_update()
{
}

Model::Model(std::vector<ICoreMesh *>& meshes) : _meshes(meshes)
{
	add_entry("meshes", &Model::_meshes);

	_pCore->AddUpdateCallback(std::bind(&Model::_update, this));
}

Model::~Model()
{
}

API Model::GetMesh(OUT ICoreMesh  **pMesh, uint idx)
{
	*pMesh = _meshes.at(idx);
	return S_OK;
}

API Model::GetNumberOfMesh(OUT uint *number)
{
	*number = (uint)_meshes.size();
	return S_OK;
}

API Model::Free()
{
	for (auto *mesh : _meshes)
	{
		IResourceManager *pResMan;
		_pCore->GetSubSystem((ISubSystem**)&pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

		uint refNum;
		if (SUCCEEDED(pResMan->GetRefNumber(&refNum, mesh))) // if not yet deleted
			mesh->Free();
	}
	standard_free_and_delete(this, std::function<void()>(), _pCore);

	return S_OK;
}

API Model::GetType(OUT RES_TYPE *type)
{
	*type = RES_TYPE::MODEL;
	return S_OK;
}


