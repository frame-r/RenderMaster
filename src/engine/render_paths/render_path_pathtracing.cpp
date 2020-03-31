#include "pch.h"
#include "render_path_pathtracing.h"
#include "core.h"
#include "render.h"
#include "shader.h"
#include "material.h"
#include "light.h"
#include "mesh.h"
#include "icorerender.h"
#include "material_manager.h"

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
		Log("Scene changed %u\n", crc_);
	}

	//if (_core->frame() == 100)

	for(int i = 0; i < scene.meshes.size(); ++i)
	{
		Render::RenderMesh& r = scene.meshes[i];

		if (r.mesh->isStd())
			continue;

		std::shared_ptr rtData = r.mesh->GetRaytracingData(r.worldTransformMat);
	}

	//render->updateEnvirenment(scene);

	uint32 frameID_ = render->frameID();
	uint32 readbackFrameID_ = render->readbackFrameID();

	{
		CORE_RENDER->TimersBeginPoint(frameID_, Render::T_PATH_TRACING_DRAW);

		//compositeMaterial->SetDef("specular_quality", render->GetSpecularQuality());
		//compositeMaterial->SetDef("environment_type", static_cast<int>(render->GetEnvironmentType()));

		//if (auto shader = pathtracingDrawMaterial->GetShader(render->fullScreen()))
		{
			//Texture* rts[1] = { color };
			//CORE_RENDER->SetRenderTextures(1, rts, nullptr);
			Texture* rts[1] = { CORE_RENDER->GetSurfaceColorTexture() };
			CORE_RENDER->SetRenderTextures(1, rts, CORE_RENDER->GetSurfaceDepthTexture());
			CORE_RENDER->Clear();

			//CORE_RENDER->SetShader(shader);
			CORE_RENDER->SetDepthTest(1);

			//constexpr int tex_count = 7;
			//Texture* texs[tex_count] = {
			//	buffers.albedo,
			//	buffers.normal,
			//	buffers.shading,
			//	buffers.diffuseLight,
			//	buffers.specularLight,
			//	buffers.depth,
			//	render->GetEnvironmentTexture()
			//};

			drawMeshes(pathtracingPreviewMaterial, scene.meshes, mats.ViewProjUnjitteredMat_, scene.sun_direction);

			//CORE_RENDER->BindTextures(tex_count, texs);
			//{
			//	CORE_RENDER->Draw(render->fullScreen(), 1);
			//}
			//CORE_RENDER->BindTextures(tex_count, nullptr);
		}

		CORE_RENDER->TimersEndPoint(frameID_, Render::T_PATH_TRACING_DRAW);
		drawMS = CORE_RENDER->GetTimeInMsForPoint(readbackFrameID_, Render::T_PATH_TRACING_DRAW);
	}

	// Restore default render target
	//Texture* rts[1] = { CORE_RENDER->GetSurfaceColorTexture() };
	//CORE_RENDER->SetRenderTextures(1, rts, CORE_RENDER->GetSurfaceDepthTexture());

	// Final copy
	//{
	//	finalPostMaterial->SetDef("view_mode", (int)render->GetViewMode());

	//	if (auto shader = finalPostMaterial->GetShader(render->fullScreen()))
	//	{
	//		constexpr int tex_count = 8;
	//		Texture* texs[tex_count] = {
	//			buffers.albedo,
	//			buffers.normal,
	//			buffers.shading,
	//			buffers.diffuseLight,
	//			buffers.specularLight,
	//			buffers.velocity,
	//			buffers.color,
	//			colorReprojection ? buffers.colorReprojected : nullptr
	//		};
	//		int tex_bind = tex_count;
	//		if (!colorReprojection) tex_bind--;

	//		CORE_RENDER->BindTextures(tex_bind, texs);
	//		CORE_RENDER->SetShader(shader);
	//		CORE_RENDER->Draw(render->fullScreen(), 1);
	//		CORE_RENDER->BindTextures(tex_bind, nullptr);
	//	}
	//}

	render->RenderGUI();
}

