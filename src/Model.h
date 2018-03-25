#pragma once
#include "Common.h"


class Model : public IModel
{
	ICoreMesh *_pMesh;

public:

	Model(ICoreMesh* pMesh);
	~Model();
	
	API GetMesh(ICoreMesh*& pMesh, uint idx) override;
	API GetMeshesNumber(uint& number) override;
	API Free() override;
	API GetType(RES_TYPE& type) override;
};

