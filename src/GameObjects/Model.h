#pragma once
#include "Common.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Serialization.h"

class Model : public BaseResource<GameObjectBase<IModel>>
{
	MeshPtr _mesh;
	AABB _aabb;
	string _matPath;
	IMaterial *_mat = nullptr;

public:
	Model(IMesh* mesh);
	Model(){}
	virtual ~Model(){};
		
	API_VOID GetMesh(OUT IMesh **pMesh) override;
	API_VOID SetMaterial(const char* path) override;
	API_VOID GetMaterial(OUT IMaterial **mat) override;
	API_VOID GetAABB(OUT AABB *aabb) override;
	API_VOID Copy(OUT IGameObject *dst) override;
};
