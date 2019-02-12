#include "Pch.h"
#include "Model.h"
#include "Core.h"
#include "ResourceManager.h"

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

API_VOID Model::SetMaterial(const char * path)
{
	_matPath = path;

	IResourceManager *iresMan = getResourceManager(_pCore);
	ResourceManager *resMan = static_cast<ResourceManager*>(iresMan);

	resMan->_FindMaterial(&_mat, path);
}

API_VOID Model::GetMaterial(OUT IMaterial ** mat)
{
	*mat = _mat;
}

API_VOID Model::GetAABB(OUT AABB *aabb)
{
	const static AABB _unitAABB = {-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f};
	*aabb = _unitAABB;
}

API_VOID Model::Copy(OUT IGameObject *dst)
{
	GameObjectBase<IModel>::Copy(dst);

	Model *modelDst = static_cast<Model*>(dst);
	modelDst->_mesh = _mesh;
	modelDst->_aabb = _aabb;
	modelDst->_mat = _mat;
	modelDst->_matPath = _matPath;
}
