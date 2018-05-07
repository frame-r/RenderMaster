#pragma once
#include "Common.h"
#include "GameObject.h"


class Model : public IModel
{
	std::vector<ICoreMesh *> _meshes;

public:

	Model(std::vector<ICoreMesh *>& meshes);
	~Model();
	
	API GetMesh(ICoreMesh*& pMesh, uint idx) override;
	API GetNumberOfMesh(uint& number) override;
	API Free() override;
	API GetType(RES_TYPE& type) override;

	IGAMEOBJECT_IMPLEMENTATION
};

