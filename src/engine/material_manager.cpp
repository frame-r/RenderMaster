#include "pch.h"
#include "material_manager.h"
#include "core.h"
#include "console.h"
#include "filesystem.h"
#include "material.h"
#include <unordered_map>

static std::unordered_map<std::string, Material*> materials; // path -> Material
static int matNameCounter;

void MaterialManager::Init()
{
	std::vector<std::string> paths = FS->GetPaths(".mat");

	for(auto& p : paths)
	{
		Material *m = new Material(p);
		m->Load();
		materials[p] = m;
	}
	Log("Materials: %i", paths.size());
}

void MaterialManager::Free()
{
	for (auto &p : materials)
	{
		p.second->Save();
		delete p.second;
	}
	materials.clear();
}

auto DLLEXPORT MaterialManager::CreateMaterial() -> Material*
{
	string path = "material_" + std::to_string(matNameCounter) + ".mat";
	while (materials.find(path) != materials.end())
	{
		matNameCounter++;
		path = "material_" + std::to_string(matNameCounter) + ".mat";
	}

	Material *mat = new Material(path);
	materials[path] = mat;

	mat->Save();

	return mat;
}

auto DLLEXPORT MaterialManager::GetMaterial(const char * path) -> Material*
{
	auto it = materials.find(path);
	if (it == materials.end())
		return nullptr;
	return it->second;
}
