#pragma once
#include "common.h"

class Material
{
	std::string path_;
	vec4 color_{1.0f, 1.0f, 1.0f, 1.0f};
	float roughness_{0.0f};
	float metallic_{0.0f};

public:
	Material(const std::string& path) : path_(path) {}

	void Load();
	void Save();

	auto DLLEXPORT GetPath() -> std::string const { return path_; }
	auto DLLEXPORT GetColor() -> vec4 const { return color_; }
	auto DLLEXPORT SetColor(const vec4& v) -> void { color_ = v; }
	auto DLLEXPORT GetRoughness() -> float const { return roughness_; }
	auto DLLEXPORT SetRoughness(float v) -> void { roughness_ = v; }
	auto DLLEXPORT GetMetallic() -> float const { return metallic_; }
	auto DLLEXPORT SetMetallic(float v) -> void { metallic_ = v; }
};
