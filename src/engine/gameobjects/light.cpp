#include "pch.h"
#include "light.h"
#include "yaml.inl"

void Light::Copy(GameObject * original)
{
	GameObject::Copy(original);

	Light *original_light = static_cast<Light*>(original);
	intensity_ = original_light->intensity_;
}

void Light::Serialize(void * yaml)
{
	GameObject::Serialize(yaml);

	YAML::Emitter *_n = static_cast<YAML::Emitter*>(yaml);
	YAML::Emitter& n = *_n;

	n << YAML::Key << "intensity" << YAML::Value << intensity_;
}

void Light::Deserialize(void * yaml)
{
	GameObject::Deserialize(yaml);

	YAML::Node *_n = static_cast<YAML::Node*>(yaml);
	YAML::Node& n = *_n;

	if (n["intensity"]) intensity_ = n["intensity"].as<float>();
}

Light::Light()
{
	type_ = OBJECT_TYPE::LIGHT;
}

auto DLLEXPORT Light::Clone() -> GameObject *
{
	Light *l =new Light;
	l->Copy(this);
	return l;
}
