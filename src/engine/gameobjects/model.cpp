#include "pch.h"
#include "model.h"
#include "material.h"
#include "core.h"
#include "material_manager.h"
#include "yaml.inl"


void Model::Copy(GameObject * original)
{
	GameObject::Copy(original);

	Model *original_model = static_cast<Model*>(original);
	meshPtr = original_model->meshPtr;
	mat_ = original_model->mat_;
}

Model::Model()
{
	type_ = OBJECT_TYPE::MODEL;
	mat_ = MAT_MAN->GetDiffuseMaterial();
}

Model::Model(StreamPtr<Mesh> mesh) : Model()
{
	meshPtr = mesh;
}

auto DLLEXPORT Model::GetMesh() -> Mesh *
{
	return meshPtr.get();
}
auto DLLEXPORT Model::GetMeshPath() -> const char *
{
	return meshPtr.path().c_str();
}

auto DLLEXPORT Model::GetWorldCenter() -> vec3
{
	vec3 center = meshPtr.isLoaded() ? meshPtr.get()->GetCenter() : vec3();
	vec4 centerWS = worldTransform_ * vec4(center);
	return (vec3)centerWS;
}

auto DLLEXPORT Model::GetTrinaglesWorldSpace(std::unique_ptr<vec3[]>& out, uint* trinaglesNum) -> void
{
}

auto DLLEXPORT Model::Clone() -> GameObject *
{
	Model *m = new Model;
	m->Copy(this);
	return m;
}

void Model::SaveYAML(void * yaml)
{
	GameObject::SaveYAML(yaml);

	YAML::Emitter *_n = static_cast<YAML::Emitter*>(yaml);
	YAML::Emitter& n = *_n;

	if (!meshPtr.path().empty())
		n << YAML::Key << "mesh" << YAML::Value << meshPtr.path();

	if (mat_)
		n << YAML::Key << "material" << YAML::Value << mat_->GetId();
}

void Model::LoadYAML(void * yaml)
{
	GameObject::LoadYAML(yaml);

	YAML::Node *_n = static_cast<YAML::Node*>(yaml);
	YAML::Node& n = *_n;

	if (n["material"])
	{
		string mat = n["material"].as<string>();
		mat_ = MAT_MAN->GetMaterial(mat.c_str());
	}
}

