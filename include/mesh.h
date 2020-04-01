#pragma once
#include "common.h"
#include "icorerender.h"


class Mesh
{
	std::unique_ptr<ICoreMesh> coreMeshPtr;
	std::string path_;
	vec3 center_;
	uint32_t triangles;
	std::shared_ptr<RaytracingData> trianglesDataObjectSpace;

public:
	Mesh(const std::string& path);
	~Mesh();

	bool Load();
	std::shared_ptr<RaytracingData> GetRaytracingData();
	bool isSphere();
	bool isPlane();
	bool isStd();

	auto DLLEXPORT GetCoreMesh() -> ICoreMesh* { return coreMeshPtr.get(); }
	auto DLLEXPORT GetAttributes() -> INPUT_ATTRUBUTE;
	auto DLLEXPORT GetVideoMemoryUsage() -> size_t;
	auto DLLEXPORT GetPath() -> const char* const { return path_.c_str(); }
	auto DLLEXPORT GetCenter() -> vec3;
};
