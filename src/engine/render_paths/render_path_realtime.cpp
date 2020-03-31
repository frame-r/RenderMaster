#include "pch.h"
#include "render_path_realtime.h"
#include "core.h"
#include "render.h"
#include "shader.h"
#include "material.h"
#include "light.h"
#include "icorerender.h"
#include "material_manager.h"

void drawMeshes(PASS pass, std::vector<Render::RenderMesh>& meshes, mat4 VP, mat4 VP_Prev)
{
	for (Render::RenderMesh& renderMesh : meshes)
	{
		Material* mat = renderMesh.mat;
		if (!mat)
			continue;

		Shader* shader = nullptr;

		if (pass == PASS::DEFERRED)
			shader = mat->GetDeferredShader(renderMesh.mesh);
		else if (pass == PASS::ID)
			shader = mat->GetIdShader(renderMesh.mesh);
		else if (pass == PASS::WIREFRAME)
			shader = mat->GetWireframeShader(renderMesh.mesh);

		if (!shader)
			continue;

		CORE_RENDER->SetShader(shader);

		mat->UploadShaderParameters(shader, pass);
		mat->BindShaderTextures(shader, pass);

		mat4 MVP = VP * renderMesh.worldTransformMat;
		mat4 MVP_prev = VP_Prev * renderMesh.worldTransformMatPrev;
		mat4 M = renderMesh.worldTransformMat;
		mat4 NM = M.Inverse().Transpose();

		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetMat4Parameter("MVP_prev", &MVP_prev);
		shader->SetMat4Parameter("M", &M);
		shader->SetMat4Parameter("NM", &NM);

		if (pass == PASS::ID)
			shader->SetUintParameter("id", renderMesh.modelId);

		shader->FlushParameters();

		CORE_RENDER->Draw(renderMesh.mesh, 1);
	}
}


RenderPathRealtime::RenderPathRealtime()
{
	MaterialManager* mm = _core->GetMaterialManager();

	compositeMaterial = mm->CreateInternalMaterial("composite");
	assert(compositeMaterial);

	finalPostMaterial = mm->CreateInternalMaterial("final_post");
	assert(finalPostMaterial);

}

std::string RenderPathRealtime::getString(uint i)
{
	switch (i)
	{
		case 0: return "Frame GPU: " + std::to_string(frameMs); break;
		case 1: return "GBuffer GPU: " + std::to_string(gbufferMs); break;
		case 2: return "Lights GPU: " + std::to_string(lightsMs); break;
		case 3: return "Composite GPU: " + std::to_string(compositeMs); break;
	}
	return "";
}

