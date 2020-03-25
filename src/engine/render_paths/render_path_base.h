#pragma once
#include "common.h"

struct Mats
{
	// original
	mat4 cameraProjUnjitteredMat_;
	mat4 cameraViewUnjitteredMat_;

	// with jitters
	mat4 cameraProjMat_;
	mat4 cameraViewMat_;
	mat4 cameraViewProjMat_;
	vec4 cameraWorldPos_;
	mat4 cameraViewProjectionInvMat_;
	mat4 cameraViewInvMat_;
};

class RenderPathBase
{
protected:
	Render* render;
	Mats mats;
	Mats prevMats;
	mat4 cameraPrevViewProjMatRejittered_; // previous Projection matrix with same jitter as current frame
	float frameMs;

public:
	RenderPathBase();

	virtual uint getNumLines() = 0;
	virtual std::string getString(uint i) = 0;
	virtual void RenderFrame() = 0;

	void FrameBegin(size_t viewID, const mat4& ViewMat, const mat4& ProjMat, Model** wireframeModels, int modelsNum);
	void FrameEnd();
};

