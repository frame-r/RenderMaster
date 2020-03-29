#pragma once
#include "gameobject.h"
#include "mesh.h"
#include "resource_manager.h"


class Model final : public GameObject
{
	StreamPtr<Mesh> meshPtr;
	vec3 meshCeneter;
	Material *mat_{nullptr};

protected:
	virtual void Copy(GameObject *original) override;
	virtual void SaveYAML(void *yaml) override;
	virtual void LoadYAML(void *yaml) override;

public:
	Model();
	Model(StreamPtr<Mesh> mesh);

	auto DLLEXPORT GetMesh() -> Mesh*;
	auto DLLEXPORT GetMeshPath() -> const char*;
	auto DLLEXPORT SetMaterial(Material *mat) -> void { mat_ = mat; }
	auto DLLEXPORT GetMaterial() -> Material* { return mat_; }
	auto DLLEXPORT GetWorldCenter() -> vec3;
	auto DLLEXPORT GetTrinaglesWorldSpace(std::unique_ptr<vec3[]>& out, uint* trinaglesNum) -> void;

	// GameObject
	auto DLLEXPORT virtual Clone() -> GameObject* override;
};
