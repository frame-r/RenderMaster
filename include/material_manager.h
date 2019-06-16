#pragma once
#include "common.h"

class Material;
struct GenericMaterial;

class MaterialManager
{
public:
	void Init();
	void Free();

public:
	auto DLLEXPORT CreateMaterial(const char *genericmat) -> Material*;
	auto DLLEXPORT GetMaterial(const char *path) -> Material*;
	auto DLLEXPORT GetGenericMaterial(const char *path) -> GenericMaterial*;
};

