#pragma once
#include "Common.h"
#include "yaml-cpp/yaml.h"

class Material;

void loadYAMLMat(YAML::Node& n, Material &mat);
YAML::Emitter& operator<<(YAML::Emitter& out, Material& mat);

class Material : public IMaterial
{
	string _path;
	vec4 _color{1.0f, 1.0f, 1.0f, 1.0f};
	float _metallic{0.0f};
	float _roughness{0.0f};

	friend void loadYAMLMat(YAML::Node& n, Material &mat);
	friend YAML::Emitter& operator<<(YAML::Emitter& out, Material& mat);

public:
	Material(const char *path) : _path(path) {}
	virtual ~Material() {}

	void Load();

	API_VOID GetBaseColor(OUT vec4 *color) override;
	API_VOID SetBaseColor(const vec4 *color) override;
	API_VOID GetPath(OUT const char **path) override;
	API_VOID Save() override;
};
