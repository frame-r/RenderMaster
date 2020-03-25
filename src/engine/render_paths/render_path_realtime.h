#pragma once
#include "common.h"
#include "render_path_base.h"

class RenderPathRealtime : public RenderPathBase
{
	float gbufferMs;
	float lightsMs;
	float compositeMs;

	Material* compositeMaterial{};
	Material* finalPostMaterial{};

	struct RenderBuffers
	{
		Texture* color;
		Texture* colorReprojected;
		Texture* albedo;
		Texture* normal;
		Texture* shading;
		Texture* depth;
		Texture* diffuseLight;
		Texture* specularLight;
		Texture* velocity;
	};

public:
	RenderPathRealtime();

	uint getNumLines() override {return 4;}
	std::string getString(uint i) override;
	void RenderFrame() override;
};
