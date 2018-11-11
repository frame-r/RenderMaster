#pragma once
#include "Common.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Serialization.h"

class Model : public GameObjectBase<IModel>
{
	vector<IResource*> _meshes;
	AABB _aabb;

	void _update();
	void _recalculate_aabb();

	friend YAML::Emitter& operator<<(YAML::Emitter& out, IResource* g);
	friend void loadResource(YAML::Node& n, IGameObject *go);

public:

	Model(const vector<IResource*>& meshes);
	Model(){}
	virtual ~Model();
		
	API GetMesh(OUT ICoreMesh **pMesh, uint idx) override;
	API GetNumberOfMesh(OUT uint *number) override;
	API GetAABB(OUT AABB *aabb) override;
	API Copy(OUT IModel *copy) override;
	API Free() override;
};
