#pragma once
#include "common.h"
#include "render_path_base.h"

class RenderPathPathTracing : public RenderPathBase
{
	Material* pathtracingDrawMaterial{};
	Material* pathtracingPreviewMaterial{};
	float drawMS;

	struct Triangle
	{
		vec4 p0, p1, p2;
		uint32 materialID;
	};
	uint32_t trianglesBufferLen{};
	SharedPtr<StructuredBuffer> trianglesBuffer;

public:
	RenderPathPathTracing();

	uint getNumLines() override {return 1;}
	std::string getString(uint i) override;
	void RenderFrame() override;
};
