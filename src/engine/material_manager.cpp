#include "pch.h"
#include "material_manager.h"
#include "core.h"
#include "console.h"
#include "filesystem.h"
#include "material.h"
#include <unordered_map>

static std::unordered_map<std::string, Material*> materials; // id -> Material
static std::unordered_map<std::string, GenericMaterial*> genericMaterials; // id -> GenericMaterial
static int matNameCounter;


void MaterialManager::Init()
{
	//
	// Load generic materials
	vector<string> paths = FS->FilterPaths(GENERIC_MATERIAL_EXT);

	for (auto& p : paths)
	{
		GenericMaterial *m = new GenericMaterial(p);
		m->LoadXML();

		string id = m->id_;

		if (genericMaterials.find(id) != genericMaterials.end())
		{
			LogCritical("MaterialManager::Init(): duplicate material with name='%s'", id.c_str());
			delete m;
			continue;
		}
		genericMaterials[id] = m;
	}

	//
	// Load user materials
	paths = FS->FilterPaths(USER_MATERIAL_EXT);
	for (auto& p : paths)
	{
		Material* m = new Material(p);
		m->LoadXML();
		materials[m->GetId()] = m;
	}
}

void MaterialManager::Free()
{
	for (auto &m : materials)
	{
		m.second->SaveXML();
		delete m.second;
	}
	materials.clear();

	for (auto &m : genericMaterials)
		delete m.second;

	genericMaterials.clear();
}

auto DLLEXPORT MaterialManager::CreateMaterial(const char* genericmat) -> Material*
{
	auto it = genericMaterials.find(genericmat);

	if (it == genericMaterials.end())
	{
		LogCritical("MaterialManager::CreateMaterial(): unable find material %s", genericmat);
		return nullptr;
	}

	string id = "material_" + std::to_string(matNameCounter);
	while (materials.find(id) != materials.end())
	{
		matNameCounter++;
		id = "material_" + std::to_string(matNameCounter);
	}

	Material *mat = new Material(id, it->second);
	materials[id] = mat;
	return mat;
}

auto DLLEXPORT MaterialManager::GetMaterial(const char * path) -> Material*
{
	auto it = materials.find(path);
	return it == materials.end() ? nullptr : it->second;
}

auto DLLEXPORT MaterialManager::GetGenericMaterial(const char* path) -> GenericMaterial*
{
	auto it = genericMaterials.find(path);
	return it == genericMaterials.end() ? nullptr : it->second;
}
