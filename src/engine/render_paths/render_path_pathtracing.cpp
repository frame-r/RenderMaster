#include "pch.h"
#include "render_path_pathtracing.h"
#include "core.h"
#include "render.h"
#include "shader.h"
#include "material.h"
#include "light.h"
#include "model.h"
#include "mesh.h"
#include "icorerender.h"
#include "material_manager.h"
#include "resource_manager.h"

vector<string> defines{ "GROUP_DIM_X=16", "GROUP_DIM_Y=16" };

RenderPathPathTracing::RenderPathPathTracing()
{
	MaterialManager* mm = _core->GetMaterialManager();

	pathtracingDrawMaterial = mm->CreateInternalMaterial("pathtracing_draw");
	assert(pathtracingDrawMaterial);

	pathtracingPreviewMaterial = mm->CreateInternalMaterial("pathtracing_preview");
	assert(pathtracingPreviewMaterial);
}

std::string RenderPathPathTracing::getString(uint i)
{
	switch (i)
	{
		case 0: return "Draw GPU: " + std::to_string(drawMS); break;
	}
	return "";
}

void drawMeshes(Material * pathtracingPreviewMaterial, std::vector<Render::RenderMesh>& meshes, mat4 VP, vec4 sun_dir)
{
	for (Render::RenderMesh& renderMesh : meshes)
	{
		Shader* shader = pathtracingPreviewMaterial->GetForwardShader(renderMesh.mesh);
		if (!shader)
			continue;

		CORE_RENDER->SetShader(shader);

		mat4 MVP = VP * renderMesh.worldTransformMat;
		mat4 M = renderMesh.worldTransformMat;
		mat4 NM = M.Inverse().Transpose();

		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetMat4Parameter("M", &M);
		shader->SetMat4Parameter("NM", &NM);
		shader->SetVec4Parameter("sun_dir", &sun_dir);

		shader->FlushParameters();

		CORE_RENDER->Draw(renderMesh.mesh, 1);
	}
}

size_t sceneTriangleCount(Render::RenderScene& scene)
{
	size_t tris = 0;

	for (int i = 0; i < scene.meshes.size(); ++i)
	{
		Render::RenderMesh& r = scene.meshes[i];

		if (r.mesh->isStd() && !r.mesh->isPlane())
			continue;

		std::shared_ptr<RaytracingData> rtWorldTriangles = r.model->GetRaytracingData();
		tris += rtWorldTriangles->size();
	}

	return tris;
}

std::shared_ptr<RaytracingData> getTriangles(Render::RenderScene& scene, size_t triangles)
{
	std::shared_ptr<RaytracingData> ret =std::make_shared<RaytracingData>(triangles);
	GPURaytracingTriangle* data = ret->triangles.data();

	for (int i = 0; i < scene.meshes.size(); ++i)
	{
		Render::RenderMesh& r = scene.meshes[i];

		if (r.mesh->isStd() && !r.mesh->isPlane())
			continue;

		std::shared_ptr<RaytracingData> rtWorldTriangles = r.model->GetRaytracingData();

		size_t bytes = sizeof(GPURaytracingTriangle) * rtWorldTriangles->size();
		memcpy(data, rtWorldTriangles->triangles.data(), bytes);
		data += rtWorldTriangles->triangles.size();
	}

	return ret;
}

