#pragma once
#include "common.h"
#include "icorerender.h"

class Mesh
{
	std::unique_ptr<ICoreMesh> coreMesh_;
	std::string path_;

public:
	Mesh(const std::string& path);
	~Mesh();

	bool Load();

	auto DLLEXPORT GetCoreMesh() -> ICoreMesh* { return coreMesh_.get(); }
	auto DLLEXPORT GetAttributes() -> INPUT_ATTRUBUTE;
	auto DLLEXPORT GetVideoMemoryUsage() -> size_t;
	auto DLLEXPORT GetPath() -> const char* const { return path_.c_str(); }
};
