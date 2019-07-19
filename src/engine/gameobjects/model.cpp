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
	mesh_ = original_model->mesh_;
	mat_ = original_model->mat_;
}

Model::Model()
{
	type_ = OBJECT_TYPE::MODEL;
	mat_ = MAT_MAN->GetDiffuseMaterial();
}

Model::Model(ManagedPtr<Mesh> mesh) : Model()
{
	mesh_ = mesh;
}

auto DLLEXPORT Model::GetMesh() -> Mesh *
{
	return mesh_.get();
}
auto DLLEXPORT Model::GetMeshPath() -> const char *
{
	return mesh_.path().c_str();
}

auto DLLEXPORT Model::GetWorldCenter() -> vec3
{
	vec3 center = mesh_.isLoaded() ? mesh_.get()->GetCenter() : vec3();
	vec4 centerWS = worldTransform_ * vec4(center);
	return (vec3)centerWS;
}

auto DLLEXPORT Model::Clone() -> GameObject *
{
	Model *m = new Model;
	m->Copy(this);
	return m;
}

void Model::Serialize(void * yaml)
{
	GameObject::Serialize(yaml);

	YAML::Emitter *_n = static_cast<YAML::Emitter*>(yaml);
	YAML::Emitter& n = *_n;

	if (!mesh_.path().empty())
		n << YAML::Key << "mesh" << YAML::Value << mesh_.path();

	if (mat_)
		n << YAML::Key << "material" << YAML::Value << mat_->GetId();
}

void Model::Deserialize(void * yaml)
{
	GameObject::Deserialize(yaml);

	YAML::Node *_n = static_cast<YAML::Node*>(yaml);
	YAML::Node& n = *_n;

	if (n["material"])
	{
		string mat = n["material"].as<string>();
		mat_ = MAT_MAN->GetMaterial(mat.c_str());
	}
}

