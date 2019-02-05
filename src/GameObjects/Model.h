#pragma once
#include "Common.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Serialization.h"

class Model : public BaseResource<GameObjectBase<IModel>>
{
	vector<MeshPtr> _meshes;
	AABB _aabb;

	//friend YAML::Emitter& operator<<(YAML::Emitter& out, IResource* g);
	//friend void loadResource(YAML::Node& n, IGameObject *go);

public:

	Model(const vector<IMesh*>& meshes);
	Model(){}
	virtual ~Model(){};
		
	API_RESULT GetMesh(OUT IMesh **pMesh, uint idx) override;
	API_RESULT GetNumberOfMesh(OUT uint *number) override;
	API_RESULT GetAABB(OUT AABB *aabb) override;
	API_RESULT Copy(OUT IModel *copy) override;
};
