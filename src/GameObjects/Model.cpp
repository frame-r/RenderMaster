#include "Pch.h"
#include "Model.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

RUNTIME_ONLY_RESOURCE_IMPLEMENTATION(Model, _pCore, RemoveRuntimeGameObject)

Model::Model(const vector<IMesh*>& meshes) 
{
	for (IMesh *m : meshes)
		_meshes.push_back(MeshPtr(m));
	//_pCore->AddUpdateCallback(std::bind(&Model::_update, this));
}

API Model::GetMesh(OUT IMesh **pMesh, uint idx)
{
	MeshPtr& m = _meshes[idx];
	*pMesh = m.Get();
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
	GameObjectBase<IModel>::Copy(copy);

	Model *copyModel = static_cast<Model*>(copy);
	copyModel->_meshes = _meshes;
	copyModel->_aabb = _aabb;

	return S_OK;
}
