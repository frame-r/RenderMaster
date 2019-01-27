#pragma once
#include "Common.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Serialization.h"

class Model : public GameObjectBase<IModel>
{
	vector<MeshPtr> _meshes;
	AABB _aabb;

	//friend YAML::Emitter& operator<<(YAML::Emitter& out, IResource* g);
	//friend void loadResource(YAML::Node& n, IGameObject *go);

public:

	Model(const vector<IMesh*>& meshes);
	Model(){}
	virtual ~Model(){};
		
	API GetMesh(OUT IMesh **pMesh, uint idx) override;
	API GetNumberOfMesh(OUT uint *number) override;
	API GetAABB(OUT AABB *aabb) override;
	API Copy(OUT IModel *copy) override;

	RUNTIME_ONLY_RESOURCE_HEADER
};
