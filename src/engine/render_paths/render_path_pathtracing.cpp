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

RenderPathPathTracing* RenderPathPathTracing::instance;

void RenderPathPathTracing::onMaterialChanged(Material* mat)
{
	instance->needUploadMaterials = true;
	instance->materialChanged = mat;
}

RenderPathPathTracing::RenderPathPathTracing()
{
	instance = this;

	MaterialManager* mm = _core->GetMaterialManager();
	mm->AddCallbackMaterialChanged(onMaterialChanged);

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

std::pair<size_t, size_t> sceneTriangleCount(Render::RenderScene& scene)
{
	size_t tris = 0;
	size_t mats = 1;

	for (int i = 0; i < scene.meshes.size(); ++i)
	{
		Render::RenderMesh& r = scene.meshes[i];

		if (r.mesh->isStd() && !r.mesh->isPlane())
			continue;

		std::shared_ptr<RaytracingData> rtWorldTriangles = r.model->GetRaytracingData(0);
		tris += rtWorldTriangles->size();
		mats++;
	}

	return std::make_pair(tris, mats);
}

std::shared_ptr<RaytracingData> RenderPathPathTracing::getScene(Render::RenderScene& scene, size_t triangles)
{
	std::shared_ptr<RaytracingData> ret = std::make_shared<RaytracingData>(triangles);
	GPURaytracingTriangle* tringlesData = ret->triangles.data();

	matPointerToIndex.clear();

	// Default diffuse material
	{
		GPUMaterial& gpuMat = ret->materials.emplace_back();
		gpuMat.albedo = vec4{ 1,1,1,1 };
		gpuMat.shading.x = 1;
		gpuMat.shading.y = 1;
		matPointerToIndex[nullptr] = 0;
	}

	for (int i = 0; i < scene.meshes.size(); ++i)
	{
		Render::RenderMesh& r = scene.meshes[i];

		if (r.mesh->isStd() && !r.mesh->isPlane())
			continue;

		uint matID = 0;
		Material* mat = r.model->GetMaterial();

		if (mat)
		{
			if (auto it = matPointerToIndex.find(mat); it != matPointerToIndex.end())
				matID = it->second;
			else
			{
				matID = ret->materials.size();
				GPUMaterial& gpuMat = ret->materials.emplace_back();
				fillGPUMaterial(gpuMat, mat);
				matPointerToIndex[mat] = matID;
			}
		}

		std::shared_ptr<RaytracingData> rtWorldTriangles = r.model->GetRaytracingData(matID);

		size_t bytes = sizeof(GPURaytracingTriangle) * rtWorldTriangles->size();
		memcpy(tringlesData, rtWorldTriangles->triangles.data(), bytes);
		tringlesData += rtWorldTriangles->triangles.size();
	}

	gpuMaterials = ret->materials;

	return ret;
}

void RenderPathPathTracing::fillGPUMaterial(GPUMaterial& gpuMat, Material* mat)
{
	gpuMat.albedo = mat->GetParamFloat4("base_color");
	gpuMat.shading.x = mat->GetParamFloat("metalness");
	gpuMat.shading.y = mat->GetParamFloat("roughness");
}

void RenderPathPathTracing::uploadScene(Render::RenderScene& scene)
{
	// 1. Triangles & material
	{
		auto [triagles, mats] = sceneTriangleCount(scene);
		size_t bufferLen = triagles * sizeof(GPURaytracingTriangle);

		if (trianglesCount < (uint32_t)triagles)
			trianglesBuffer = RES_MAN->CreateStructuredBuffer((uint)bufferLen, sizeof(GPURaytracingTriangle), BUFFER_USAGE::GPU_READ);
		trianglesCount = (uint32_t)triagles;

		auto trianglesPtr = getScene(scene, triagles);
		trianglesBuffer->SetData((uint8*)trianglesPtr->triangles.data(), bufferLen);

		uploadMaterials(mats);
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

			float S = cross((vec3)areaLightData[i].p1 - (vec3)areaLightData[i].p0, (vec3)areaLightData[i].p3 - (vec3)areaLightData[i].p0).Lenght();
			areaLightData[i].color = vec4(1.0f) * scene.areaLights[i].intensity / S;
		}

		areaLightBuffer->SetData((uint8*)areaLightData.data(), bufferLen);
	}
}

void RenderPathPathTracing::uploadMaterials(size_t mats)
{
	if (materialsCount < mats)
		materialsBuffer = RES_MAN->CreateStructuredBuffer((uint)sizeof(GPUMaterial) * mats, sizeof(GPUMaterial), BUFFER_USAGE::GPU_READ);
	materialsCount = mats;
	materialsBuffer->SetData((uint8*)gpuMaterials.data(), sizeof(GPUMaterial) * mats);
}

void RenderPathPathTracing::RenderFrame()
{
	Texture* color = render->GetPrevRenderTexture(PREV_TEXTURES::PATH_TRACING_HDR, width, height, TEXTURE_FORMAT::RGBA16F);

	Render::RenderScene scene = render->getRenderScene();
	uint32_t nextcrc = scene.getHash();

	if (!out || out->GetHeight() != height || out->GetWidth() != width)
		out = RES_MAN->CreateTexture(width, height, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::RGBA32F,
									 TEXTURE_CREATE_FLAGS::USAGE_UNORDRED_ACCESS | TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET);

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

		//Log("Scene upload. Hash: %u\n", crc_);
	}

	if (needUploadMaterials)
	{
		if (auto it = matPointerToIndex.find(materialChanged); it != matPointerToIndex.end())
		{
			fillGPUMaterial(gpuMaterials[it->second], materialChanged);
			uploadMaterials(materialsCount);
		}

		clearHDRbuffer();
		fillDepthBuffer(scene);

		needUploadMaterials = false;
		materialChanged = nullptr;
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
		CORE_RENDER->BindStructuredBuffer(2, materialsBuffer.get());

		Texture* uavs[] = { out.get() };
		CORE_RENDER->CSBindUnorderedAccessTextures(1, uavs);

		constexpr uint warpSize = 16;
		int numGroupsX = (width + (warpSize - 1)) / warpSize;
		int numGroupsY = (height + (warpSize - 1)) / warpSize;

		CORE_RENDER->Dispatch(numGroupsX, numGroupsY, 1);

		CORE_RENDER->CSBindUnorderedAccessTextures(1, nullptr);
	}

	// emblem
	{
		Texture* rts_out[1] = { out.get() };
		CORE_RENDER->SetRenderTextures(1, rts_out, CORE_RENDER->GetSurfaceDepthTexture());

		draw_AreaLightEmblems(scene);
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

	CORE_RENDER->TimersEndPoint(frameID_, Render::T_PATH_TRACING_DRAW);
	drawMS = CORE_RENDER->GetTimeInMsForPoint(readbackFrameID_, Render::T_PATH_TRACING_DRAW);

	render->RenderGUI();
}

