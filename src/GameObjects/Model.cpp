#include "Pch.h"
#include "Model.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

Model::Model(IMesh* mesh) 
{
	_mesh = MeshPtr(mesh);
}

API_VOID Model::GetMesh(OUT IMesh **pMesh)
{
	*pMesh = _mesh.Get();
}

API_VOID Model::GetAABB(OUT AABB *aabb)
{
	const static AABB _unitAABB = {-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f};
	*aabb = _unitAABB;
}

API_VOID Model::Copy(OUT IGameObject *copy)
{
	GameObjectBase<IModel>::Copy(copy);

	Model *copyModel = static_cast<Model*>(copy);
	copyModel->_mesh = _mesh;
	copyModel->_aabb = _aabb;
}
