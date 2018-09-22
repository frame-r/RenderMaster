#include "pch.h"
#include "Model.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

void Model::_update()
{
}

Model::Model(const std::vector<TResource<ICoreMesh>*>& meshes) : _meshes(meshes)
{
	//add_entry("meshes", &Model::_meshes);

	_pCore->AddUpdateCallback(std::bind(&Model::_update, this));
}

Model::~Model()
{
}

API Model::GetMesh(OUT ICoreMesh  **pMesh, uint idx)
{
	*pMesh = (*_meshes[idx]).get();
	return S_OK;
}

API Model::GetNumberOfMesh(OUT uint *number)
{
	*number = (uint)_meshes.size();
	return S_OK;
}

API Model::Free()
{
	// free each mesh

	for (TResource<ICoreMesh>* m : _meshes)
	{
		(*m).Release();
	}

	return S_OK;
}
