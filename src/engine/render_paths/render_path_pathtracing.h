#pragma once
#include "common.h"
#include "render_path_base.h"

class RenderPathPathTracing : public RenderPathBase
{
	Material* pathtracingDrawMaterial{};
	Material* pathtracingPreviewMaterial{};
	float drawMS;
	SharedPtr<Texture> out;

	uint32_t trianglesCount{};
	SharedPtr<StructuredBuffer> trianglesBuffer;

	uint32_t areaLightsCount{};
	SharedPtr<StructuredBuffer> areaLightBuffer;

	uint32_t crc_{};

public:
	RenderPathPathTracing();

	uint getNumLines() override {return 1;}
	std::string getString(uint i) override;
	void uploadScene(Render::RenderScene& scene);
	void RenderFrame() override;
};
