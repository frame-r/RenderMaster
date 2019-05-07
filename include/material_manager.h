#pragma once
#include "common.h"

class Material;

class MaterialManager
{
public:
	void Init();
	void Free();

public:
	auto DLLEXPORT CreateMaterial() -> Material*;
	auto DLLEXPORT GetMaterial(const char *path) -> Material*;
};

