#pragma once
#include "Common.h"
#include "GameObject.h"
#include "ResourceManager.h"

class Model : public GameObjectBase<IModel>
{
	vector<IResource*> _meshes;
	AABB _aabb;

	void _update();
	void _recalculate_aabb();

public:

	Model(const vector<IResource*>& meshes);
	virtual ~Model();
		
	API GetMesh(OUT ICoreMesh **pMesh, uint idx) override;
	API GetNumberOfMesh(OUT uint *number) override;
	API GetAABB(OUT AABB *aabb) override;
	API Free() override;
};
