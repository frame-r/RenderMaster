#pragma once
#include "common.h"

struct Mats
{
	// Original
	mat4 ProjUnjitteredMat_;
	mat4 ViewUnjitteredMat_;
	mat4 ViewProjUnjitteredMat_;

	// Jittered
	mat4 ProjMat_;
	mat4 ViewMat_;
	mat4 ViewProjMat_;
	mat4 ViewProjectionInvMat_;
	mat4 ViewInvMat_;

	vec4 WorldPos_;
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

