#pragma once
#include "Common.h"
#include "GameObject.h"
#include "ResourceManager.h"

class Model : public GameObjectBase<IModel>
{
	vector<IResource*> _meshes;

	void _update();

public:

	Model(const vector<IResource*>& meshes);
	virtual ~Model();
		
	API GetMesh(OUT ICoreMesh **pMesh, uint idx) override;
	API GetNumberOfMesh(OUT uint *number) override;
	API Free() override;
};