void RenderPathRealtime::RenderFrame()
{
	uint w, h;
	CORE_RENDER->GetViewport(&w, &h);

	bool colorReprojection = render->GetViewMode() == VIEW_MODE::COLOR_REPROJECTION || render->IsTAA();

	RenderBuffers buffers;
	buffers.color = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);
	buffers.velocity = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RG16F);
	buffers.depth = CORE_RENDER->GetSurfaceDepthTexture();
	buffers.albedo = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);
	buffers.diffuseLight = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA16F);
	buffers.specularLight = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA16F);
	buffers.normal = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA32F);
	buffers.shading = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);
	if (colorReprojection)
		buffers.colorReprojected = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);

	Texture* colorPrev = render->GetPrevRenderTexture(PREV_TEXTURES::COLOR, w, h, TEXTURE_FORMAT::RGBA8);

	Render::RenderScene scene = render->getRenderScene();

	render->updateEnvirenment(scene);

	// Sky velocity
	if (Shader *shader = render->GetShader("sky_velocity.hlsl", render->fullScreen()))
	{
		CORE_RENDER->SetShader(shader);

		CORE_RENDER->SetRenderTextures(1, &buffers.velocity, nullptr);
		CORE_RENDER->Clear();

		mat4 m = prevMats.ViewMat_;
		m.SetColumn3(3, vec3(0, 0, 0)); // remove translation
		m = prevMats.ProjUnjitteredMat_ * prevMats.ViewMat_;
		shader->SetMat4Parameter("VP_prev", &m);

		m = mats.ViewUnjitteredMat_;
		m.SetColumn3(3, vec3(0, 0, 0));
		m = mats.ProjUnjitteredMat_ * m;
		m = m.Inverse();
		shader->SetMat4Parameter("VP_inv", &m);

		shader->FlushParameters();

		CORE_RENDER->Draw(render->fullScreen(), 1);

		CORE_RENDER->SetRenderTextures(1, nullptr, nullptr);
	}

	uint32 frameID_ = render->frameID();
	uint32 readbackFrameID_ = render->readbackFrameID();


	// G-buffer
	{
		CORE_RENDER->TimersBeginPoint(frameID_, Render::T_GBUFFER);

		Texture* texs[4] = { buffers.albedo, buffers.shading, buffers.normal, buffers.velocity };
		CORE_RENDER->SetRenderTextures(3, texs, buffers.depth);
		CORE_RENDER->Clear();

		CORE_RENDER->SetRenderTextures(4, texs, buffers.depth);

		CORE_RENDER->SetDepthTest(1);
		{
			drawMeshes(PASS::DEFERRED, scene.meshes, mats.ViewProjMat_, cameraPrevViewProjMatRejittered_);
		}
		CORE_RENDER->SetRenderTextures(4, nullptr, nullptr);

		CORE_RENDER->SetDepthTest(0);

		CORE_RENDER->TimersEndPoint(frameID_, Render::T_GBUFFER);
		gbufferMs = CORE_RENDER->GetTimeInMsForPoint(readbackFrameID_, Render::T_GBUFFER);
	}

	// Color reprojection
	if (colorReprojection)
	{
		Shader* shader = render->GetShader("reprojection.hlsl", render->fullScreen());
		if (shader)
		{
			Texture* texs_rt[1] = { buffers.colorReprojected };
			CORE_RENDER->SetRenderTextures(1, texs_rt, nullptr);

			Texture* texs[2] = { colorPrev, buffers.velocity };
			CORE_RENDER->BindTextures(2, texs);
			CORE_RENDER->SetShader(shader);

			vec4 s((float)w, (float)h, 0, 0);
			shader->SetVec4Parameter("bufer_size", &s);
			shader->FlushParameters();

			CORE_RENDER->Draw(render->fullScreen(), 1);
			CORE_RENDER->BindTextures(2, nullptr);

			CORE_RENDER->SetRenderTextures(1, nullptr, nullptr);
		}
	}

	// Lights
	{
		CORE_RENDER->TimersBeginPoint(frameID_, Render::T_LIGHTS);

		CORE_RENDER->SetDepthTest(0);
		Texture* rts[2] = { buffers.diffuseLight, buffers.specularLight };
		CORE_RENDER->SetRenderTextures(2, rts, nullptr);
		CORE_RENDER->Clear();

		Shader* shader = render->GetShader("deferred_light.hlsl", render->fullScreen());
		if (shader && scene.lights.size())
		{
			CORE_RENDER->SetShader(shader);

			shader->SetVec4Parameter("camera_position", &mats.WorldPos_);
			shader->SetMat4Parameter("camera_view_projection_inv", &mats.ViewProjInvMat_);

			CORE_RENDER->SetBlendState(BLEND_FACTOR::ONE, BLEND_FACTOR::ONE_MINUS_SRC_ALPHA);
			Texture* texs[4] = { buffers.normal, buffers.shading, buffers.albedo, buffers.depth };
			CORE_RENDER->BindTextures(4, texs);

			for (Render::RenderLight& renderLight : scene.lights)
			{
				vec4 lightColor(renderLight.light->GetIntensity());
				vec4 dir = vec4(renderLight.worldDirection);

				shader->SetVec4Parameter("light_color", &lightColor);
				shader->SetVec4Parameter("light_direction", &dir);
				shader->FlushParameters();

				CORE_RENDER->Draw(render->fullScreen(), 1);
			}
			CORE_RENDER->BindTextures(4, nullptr);
			CORE_RENDER->SetBlendState(BLEND_FACTOR::NONE, BLEND_FACTOR::NONE);
		}

		CORE_RENDER->SetRenderTextures(2, nullptr, nullptr);

		CORE_RENDER->TimersEndPoint(frameID_, Render::T_LIGHTS);
		lightsMs = CORE_RENDER->GetTimeInMsForPoint(readbackFrameID_, Render::T_LIGHTS);

	}

	// Composite
	{
		CORE_RENDER->TimersBeginPoint(frameID_, Render::T_COMPOSITE);

		compositeMaterial->SetDef("specular_quality", render->GetSpecularQuality());
		compositeMaterial->SetDef("environment_type", static_cast<int>(render->GetEnvironmentType()));

		if (auto shader = compositeMaterial->GetShader(render->fullScreen()))
		{
			Texture* rts[1] = { buffers.color };
			CORE_RENDER->SetRenderTextures(1, rts, nullptr);
			CORE_RENDER->SetShader(shader);

			vec4 environment_resolution;
			render->GetEnvironmentResolution(environment_resolution);
			shader->SetVec4Parameter("environment_resolution", &environment_resolution);

			vec4 environment_intensity;
			render->GetEnvironmentIntensity(environment_intensity);
			shader->SetVec4Parameter("environment_intensity", &environment_intensity);

			shader->SetVec4Parameter("camera_position", &mats.WorldPos_);

			shader->SetMat4Parameter("camera_view_projection_inv", &mats.ViewProjInvMat_);

			vec4 sun_disrection = vec4(0, 0, 1, 0);
			if (scene.hasWorldLight)
			{
				//RenderVector(scene.lights[0].worldDirection, vec4(1, 0, 0, 1));
				sun_disrection = scene.lights[0].worldDirection;
			}
			shader->SetVec4Parameter("sun_disrection", &sun_disrection);

			shader->FlushParameters();

			CORE_RENDER->SetDepthTest(0);

			constexpr int tex_count = 7;
			Texture* texs[tex_count] = {
				buffers.albedo,
				buffers.normal,
				buffers.shading,
				buffers.diffuseLight,
				buffers.specularLight,
				buffers.depth,
				render->GetEnvironmentTexture()
			};

			CORE_RENDER->BindTextures(tex_count, texs);
			{
				CORE_RENDER->Draw(render->fullScreen(), 1);
			}
			CORE_RENDER->BindTextures(tex_count, nullptr);
		}

		CORE_RENDER->TimersEndPoint(frameID_, Render::T_COMPOSITE);
		compositeMs = CORE_RENDER->GetTimeInMsForPoint(readbackFrameID_, Render::T_COMPOSITE);
	}

	// TAA
	if (render->IsTAA())
		if (Shader * shader = render->GetShader("taa.hlsl", render->fullScreen()))
		{
			Texture* taaOut = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);

			Texture* texs_rt[1] = { taaOut };
			CORE_RENDER->SetRenderTextures(1, texs_rt, nullptr);

			Texture* texs[2] = { buffers.color, buffers.colorReprojected };
			CORE_RENDER->BindTextures(2, texs);
			CORE_RENDER->SetShader(shader);

			CORE_RENDER->Draw(render->fullScreen(), 1);
			CORE_RENDER->BindTextures(2, nullptr);

			CORE_RENDER->SetRenderTextures(1, nullptr, nullptr);

			std::swap(taaOut, buffers.color);
			render->ReleaseRenderTexture(taaOut);
		}

	// Restore default render target
	Texture* rts[1] = { CORE_RENDER->GetSurfaceColorTexture() };
	CORE_RENDER->SetRenderTextures(1, rts, CORE_RENDER->GetSurfaceDepthTexture());

	// Final copy
	{
		finalPostMaterial->SetDef("view_mode", (int)render->GetViewMode());

		if (auto shader = finalPostMaterial->GetShader(render->fullScreen()))
		{
			constexpr int tex_count = 8;
			Texture* texs[tex_count] = {
				buffers.albedo,
				buffers.normal,
				buffers.shading,
				buffers.diffuseLight,
				buffers.specularLight,
				buffers.velocity,
				buffers.color,
				colorReprojection ? buffers.colorReprojected : nullptr
			};
			int tex_bind = tex_count;
			if (!colorReprojection) tex_bind--;

			CORE_RENDER->BindTextures(tex_bind, texs);
			CORE_RENDER->SetShader(shader);
			CORE_RENDER->Draw(render->fullScreen(), 1);
			CORE_RENDER->BindTextures(tex_bind, nullptr);
		}
	}

	//renderGrid();

	// Wireframe
	/*if (wireframeModels && IsWireframe())
	{
		Texture* wireframe_depth = CORE_RENDER->GetSurfaceDepthTexture();

		const bool DepthTest = true;
		CORE_RENDER->SetDepthTest(DepthTest);

		// WTF? Why original depth buffer don't without TAA
		if (DepthTest)
		{
			if (Shader * shader = render->GetShader("depth_indentation.hlsl", render->fullScreen()))
			{
				CORE_RENDER->SetDepthFunc(DEPTH_FUNC::ALWAYS);
				CORE_RENDER->SetBlendState(BLEND_FACTOR::NONE, BLEND_FACTOR::NONE);

				wireframe_depth = GetRenderTexture(w, h, TEXTURE_FORMAT::D24S8);
				CORE_RENDER->SetRenderTextures(1, nullptr, wireframe_depth);

				Texture* texs[1] = { CORE_RENDER->GetSurfaceDepthTexture() };
				CORE_RENDER->BindTextures(1, texs);

				CORE_RENDER->SetShader(shader);
				CORE_RENDER->Draw(render->fullScreen(), 1);

				CORE_RENDER->BindTextures(1, nullptr);

				CORE_RENDER->SetDepthFunc(DEPTH_FUNC::LESS_EQUAL);
			}

			// remove jitter form proj matrix
			mats.ViewProjMat_ = ProjMat * ViewMat;
			mats.ViewProjInvMat_ = mats.ViewProjMat_.Inverse();
		}

		CORE_RENDER->SetFillingMode(FILLING_MODE::WIREFRAME);
		CORE_RENDER->SetBlendState(BLEND_FACTOR::SRC_ALPHA, BLEND_FACTOR::ONE_MINUS_SRC_ALPHA);

		std::vector<RenderMesh> meshes;

		for (int i = 0; i < modelsNum; ++i)
		{
			Model* m = wireframeModels[i];
			Mesh* mesh = m->GetMesh();

			if (!mesh)
				continue;

			meshes.emplace_back(RenderMesh{ m->GetId(), mesh, m->GetMaterial(), m->GetWorldTransform(), m->GetWorldTransformPrev() });
		}

		// render to MSAA RT + resolve

		if (IsWireframeAA())
		{
			Texture* msaaTex = { GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8, 4) };
			CORE_RENDER->SetRenderTextures(1, &msaaTex, nullptr);

			CORE_RENDER->Clear();

			CORE_RENDER->SetBlendState(BLEND_FACTOR::NONE, BLEND_FACTOR::NONE);
			CORE_RENDER->SetDepthTest(0);

			CORE_RENDER->BindTextures(1, &wireframe_depth);

			drawMeshes(PASS::WIREFRAME, meshes);

			CORE_RENDER->SetDepthTest(1);
			CORE_RENDER->SetBlendState(BLEND_FACTOR::ONE, BLEND_FACTOR::ONE_MINUS_SRC_ALPHA);
			CORE_RENDER->SetFillingMode(FILLING_MODE::SOLID);

			// Restore default render target
			Texture* rts_[1] = { CORE_RENDER->GetSurfaceColorTexture() };
			CORE_RENDER->SetRenderTextures(1, rts_, CORE_RENDER->GetSurfaceDepthTexture());

			Shader* shader = render->GetShader("msaa_resolve.hlsl", render->fullScreen());
			CORE_RENDER->SetShader(shader);

			CORE_RENDER->BindTextures(1, &msaaTex, BIND_TETURE_FLAGS::PIXEL);
			CORE_RENDER->Draw(render->fullScreen(), 1);
			CORE_RENDER->BindTextures(1, nullptr, BIND_TETURE_FLAGS::PIXEL);

			ReleaseRenderTexture(msaaTex);
		}
		else
		{
			Texture* rts_[1] = { CORE_RENDER->GetSurfaceColorTexture() };
			CORE_RENDER->SetRenderTextures(1, rts_, nullptr);

			CORE_RENDER->BindTextures(1, &wireframe_depth);
			drawMeshes(PASS::WIREFRAME, meshes);
			CORE_RENDER->BindTextures(1, nullptr);

			CORE_RENDER->SetFillingMode(FILLING_MODE::SOLID);
		}

		if (wireframe_depth != CORE_RENDER->GetSurfaceDepthTexture())
		{
			ReleaseRenderTexture(wireframe_depth);
		}

		// Restore default render target
		Texture* rts_[1] = { CORE_RENDER->GetSurfaceColorTexture() };
		CORE_RENDER->SetRenderTextures(1, rts_, nullptr);

		CORE_RENDER->SetBlendState(BLEND_FACTOR::NONE, BLEND_FACTOR::NONE);
	}
	*/

	// Vectors
	/*
	if (renderVectors.size())
	{
		CORE_RENDER->SetDepthTest(1);

		if (auto shader = render->GetShader("primitive.hlsl", lineMesh.get()))
		{
			CORE_RENDER->SetShader(shader);

			for (auto& v : renderVectors)
			{
				shader->SetVec4Parameter("main_color", &v.color);
				mat4 transform(0.0f);
				transform.el_2D[0][0] = v.v.x;
				transform.el_2D[1][0] = v.v.y;
				transform.el_2D[2][0] = v.v.z;
				mat4 MVP = prev.ViewProjMat_ * transform;
				shader->SetMat4Parameter("MVP", &MVP);
				shader->FlushParameters();

				CORE_RENDER->Draw(lineMesh.get(), 1);
			}
		}
	}
	*/
	render->RenderGUI();

	buffers.depth = nullptr;
	render->ReleaseRenderTexture(buffers.color);
	render->ReleaseRenderTexture(buffers.albedo);
	render->ReleaseRenderTexture(buffers.diffuseLight);
	render->ReleaseRenderTexture(buffers.specularLight);
	render->ReleaseRenderTexture(buffers.normal);
	render->ReleaseRenderTexture(buffers.shading);
	render->ReleaseRenderTexture(buffers.velocity);
	if (colorReprojection)
		render->ReleaseRenderTexture(buffers.colorReprojected);

	render->ExchangePrevRenderTexture(colorPrev, buffers.color);
}

