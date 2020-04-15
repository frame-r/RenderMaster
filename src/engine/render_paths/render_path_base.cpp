#include "pch.h"
#include "render_path_base.h"
#include "core.h"
#include "render.h"
#include "shader.h"
#include "material.h"
#include "icorerender.h"

static std::unordered_map<size_t, Mats> viewsDataMap; // view ID -> Data

static const vec2 taaSamples[16] =
{
	vec2(1.0f / 2.0f,  1.0f / 3.0f),
	vec2(1.0f / 4.0f,  2.0f / 3.0f),
	vec2(3.0f / 4.0f,  1.0f / 9.0f),
	vec2(1.0f / 8.0f,  4.0f / 9.0f),
	vec2(5.0f / 8.0f,  7.0f / 9.0f),
	vec2(3.0f / 8.0f,  2.0f / 9.0f),
	vec2(7.0f / 8.0f,  5.0f / 9.0f),
	vec2(1.0f / 16.0f,  8.0f / 9.0f),
	vec2(9.0f / 16.0f,  1.0f / 27.0f),
	vec2(5.0f / 16.0f, 10.0f / 27.0f),
	vec2(13.0f / 16.0f, 19.0f / 27.0f),
	vec2(3.0f / 16.0f,  4.0f / 27.0f),
	vec2(11.0f / 16.0f, 13.0f / 27.0f),
	vec2(7.0f / 16.0f, 22.0f / 27.0f),
	vec2(15.0f / 16.0f,  7.0f / 27.0f),
	vec2(1.0f / 32.0f, 16.0f / 27.0f)
};

void RenderPathBase::draw_AreaLightEmblems(const Render::RenderScene& scene)
{
	if (Shader* shader = render->GetShader("primitive.hlsl", render->fullScreen()))
	{
		CORE_RENDER->SetDepthTest(1);
		CORE_RENDER->SetShader(shader);

		for (const Render::RenderLight& renderLight : scene.areaLights)
		{
			static mat4 facking_wtf_correct;
			facking_wtf_correct.el_2D[0][3] = 0.0f;
			facking_wtf_correct.el_2D[1][3] = 0.5f;

			mat4 MVP = mats.ViewProjUnjitteredMat_ * renderLight.transform * facking_wtf_correct;

			shader->SetMat4Parameter("MVP", &MVP);

			vec4 one = vec4{ 1,1,1,1 };
			one *= renderLight.intensity;

			shader->SetVec4Parameter("main_color", &one);
			shader->FlushParameters();

			CORE_RENDER->Draw(render->fullScreen(), 1);
		}
		CORE_RENDER->SetDepthTest(0);
	}
}

RenderPathBase::RenderPathBase()
{
	render = _core->GetRender();
}

void RenderPathBase::FrameBegin(size_t viewID, const Engine::CameraData& camera, Model** wireframeModels, int modelsNum)
{
	verFullFovInRadians = camera.verFullFovInRadians;

	uint32 timerID_ = render->frameID();
	uint32 dataTimerID_ = render->readbackFrameID();

	CORE_RENDER->TimersBeginFrame(timerID_);
	CORE_RENDER->TimersBeginPoint(timerID_, Render::T_ALL_FRAME);

	CORE_RENDER->GetViewport(&width, &height);
	aspect = float(width) / height;

	CORE_RENDER->ResizeBuffersByViewort();

	// Init mats
	mats.ProjUnjitteredMat_ = camera.ProjMat;
	mats.ViewUnjitteredMat_ = camera.ViewMat;
	mats.ViewProjUnjitteredMat_ = camera.ProjMat * camera.ViewMat;
	mats.ProjMat_ = camera.ProjMat;

	Mats& prev = viewsDataMap[viewID];

	vec2 taaOffset;

	if (render->IsTAA())
	{
		taaOffset = taaSamples[_core->frame() % 16];
		taaOffset = (taaOffset * 2.0f - vec2(1, 1));

		float needJitter = float(render->GetViewMode() == VIEW_MODE::FINAL);

		mats.ProjMat_.el_2D[0][2] += needJitter * taaOffset.x / width;
		mats.ProjMat_.el_2D[1][2] += needJitter * taaOffset.y / height;

		// rejitter prev
		if (_core->frame() > 1)
			cameraPrevViewProjMatRejittered_ = prev.ProjUnjitteredMat_;
		else
			cameraPrevViewProjMatRejittered_ = camera.ProjMat;

		cameraPrevViewProjMatRejittered_.el_2D[0][2] += taaOffset.x / width;
		cameraPrevViewProjMatRejittered_.el_2D[1][2] += taaOffset.y / height;

		if (_core->frame() > 1)
			cameraPrevViewProjMatRejittered_ = cameraPrevViewProjMatRejittered_ * prev.ViewMat_;
		else
			cameraPrevViewProjMatRejittered_ = cameraPrevViewProjMatRejittered_ * camera.ViewMat;
	}
	else
		cameraPrevViewProjMatRejittered_ = prev.ProjMat_ * prev.ViewMat_;

	mats.ViewProjMat_ = mats.ProjMat_ * camera.ViewMat;
	mats.ViewMat_ = camera.ViewMat;
	mats.WorldPos_ = camera.ViewMat.Inverse().Column3(3);
	mats.ViewProjInvMat_ = mats.ViewProjMat_.Inverse();
	mats.ViewInvMat_ = mats.ViewMat_.Inverse();
	//

	// Restore prev matricies
	if (_core->frame() > 1)
		prevMats = prev;
	else
		prevMats = mats;

	// Save prev matricies
	prev = mats;
}

void RenderPathBase::FrameEnd()
{
	uint32 timerID_ = render->frameID();
	uint32 dataTimerID_ = render->readbackFrameID();

	CORE_RENDER->TimersEndPoint(timerID_, Render::T_ALL_FRAME);
	frameMs = CORE_RENDER->GetTimeInMsForPoint(dataTimerID_, Render::T_ALL_FRAME);

	CORE_RENDER->TimersEndFrame(timerID_);
}
