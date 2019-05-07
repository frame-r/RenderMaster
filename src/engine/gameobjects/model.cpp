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
}

Model::Model(ManagedPtr<Mesh> mesh) : mesh_(mesh)
{
	type_ = OBJECT_TYPE::MODEL;
}

auto DLLEXPORT Model::GetMesh() -> Mesh *
{
	return mesh_.get();
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

	n << YAML::Key << "mesh" << YAML::Value << mesh_.path();
	n << YAML::Key << "material" << YAML::Value << mat_->GetPath();
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