void RenderPathPathTracing::uploadScene(Render::RenderScene& scene)
{
	// 1. Triangles
	{
		size_t triagles = sceneTriangleCount(scene);
		size_t bufferLen = triagles * sizeof(GPURaytracingTriangle);

		if (trianglesCount < (uint32_t)triagles)
			trianglesBuffer = RES_MAN->CreateStructuredBuffer((uint)bufferLen, sizeof(GPURaytracingTriangle), BUFFER_USAGE::GPU_READ);
		trianglesCount = (uint32_t)triagles;

		auto trianglesPtr = getTriangles(scene, triagles);
		trianglesBuffer->SetData((uint8*)trianglesPtr->triangles.data(), bufferLen);
	}

	// 2. Area lights
	{
		size_t newAreaLightsCount = scene.areaLightCount();
		size_t bufferLen = newAreaLightsCount * sizeof(GPURaytracingAreaLight);

		if (areaLightsCount < (uint32_t)newAreaLightsCount)
			areaLightBuffer = RES_MAN->CreateStructuredBuffer((uint)bufferLen, sizeof(GPURaytracingAreaLight), BUFFER_USAGE::GPU_READ);
		areaLightsCount = (uint32_t)newAreaLightsCount;

		vector<GPURaytracingAreaLight> areaLightData(newAreaLightsCount);
		for (size_t i = 0; i < scene.areaLightCount(); ++i)
		{
			areaLightData[i].p0 = scene.areaLights[i].transform * (vec4(-1, 1, 0, 1));
			areaLightData[i].p1 = scene.areaLights[i].transform * (vec4(-1,-1, 0, 1));
			areaLightData[i].p2 = scene.areaLights[i].transform * (vec4( 1,-1, 0, 1));
			areaLightData[i].p3 = scene.areaLights[i].transform * (vec4( 1, 1, 0, 1));
			areaLightData[i].center = vec4(scene.areaLights[i].transform.Column3(3));
			areaLightData[i].center.w = 1;
			areaLightData[i].T = (areaLightData[i].p1 - areaLightData[i].p0) * .5f;
			areaLightData[i].B = (areaLightData[i].p3 - areaLightData[i].p0) * .5f;
			areaLightData[i].T.w = 0;
			areaLightData[i].B.w = 0;
			areaLightData[i].n = -triangle_normal(areaLightData[i].p0, areaLightData[i].p1, areaLightData[i].p2);
			areaLightData[i].color = vec4(1.0f) * scene.areaLights[i].intensity;
		}

		areaLightBuffer->SetData((uint8*)areaLightData.data(), bufferLen);
	}
}

