#pragma once
#include "common.h"
#include "render_path_base.h"

class RenderPathPathTracing : public RenderPathBase
{
	Material* pathtracingDrawMaterial{};
	float drawMS;

public:
	RenderPathPathTracing();

	uint getNumLines() override {return 1;}
	std::string getString(uint i) override;
	void RenderFrame() override;
};
