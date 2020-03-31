#pragma once
#include "common.h"
#include "icorerender.h"


struct RaytracingData
{
	std::vector<vec4> triangles;
};

class Mesh
{
	std::unique_ptr<ICoreMesh> coreMeshPtr;
	std::string path_;
	vec3 center_;
	std::shared_ptr<RaytracingData> trianglesDataPtr;

public:
	Mesh(const std::string& path);
	~Mesh();

	bool Load();
	std::shared_ptr<RaytracingData> GetRaytracingData(mat4 worldTransformMat);
	bool isSphere();
	bool isStd();

	auto DLLEXPORT GetCoreMesh() -> ICoreMesh* { return coreMeshPtr.get(); }
	auto DLLEXPORT GetAttributes() -> INPUT_ATTRUBUTE;
	auto DLLEXPORT GetVideoMemoryUsage() -> size_t;
	auto DLLEXPORT GetPath() -> const char* const { return path_.c_str(); }
	auto DLLEXPORT GetCenter() -> vec3;
};