void RenderPathPathTracing::RenderFrame()
{
	Texture* color = render->GetPrevRenderTexture(PREV_TEXTURES::PATH_TRACING_HDR, width, height, TEXTURE_FORMAT::RGBA16F);

	Render::RenderScene scene = render->getRenderScene();
	uint32_t nextcrc = scene.getHash();

	if (!out || out->GetHeight() != height || out->GetWidth() != width)
		out = RES_MAN->CreateTexture(width, height, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::RGBA32F, TEXTURE_CREATE_FLAGS::USAGE_UNORDRED_ACCESS);

	uint32 frameID_ = render->frameID();
	uint32 readbackFrameID_ = render->readbackFrameID();
	CORE_RENDER->TimersBeginPoint(frameID_, Render::T_PATH_TRACING_DRAW);

	auto clearHDRbuffer = [this]() -> void
	{		
		if (Shader* pathtracingshader = RENDER->GetComputeShader("pathtracing\\pathtracing_clear.hlsl", &defines))
		{
			CORE_RENDER->SetShader(pathtracingshader);
			pathtracingshader->SetUintParameter("maxSize_x", width);
			pathtracingshader->SetUintParameter("maxSize_y", height);
			pathtracingshader->FlushParameters();

			CORE_RENDER->BindStructuredBuffer(0, trianglesBuffer.get());

			Texture* uavs[] = { out.get() };
			CORE_RENDER->CSBindUnorderedAccessTextures(1, uavs);

			constexpr uint warpSize = 16;
			int numGroupsX = (width + (warpSize - 1)) / warpSize;
			int numGroupsY = (height + (warpSize - 1)) / warpSize;

			CORE_RENDER->Dispatch(numGroupsX, numGroupsY, 1);

			CORE_RENDER->CSBindUnorderedAccessTextures(1, nullptr);
		}
	};

	auto fillDepthBuffer = [this](Render::RenderScene& scene)
	{
		CORE_RENDER->Clear();
		CORE_RENDER->SetDepthTest(1);

		drawMeshes(pathtracingPreviewMaterial, scene.meshes, mats.ViewProjUnjitteredMat_, scene.sun_direction);
	};

	Texture* rts[1] = { CORE_RENDER->GetSurfaceColorTexture() };
	CORE_RENDER->SetRenderTextures(1, rts, CORE_RENDER->GetSurfaceDepthTexture());

	if (crc_ != nextcrc)
	{
		crc_ = nextcrc;

		uploadScene(scene);
		clearHDRbuffer();
		fillDepthBuffer(scene);

		Log("Scene upload. Hash: %u\n", crc_);
	}

	if (memcmp(&prevMats.ViewProjUnjitteredMat_, &mats.ViewProjUnjitteredMat_, sizeof(mat4)) != 0)
	{
		clearHDRbuffer();
		fillDepthBuffer(scene);
	}

	if (Shader* pathtracingshader = RENDER->GetComputeShader("pathtracing\\pathtracing_draw.hlsl", &defines))
	{
		CORE_RENDER->SetShader(pathtracingshader);

		vec3 forwardWS = -mats.ViewInvMat_.Column3(2).Normalized();
		vec3 rightWS = mats.ViewInvMat_.Column3(0).Normalized() * tan(verFullFovInRadians * 0.5f) * aspect;
		vec3 upWS = mats.ViewInvMat_.Column3(1).Normalized() * tan(verFullFovInRadians * 0.5f);

		pathtracingshader->SetVec4Parameter("cam_forward_ws", &vec4(forwardWS));
		pathtracingshader->SetVec4Parameter("cam_right_ws", &vec4(rightWS));
		pathtracingshader->SetVec4Parameter("cam_up_ws", &vec4(upWS));
		pathtracingshader->SetVec4Parameter("cam_pos_ws", &mats.WorldPos_);
		pathtracingshader->SetUintParameter("maxSize_x", width);
		pathtracingshader->SetUintParameter("maxSize_y", height);
		pathtracingshader->SetUintParameter("triCount", trianglesCount);
		pathtracingshader->SetUintParameter("lightsCount", areaLightsCount);
		pathtracingshader->FlushParameters();

		CORE_RENDER->BindStructuredBuffer(0, trianglesBuffer.get());
		CORE_RENDER->BindStructuredBuffer(1, areaLightBuffer.get());

		Texture* uavs[] = { out.get() };
		CORE_RENDER->CSBindUnorderedAccessTextures(1, uavs);

		constexpr uint warpSize = 16;
		int numGroupsX = (width + (warpSize - 1)) / warpSize;
		int numGroupsY = (height + (warpSize - 1)) / warpSize;

		CORE_RENDER->Dispatch(numGroupsX, numGroupsY, 1);

		CORE_RENDER->CSBindUnorderedAccessTextures(1, nullptr);
	}

	CORE_RENDER->SetRenderTextures(1, rts, CORE_RENDER->GetSurfaceDepthTexture());

	if (Shader* pathtracingshader = RENDER->GetShader("pathtracing\\pathtracing_hdr_to_ldr.hlsl", nullptr))
	{
		CORE_RENDER->SetShader(pathtracingshader);

		constexpr int tex_count = 1;
		Texture* texs[tex_count] = { out.get() };

		CORE_RENDER->SetDepthTest(0);
		CORE_RENDER->BindTextures(tex_count, texs);
		{
			CORE_RENDER->Draw(render->fullScreen(), 1);
		}
		CORE_RENDER->BindTextures(tex_count, nullptr);
		CORE_RENDER->SetDepthTest(1);
	}

	draw_AreaLightEmblems(scene);

	CORE_RENDER->TimersEndPoint(frameID_, Render::T_PATH_TRACING_DRAW);
	drawMS = CORE_RENDER->GetTimeInMsForPoint(readbackFrameID_, Render::T_PATH_TRACING_DRAW);

	render->RenderGUI();
}

