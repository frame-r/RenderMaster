#pragma once
#include "Common.h"
#include "GameObject.h"


class Model : public GameObjectBase<IModel>
{
	std::vector<ICoreMesh *> _meshes;

public:

	Model(std::vector<ICoreMesh *>& meshes);
	~Model();
	
	API GetMesh(OUT ICoreMesh **pMesh, uint idx) override;
	API GetNumberOfMesh(OUT uint *number) override;
	API Free() override;
	API GetType(OUT RES_TYPE *type) override;
};

