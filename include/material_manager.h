#pragma once
#include "common.h"


class MaterialManager
{
public:
	void Init();
	void Free();

public:
	auto DLLEXPORT CreateMaterial(const char *genericmat) -> Material*;
	auto DLLEXPORT CreateInternalMaterial(const char *genericmat) -> Material*;
	auto DLLEXPORT DestoryMaterial(Material *mat) -> void;
	auto DLLEXPORT GetMaterial(const char *id) -> Material*;
	auto DLLEXPORT GetGenericMaterial(const char *path) -> GenericMaterial*;
	auto DLLEXPORT GetDiffuseMaterial() -> Material*;
};

