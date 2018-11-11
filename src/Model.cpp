#include "pch.h"
#include "Model.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

Model::Model(const vector<IResource*>& meshes) : _meshes(meshes)
{
	for (IResource *m : _meshes)
		m->AddRef();

	_pCore->AddUpdateCallback(std::bind(&Model::_update, this));
}

Model::~Model()
{
}

void Model::_update()
{
}

void Model::_recalculate_aabb()
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

API Model::GetAABB(OUT AABB *aabb)
{
	const static AABB _unitAABB = {-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f};
	*aabb = _unitAABB;
	return S_OK;
}

API Model::Copy(OUT IModel *copy)
{
	IGameObject *copyGO = copy;
	GameObjectBase<IModel>::Copy(copy);

	Model *copyModel = static_cast<Model*>(copy);

	copyModel->_meshes = _meshes;
	for(IResource * m: _meshes)
		m->AddRef();

	copyModel->_aabb = _aabb;

	return S_OK;
}

API Model::Free()
{
	// free each mesh
	for (IResource *m : _meshes)
		m->Release();

	IResourceManager *rm = getResourceManager(_pCore);
	rm->RemoveModel(this);

	return S_OK;
}
