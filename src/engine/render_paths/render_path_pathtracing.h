#pragma once
#include "common.h"
#include "render_path_base.h"

class RenderPathPathTracing : public RenderPathBase
{
	static RenderPathPathTracing* instance;

	Material* pathtracingDrawMaterial{};
	Material* pathtracingPreviewMaterial{};
	float drawMS;
	SharedPtr<Texture> out;

	uint32_t trianglesCount{};
	SharedPtr<StructuredBuffer> trianglesBuffer;

	uint32_t areaLightsCount{};
	SharedPtr<StructuredBuffer> areaLightBuffer;

	uint32_t materialsCount{};
	SharedPtr<StructuredBuffer> materialsBuffer;
	bool needUploadMaterials{ false };
	Material* materialChanged;
	std::vector<GPUMaterial> gpuMaterials;
	std::unordered_map<Material*, size_t> matPointerToIndex;

	uint32_t crc_{};

	static void onMaterialChanged(Material* mat);

	std::shared_ptr<RaytracingData> getScene(Render::RenderScene& scene, size_t triangles);
	void fillGPUMaterial(GPUMaterial& out, Material* in);

public:
	RenderPathPathTracing();

	uint getNumLines() override {return 1;}
	std::string getString(uint i) override;
	void uploadScene(Render::RenderScene& scene);
	void uploadMaterials(size_t mats);
	void RenderFrame() override;
};
