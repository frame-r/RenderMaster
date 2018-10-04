#include "pch.h"
#include "Model.h"
#include "Core.h"
#include "Serialization.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

void Model::_update()
{
}

Model::Model(const vector<IResource*>& meshes) : _meshes(meshes)
{
	add_entry("meshes", &Model::_meshes);

	for (IResource *m : _meshes)
		m->AddRef();

	_pCore->AddUpdateCallback(std::bind(&Model::_update, this));
}

Model::~Model()
{
}

API Model::GetMesh(OUT ICoreMesh  **pMesh, uint idx)
{
	IResource * res = _meshes[idx];
	ICoreMesh *mesh;
	res->GetPointer((void**)&mesh);
	*pMesh = mesh;
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
	for (IResource *m : _meshes)
		m->Release();

	return S_OK;
}
