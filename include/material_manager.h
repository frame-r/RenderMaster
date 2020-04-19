#pragma once
#include "common.h"



class MaterialManager
{
	Signal<Material*> onMaterialChanged;

public:
	void Init();
	void Free();
	void MaterialChanged(Material* mat)
	{
		onMaterialChanged.Invoke(mat);
	}
	void AddCallbackMaterialChanged(MaterialCallback callback)
	{
		onMaterialChanged.Add(callback);
	}

public:
	auto DLLEXPORT CreateMaterial(const char *genericmat) -> Material*;
	auto DLLEXPORT CreateInternalMaterial(const char *genericmat) -> Material*;
	auto DLLEXPORT DestoryMaterial(Material *mat) -> void;
	auto DLLEXPORT GetMaterial(const char *id) -> Material*;
	auto DLLEXPORT GetGenericMaterial(const char *path) -> GenericMaterial*;
	auto DLLEXPORT GetDiffuseMaterial() -> Material*;
};

