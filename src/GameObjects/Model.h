#pragma once
#include "Common.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Serialization.h"

class Model : public BaseResource<GameObjectBase<IModel>>
{
	MeshPtr _mesh;
	AABB _aabb;

public:
	Model(IMesh* mesh);
	Model(){}
	virtual ~Model(){};
		
	API_VOID GetMesh(OUT IMesh **pMesh) override;
	API_VOID GetAABB(OUT AABB *aabb) override;
	API_VOID Copy(OUT IModel *copy) override;
};
