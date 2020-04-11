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

size_t uploadSceneToGPU(Render::RenderScene& scene)
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
	RaytracingTriangle* data = ret->triangles.data();

	for (int i = 0; i < scene.meshes.size(); ++i)
	{
		Render::RenderMesh& r = scene.meshes[i];

		if (r.mesh->isStd() && !r.mesh->isPlane())
			continue;

		std::shared_ptr<RaytracingData> rtWorldTriangles = r.model->GetRaytracingData();

		size_t bytes = sizeof(RaytracingTriangle) * rtWorldTriangles->size();
		memcpy(data, rtWorldTriangles->triangles.data(), bytes);
		data += rtWorldTriangles->triangles.size();
	}

	return ret;
}

void RenderPathPathTracing::RenderFrame()
{
	uint w, h;
	CORE_RENDER->GetViewport(&w, &h);

	Texture* color = render->GetPrevRenderTexture(PREV_TEXTURES::PATH_TRACING_HDR, w, h, TEXTURE_FORMAT::RGBA16F);

	Render::RenderScene scene = render->getRenderScene();
	uint32_t nextcrc = scene.getHash();

	static uint32_t crc_;
	if (crc_ != nextcrc)
	{
		crc_ = nextcrc;
		size_t triagles = uploadSceneToGPU(scene);
		size_t allDataInBytes = triagles * sizeof(RaytracingTriangle);

		if (triagles > trianglesBufferLen)
		{
			trianglesBufferLen = triagles;
			trianglesBuffer = RES_MAN->CreateStructuredBuffer(allDataInBytes, sizeof(RaytracingTriangle), BUFFER_USAGE::GPU_READ);
		}

		auto trianglesPtr = getTriangles(scene, triagles);
		trianglesBuffer->SetData((uint8*)trianglesPtr->triangles.data(), allDataInBytes);

		Log("Scene changed %u\n", crc_);
	}

	//render->updateEnvirenment(scene);
	uint32 frameID_ = render->frameID();
	uint32 readbackFrameID_ = render->readbackFrameID();

	CORE_RENDER->TimersBeginPoint(frameID_, Render::T_PATH_TRACING_DRAW);

	static bool s = 0;
	if (s)
	{
		Texture* rts[1] = { CORE_RENDER->GetSurfaceColorTexture() };
		CORE_RENDER->SetRenderTextures(1, rts, CORE_RENDER->GetSurfaceDepthTexture());
		CORE_RENDER->Clear();
		CORE_RENDER->SetDepthTest(1);

		drawMeshes(pathtracingPreviewMaterial, scene.meshes, mats.ViewProjUnjitteredMat_, scene.sun_direction);
	}
	else
	{
		vector<string> defines;
		defines.push_back("GROUP_DIM_X=16");
		defines.push_back("GROUP_DIM_Y=16");

		if (Shader* pathtracingshader = RENDER->GetComputeShader("pathtracing\\pathtracing_draw.hlsl", &defines))
		{
			if (!out || out->GetHeight() != h || out->GetWidth() != w)
				out = RES_MAN->CreateTexture(w, h, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::RGBA32F, TEXTURE_CREATE_FLAGS::USAGE_UNORDRED_ACCESS);

			CORE_RENDER->SetShader(pathtracingshader);

			// TODO:
			//vec3 forwardWS = mats.ViewInvMat_.Column3(2).Normalized();
			//vec3 rightWS = mats.ViewInvMat_.Column3(0).Normalized() * tan(fovInRads * 0.5f) * aspect;
			//vec3 upWS = mats.ViewInvMat_.Column3(1).Normalized() * tan(fovInRads * 0.5f);

			//pathtracingshader->SetUintParameter("triCount", trianglesBufferLen);
			pathtracingshader->FlushParameters();

			CORE_RENDER->BindStructuredBuffer(0, trianglesBuffer.get());

			Texture* uavs[] = { out.get() };
			CORE_RENDER->CSBindUnorderedAccessTextures(1, uavs);

			constexpr uint warpSize = 16;
			int numGroupsX = (w + (warpSize - 1)) / warpSize;
			int numGroupsY = (h + (warpSize - 1)) / warpSize;

			CORE_RENDER->Dispatch(numGroupsX, numGroupsY, 1);

			CORE_RENDER->CSBindUnorderedAccessTextures(1, nullptr);
		}

		if (Shader* pathtracingshader = RENDER->GetShader("pathtracing\\pathtracing_hdr_to_ldr.hlsl", nullptr))
		{
			CORE_RENDER->SetShader(pathtracingshader);
			Texture* rts[1] = { CORE_RENDER->GetSurfaceColorTexture() };
			CORE_RENDER->SetRenderTextures(1, rts, CORE_RENDER->GetSurfaceDepthTexture());
			constexpr int tex_count = 1;
			Texture* texs[tex_count] = {
				out.get()
			};
			CORE_RENDER->SetDepthTest(0);

			CORE_RENDER->BindTextures(tex_count, texs);
			{
				CORE_RENDER->Draw(render->fullScreen(), 1);
			}
			CORE_RENDER->BindTextures(tex_count, nullptr);
		}
	}

	CORE_RENDER->TimersEndPoint(frameID_, Render::T_PATH_TRACING_DRAW);
	drawMS = CORE_RENDER->GetTimeInMsForPoint(readbackFrameID_, Render::T_PATH_TRACING_DRAW);

	render->RenderGUI();
}

