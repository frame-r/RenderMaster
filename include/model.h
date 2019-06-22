#pragma once
#include "gameobject.h"
#include "mesh.h"
#include "resource_manager.h"


class Model final : public GameObject
{
	ManagedPtr<Mesh> mesh_;
	vec3 meshCeneter;
	Material *mat_{nullptr};

protected:
	virtual void Copy(GameObject *original) override;
	virtual void Serialize(void *yaml) override;
	virtual void Deserialize(void *yaml) override;

public:
	Model();
	Model(ManagedPtr<Mesh> mesh);
	virtual ~Model(){}

public:
	auto DLLEXPORT virtual GetMesh() -> Mesh*;
	auto DLLEXPORT virtual GetMeshPath() -> const char*;
	auto DLLEXPORT virtual SetMaterial(Material *mat) -> void { mat_ = mat; }
	auto DLLEXPORT virtual GetMaterial() -> Material* { return mat_; }
	auto DLLEXPORT virtual GetWorldCenter() -> vec3;

	// GameObject
	auto DLLEXPORT virtual Clone() -> GameObject* override;
};
