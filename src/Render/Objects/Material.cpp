#include "Pch.h"
#include "Material.h"
#include "Core.h"
#include "yaml-cpp/yaml.h"

using namespace YAML;

extern Core *_pCore;


Emitter& operator<<(Emitter& out, const vec4& v)
{
	out << Flow;
	out << BeginSeq << v.x << v.y << v.z << v.w << EndSeq;
	return out;
}

void loadYAMLMat(YAML::Node& n, Material &mat)
{
	if (n["base_color"])
	{
		Node pos = n["base_color"];
		for (std::size_t i = 0; i < 4; i++)
			mat._color.xyzw[i] = pos[i].as<float>();
	}	
	if (n["metallic"])
	{
		Node pos = n["metallic"];
		mat._metallic = pos.as<float>();
	}	
	if (n["roughness"])
	{
		Node pos = n["roughness"];
		mat._metallic = pos.as<float>();
	}	
}

YAML::Emitter& operator<<(YAML::Emitter & out, Material & mat)
{
	out << LocalTag("Material");
	out << BeginMap;
	out << Key << "base_color" << Value << mat._color;
	out << Key << "metallic" << Value << mat._metallic;
	out << Key << "roughness" << Value << mat._roughness;
	out << EndMap;
	return out;
}

void Material::Load()
{
	IFileSystem *fs;
	_pCore->GetSubSystem((ISubSystem**)&fs, SUBSYSTEM_TYPE::FILESYSTEM);
	
	YAML::Emitter out;
	out << YAML::Block << *this;
	
	IFile *f = nullptr;
	fs->OpenFile(&f, _path.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
	
	uint fileSize;
	f->FileSize(&fileSize);
	
	char *tmp = new char[fileSize + 1];
	tmp[fileSize] = '\0';
	
	f->Read((uint8 *)tmp, fileSize);
	f->CloseAndFree();
	
	YAML::Node model_yaml = YAML::Load(tmp);
	auto t = model_yaml.Type();	
	
	loadYAMLMat(model_yaml, *this);
	
	delete tmp;

}

API_VOID Material::GetBaseColor(OUT vec4 * color)
{
	*color = _color;
}

API_VOID Material::SetBaseColor(const vec4 *color)
{
	_color = *color;
}

API_VOID Material::GetPath(OUT const char ** path)
{
	*path = _path.c_str();
}

API_VOID Material::Save()
{
	YAML::Emitter out;
	out << YAML::Block << *this;

	IFileSystem *fs;
	_pCore->GetSubSystem((ISubSystem**)&fs, SUBSYSTEM_TYPE::FILESYSTEM);
	
	IFile *f = nullptr;
	fs->OpenFile(&f, _path.c_str(), FILE_OPEN_MODE::WRITE | FILE_OPEN_MODE::BINARY);
	
	f->WriteStr(out.c_str());
	
	f->CloseAndFree();
}
