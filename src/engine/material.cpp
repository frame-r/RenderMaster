#include "pch.h"
#include "material.h"
#include "core.h"
#include "console.h"
#include "resource_manager.h"
#include "filesystem.h"
#include "yaml-cpp/yaml.h"
#include "yaml.inl"

using namespace YAML;

void Material::Load()
{
	YAML::Emitter out;
	
	File f = FS->OpenFile(path_.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
	
	uint fileSize =	(uint)f.FileSize();
	
	unique_ptr<char[]> tmp = unique_ptr<char[]>(new char[fileSize + 1]);
	tmp[fileSize] = '\0';
	
	f.Read((uint8 *)tmp.get(), fileSize);
	
	YAML::Node n = YAML::Load(tmp.get());
	auto t = n.Type();

	loadVec4(n, "color", color_);

	if (n["metallic"])
		metallic_ = n["metallic"].as<float>();
	if (n["roughness"])
		roughness_ = n["roughness"].as<float>();
	if (n["albedoTex"])
		albedoTex_ = n["albedoTex"].as<string>();
}

void Material::Save()
{
	YAML::Emitter out;

	out << LocalTag("Material");
	out << BeginMap;
	out << Key << "color" << Value << color_;
	out << Key << "metallic" << Value << metallic_;
	out << Key << "roughness" << Value << roughness_;
	if (!albedoTex_.empty())
		out << Key << "albedoTex" << Value << albedoTex_;
	out << EndMap;

	File f = FS->OpenFile(path_.c_str(), FILE_OPEN_MODE::WRITE | FILE_OPEN_MODE::BINARY);
	
	f.WriteStr(out.c_str());
}

auto DLLEXPORT Material::SetAlbedoTexName(const char * path) -> void
{
	albedoTex_ = path;
	albedoTexPtr_ = RES_MAN->CreateStreamTexture(path, TEXTURE_CREATE_FLAGS::NONE);
}
