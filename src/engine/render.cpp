#include "pch.h"
#include "render.h"
#include "shader.h"
#include "core.h"
#include "console.h"
#include "resource_manager.h"
#include "light.h"
#include "material.h"
#include "material_manager.h"
#include "model.h"
#include "camera.h"
#include "filesystem.h"
#include "thirdparty/simplecpp/SimpleCpp.h"
#include <memory>
#include <sstream>


static std::unordered_map<string, SharedPtr<Shader>> runtimeShaders; // defines -> Shader

static std::unordered_map<size_t, ViewData> viewsDataMap; // view ID -> Data


// One Profiler character
struct charr
{
	float data[4];
	uint32_t id;
	uint32_t __align[3];
};

// One profiler line
struct RenderProfileRecord
{
	size_t txtHash{};
	size_t length{};
	std::unique_ptr<charr[]> bufferData;
	SharedPtr<StructuredBuffer> buffer;
};

static std::vector<RenderProfileRecord> records;


struct RenderTexture
{
	int64_t frame;
	int free;
	uint width;
	uint height;
	TEXTURE_FORMAT format;
	int msaaSamples;
	SharedPtr<Texture> pointer;
	PREV_TEXTURES id{ PREV_TEXTURES ::UNKNOWN};
};

static vector<RenderTexture> renderTextures;
static vector<RenderTexture> prevRenderTextures;

static const uint fontWidth[256] =
{
 8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8, 8 ,8 ,8 ,8 ,0 ,8 ,8 ,8 ,8 ,8, 8
,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8, 8, 8 ,8 ,4 ,4 ,5 ,8 ,7 ,1 ,10,3
,4, 4, 5, 9, 3, 5, 3, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 3, 3
,9, 8, 9, 6,12, 9, 8, 8, 9, 8, 7, 9, 10,3, 7, 8, 8,12,10,10
,8,10, 9, 9, 8, 9, 9,13, 8, 9, 7, 4, 5, 4, 9, 5, 3, 7, 8, 6
,8, 7, 4, 8, 7, 3, 3, 6, 3,11, 7, 8, 8, 8, 4, 6, 4, 7, 6, 9
,6, 6, 6, 4, 3, 4, 9, 8, 7, 8, 3, 7, 5, 9, 5, 5, 5,16, 7, 4
,12,8, 7, 8, 8, 3, 3, 5, 5, 5, 7,13, 4,10, 6, 4, 12,8, 6, 7
,4, 4, 7, 7, 7, 7, 3, 6, 5,12, 5, 7, 9, 5,12, 5, 5, 9, 5, 5
,4, 8, 6, 3, 3, 5, 6, 7,12,12, 12,6, 8, 8, 8, 8, 8, 8,11, 8
,7, 7, 7, 7, 3, 3, 3, 3, 9,10, 10,10,10,10,10,9,10, 9, 9, 9
,9, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 6,7, 7, 7, 7, 3, 3, 3, 3
,7, 7, 8, 8, 8, 8, 8, 9, 8, 7, 7, 7, 7, 6, 8, 6
};

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


template<typename T>
void addObjectsRecursive(std::vector<T*>& ret, GameObject *root, OBJECT_TYPE type)
{
	size_t childs = root->GetNumChilds();

	for (size_t i = 0; i < childs; i++)
	{
		GameObject *g = root->GetChild(i);
		addObjectsRecursive<T>(ret, g, type);
	}

	if (root->GetType() == type && root->IsEnabled())
		ret.push_back(static_cast<T*>(root));
}

template<typename T>
void getObjects(vector<T*>& vec, OBJECT_TYPE type)
{
	size_t objects = RES_MAN->GetNumObjects();

	for (size_t i = 0; i < objects; i++)
	{
		T *g = static_cast<T*>(RES_MAN->GetObject_(i));
		addObjectsRecursive<T>(vec, g, type);
	}
}

auto Render::GetShader(const char *path, Mesh *mesh, const vector<string>& defines) ->Shader*
{
	SharedPtr<Shader> shader;

	INPUT_ATTRUBUTE attrib = INPUT_ATTRUBUTE::UNKNOWN;

	if (mesh)
		attrib = mesh->GetAttributes();

	string shaderKey = string(path) + '-';

	for(const string& def : defines)
		shaderKey += def;

	if ((int)(attrib & INPUT_ATTRUBUTE::NORMAL)) shaderKey += "ENG_INPUT_NORMAL";
	if ((int)(attrib & INPUT_ATTRUBUTE::TEX_COORD)) shaderKey += "ENG_INPUT_TEXCOORD";
	if ((int)(attrib & INPUT_ATTRUBUTE::COLOR)) shaderKey += "ENG_INPUT_COLOR";

	auto it = runtimeShaders.find(shaderKey);

	if (it != runtimeShaders.end())
	{
		return it->second.get();
	}
	else
	{
		const auto process_shader = [&](const char *&ppTextOut, const char *ppTextIn, const string& fullPath, const string&& fileNameOut, int type) -> void
		{
			simplecpp::DUI dui;

			copy(begin(defines), end(defines), back_inserter(dui.defines));

			if ((int)(attrib & INPUT_ATTRUBUTE::NORMAL)) dui.defines.push_back("ENG_INPUT_NORMAL");
			if ((int)(attrib & INPUT_ATTRUBUTE::TEX_COORD)) dui.defines.push_back("ENG_INPUT_TEXCOORD");
			if ((int)(attrib & INPUT_ATTRUBUTE::COLOR)) dui.defines.push_back("ENG_INPUT_COLOR");

			if (type == 0)
				dui.defines.push_back("ENG_SHADER_VERTEX");
			else if (type == 1)
				dui.defines.push_back("ENG_SHADER_PIXEL");
			else if (type == 2)
				dui.defines.push_back("ENG_SHADER_GEOMETRY");
			
			simplecpp::OutputList outputList;
			std::vector<std::string> files;
			string textIn = ppTextIn;
			std::stringstream f(textIn);
			std::map<std::string, simplecpp::TokenList*> included;

			simplecpp::TokenList rawtokens(f, files, fullPath, &outputList);

			simplecpp::TokenList outputTokens(files);

			simplecpp::preprocess(outputTokens, rawtokens, files, included, dui, &outputList);
			const string out = outputTokens.stringify();
			auto size = out.size();

			char *tmp = new char[size + 1];
			strncpy(tmp, out.c_str(), size);
			tmp[size] = '\0';

			ppTextOut = tmp;
		};

		char *text;

		string dataDir = _core->GetDataPath();
		string fileIn = dataDir + '\\' + string(SHADER_DIR) + path;

		File f = FS->OpenFile(fileIn.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
		size_t size = f.FileSize();
		text = new char[size + 1];
		*(text + size) = '\0';
		f.Read((uint8*)text, size);

		const char *textVertParced;
		process_shader(textVertParced, text, fileIn, "out_v.shader", 0);

		const char *textFragParced;
		process_shader(textFragParced, text, fileIn, "out_f.shader", 1);

		shader = RES_MAN->CreateShader(textVertParced, nullptr, textFragParced);

		if (!shader)
			LogCritical("Render::GetShader(): can't compile %s standard shader", path);

		runtimeShaders.emplace(shaderKey, shader);
	}
	return shader.get();
}

auto DLLEXPORT Render::GetComputeShader(const char* path, const vector<string>& defines) -> Shader*
{
	SharedPtr<Shader> shader;

	string shaderKey = string(path);

	for (const string& def : defines)
		shaderKey += def;


	auto it = runtimeShaders.find(shaderKey);

	if (it != runtimeShaders.end())
	{
		return it->second.get();
	}
	else
	{
		const auto process_shader = [&](const char*& ppTextOut, const char* ppTextIn, const string& fullPath, const string&& fileNameOut, int type) -> void
		{
			simplecpp::DUI dui;

			copy(begin(defines), end(defines), back_inserter(dui.defines));

			simplecpp::OutputList outputList;
			std::vector<std::string> files;
			string textIn = ppTextIn;
			std::stringstream f(textIn);
			std::map<std::string, simplecpp::TokenList*> included;

			simplecpp::TokenList rawtokens(f, files, fullPath, &outputList);

			simplecpp::TokenList outputTokens(files);

			simplecpp::preprocess(outputTokens, rawtokens, files, included, dui, &outputList);
			const string out = outputTokens.stringify();
			auto size = out.size();

			char* tmp = new char[size + 1];
			strncpy(tmp, out.c_str(), size);
			tmp[size] = '\0';

			ppTextOut = tmp;
		};

		char* text;

		string dataDir = _core->GetDataPath();
		string fileIn = dataDir + '\\' + string(SHADER_DIR) + path;

		File f = FS->OpenFile(fileIn.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
		size_t size = f.FileSize();
		text = new char[size + 1];
		*(text + size) = '\0';
		f.Read((uint8*)text, size);

		const char* textVertParced;
		process_shader(textVertParced, text, fileIn, "out_v.shader", 0);

		shader = RES_MAN->CreateComputeShader(textVertParced);

		if (!shader)
			LogCritical("Render::GetShader(): can't compile %s standard shader", path);

		runtimeShaders.emplace(shaderKey, shader);
	}
	return shader.get();

}

auto DLLEXPORT Render::ReloadShaders() -> void
{
	Log("Reloading shaders...");
	runtimeShaders.clear();
}

auto DLLEXPORT Render::RenderGUI() -> void
{
	if (!_core->IsProfiler())
		return;

	INPUT_ATTRUBUTE attribs = planeMesh.get()->GetAttributes();

	Shader *shader = GetShader("font.hlsl");
	if (!shader)
		return;

	CORE_RENDER->SetShader(shader);

	uint w, h;
	CORE_RENDER->GetViewport(&w, &h);

	shader->SetFloatParameter("invHeight2", 2.0f / h);
	shader->SetFloatParameter("invWidth2", 2.0f / w);
	shader->FlushParameters();

	Texture *texs[1] = {fontTexture.get()};
	CORE_RENDER->BindTextures(1, texs);
	CORE_RENDER->SetDepthTest(0);

	float offsetVert = 0.0f;
	int lines = 0;

	for (int i = 0; i < _core->ProfilerCallbacks(); i++)
	{
		IProfilerCallback *c = _core->getCallback(i);

		for (auto j = 0u; j < c->getNumLines(); j++)
		{
			if (lines >= records.size())
				records.push_back(RenderProfileRecord());

			RenderProfileRecord &r = records[lines];

			string line = c->getString(j);

			if (line.size() > 0)
			{
				// recreate buffer
				if (!r.buffer || r.length < line.size())
				{
					r.length = line.size();
					r.buffer = RES_MAN->CreateStructuredBuffer((uint)r.length * sizeof(charr), sizeof(charr));
					r.bufferData = unique_ptr<charr[]>(new charr[r.length]);
				}	

				std::hash<string> hashFn;
				size_t newHash = hashFn(line);

				// upload data
				if (newHash != r.txtHash)
				{
					float offset = 0.0f;
					for (size_t i = 0u; i < line.size(); i++)
					{
						float w = static_cast<float>(fontWidth[line[i]]);
						r.bufferData[i].data[0] = w;
						r.bufferData[i].data[1] = offset;
						r.bufferData[i].data[2] = offsetVert;
						r.bufferData[i].id = static_cast<uint>(line[i]);
						offset += w;
					}
					r.txtHash = newHash;
					r.buffer->SetData(reinterpret_cast<uint8*>(&r.bufferData[0].data[0]), line.size() * sizeof(charr));
				}

				CORE_RENDER->BindStructuredBuffer(1, r.buffer.get());
				CORE_RENDER->Draw(planeMesh.get(), (uint)line.size());
			}

			offsetVert -= 17.0f;
			lines++;
		}
	}

	CORE_RENDER->BindTextures(1, nullptr);
	CORE_RENDER->BindStructuredBuffer(1, nullptr);
	CORE_RENDER->SetDepthTest(1);
}

vector<Render::RenderMesh> Render::getRenderMeshes()
{
	vector<RenderMesh> meshesVec;

	vector<Model*> models;
	getObjects(models, OBJECT_TYPE::MODEL);

	for (Model *model : models)
	{
		if (!model->IsEnabled())
			continue;

		Mesh *mesh = model->GetMesh();

		if (!mesh)
			continue;

		meshesVec.emplace_back(RenderMesh{model->GetId(), mesh, model->GetMaterial(), model->GetWorldTransform(), model->GetWorldTransformPrev()});
	}
	return meshesVec;
}

Render::RenderScene Render::getRenderScene()
{
	RenderScene scene;
	scene.meshes = getRenderMeshes();

	vector<Light*> lights;
	getObjects(lights, OBJECT_TYPE::LIGHT);

	for (Light *l : lights)
	{
		if (!l->IsEnabled())
			continue;

		vec3 worldDir = l->GetWorldTransform().Column3(2);
		worldDir.Normalize();

		scene.lights.emplace_back(RenderLight{l, worldDir});
	}
	scene.hasWorldLight = scene.lights.size() > 0;

	return scene;
}

auto DLLEXPORT Render::DrawMeshes(PASS pass) -> void
{
	vector<RenderMesh> meshes = getRenderMeshes();
	drawMeshes(pass, meshes);
}

auto DLLEXPORT Render::GetRenderTexture(uint width, uint height, TEXTURE_FORMAT format, int msaaSamples) -> Texture *
{
	auto it = std::find_if(renderTextures.begin(), renderTextures.end(),
		[width, height, format, msaaSamples](const RenderTexture& tex)
		{
			return tex.free && width == tex.width && height == tex.height && format == tex.format && msaaSamples == tex.msaaSamples;
		});

	if (it != renderTextures.end())
	{
		it->free = 0;
		it->frame = _core->frame();
		return it->pointer.get();
	}

	TEXTURE_CREATE_FLAGS flags = TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET | TEXTURE_CREATE_FLAGS::COORDS_WRAP | TEXTURE_CREATE_FLAGS::FILTER_POINT;

	switch (msaaSamples)
	{
		case 0:
		case 1: break;
		case 2: flags = flags | TEXTURE_CREATE_FLAGS::MSAA_2x; break;
		case 4: flags = flags | TEXTURE_CREATE_FLAGS::MSAA_4x; break;
		case 8: flags = flags | TEXTURE_CREATE_FLAGS::MSAA_8x; break;
		default: LogWarning("Render::GetRenderTexture(): Unknown number of MSAA samples"); break;
	}

	SharedPtr<Texture> tex = RES_MAN->CreateTexture(width, height, TEXTURE_TYPE::TYPE_2D, format, flags);

	renderTextures.push_back({_core->frame(), 0, width, height, format, msaaSamples, tex });

	return tex.get();
}

auto DLLEXPORT Render::ReleaseRenderTexture(Texture* tex) -> void
{
	std::for_each(renderTextures.begin(), renderTextures.end(), [tex](RenderTexture& t) { if (tex == t.pointer.get()) t.free = 1; });
}

void Render::RenderFrame(size_t viewID, const mat4& ViewMat, const mat4& ProjMat, Model** wireframeModels, int modelsNum)
{
	uint w, h;
	CORE_RENDER->GetViewport(&w, &h);

	cameraProjUnjitteredMat_ = ProjMat;
	cameraProjMat_ = ProjMat;

	ViewData& prev = viewsDataMap[viewID];

	vec2 taaOfffset;

	if (taa)
	{
		taaOfffset = taaSamples[_core->frame() % 16];
		taaOfffset = (taaOfffset * 2.0f - vec2(1, 1));

		float needJitter = float(viewMode == VIEW_MODE::FINAL);

		cameraProjMat_.el_2D[0][2] += needJitter * taaOfffset.x / w;
		cameraProjMat_.el_2D[1][2] += needJitter * taaOfffset.y / h;

		// rejitter prev
		if (_core->frame() > 1)
			cameraPrevViewProjMatRejittered_ = prev.cameraProjUnjitteredMat_;
		else
			cameraPrevViewProjMatRejittered_ = ProjMat;

		cameraPrevViewProjMatRejittered_.el_2D[0][2] += taaOfffset.x / w;
		cameraPrevViewProjMatRejittered_.el_2D[1][2] += taaOfffset.y / h;

		if (_core->frame() > 1)
			cameraPrevViewProjMatRejittered_ = cameraPrevViewProjMatRejittered_ * prev.cameraViewMat_;
		else
			cameraPrevViewProjMatRejittered_ = cameraPrevViewProjMatRejittered_ * ViewMat;
	}
	else
		cameraPrevViewProjMatRejittered_ = prev.cameraProjMat_ * prev.cameraViewMat_;
	
	cameraViewProjMat_			= cameraProjMat_ * ViewMat;
	cameraViewMat_				= ViewMat;
	cameraWorldPos_				= ViewMat.Inverse().Column3(3);
	cameraViewProjectionInvMat_ = cameraViewProjMat_.Inverse();
	cameraViewInvMat_			= cameraViewMat_.Inverse();

	// Restore prev matricies
	if (_core->frame() > 1)
	{
		cameraPrevProjUnjitteredMat_ = prev.cameraProjUnjitteredMat_;
		cameraPrevProjMat_ = prev.cameraProjMat_;
		cameraPrevViewProjMat_ = prev.cameraViewProjMat_;
		cameraPrevViewMat_ = prev.cameraViewMat_;
		cameraPrevWorldPos_ = prev.cameraWorldPos_;
		cameraPrevViewProjectionInvMat_ = prev.cameraViewProjectionInvMat_;
		cameraPrevViewInvMat_ = prev.cameraViewInvMat_;
	}
	else
	{
		cameraPrevProjUnjitteredMat_ = cameraProjUnjitteredMat_;
		cameraPrevProjMat_ = cameraProjMat_;
		cameraPrevViewProjMat_ = cameraViewProjMat_;
		cameraPrevViewMat_ = cameraViewMat_;
		cameraPrevWorldPos_ = cameraWorldPos_;
		cameraPrevViewProjectionInvMat_ = cameraViewProjectionInvMat_;
		cameraPrevViewInvMat_ = cameraViewInvMat_;
	}

	bool colorReprojection = viewMode == VIEW_MODE::COLOR_REPROJECTION || taa;

	// Save prev matricies
	prev.cameraProjUnjitteredMat_ = cameraProjUnjitteredMat_;
	prev.cameraProjMat_ = cameraProjMat_;
	prev.cameraViewProjMat_ = cameraViewProjMat_;
	prev.cameraViewMat_ = cameraViewMat_;
	prev.cameraWorldPos_ = cameraWorldPos_;
	prev.cameraViewProjectionInvMat_ = cameraViewProjectionInvMat_;
	prev.cameraViewInvMat_ = cameraViewInvMat_;

	RenderBuffers buffers;
	buffers.color =				GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);
	buffers.velocity =			GetRenderTexture(w, h, TEXTURE_FORMAT::RG16F);
	buffers.depth =				CORE_RENDER->GetSurfaceDepthTexture();
	buffers.albedo =			GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);
	buffers.diffuseLight =		GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA16F);
	buffers.specularLight =		GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA16F);
	buffers.normal =			GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA32F);
	buffers.shading =			GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);
	if (colorReprojection)
		buffers.colorReprojected = GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);

	Texture* colorPrev = GetPrevRenderTexture(PREV_TEXTURES::COLOR, w, h, TEXTURE_FORMAT::RGBA8);

	RenderScene scene = getRenderScene();

	// G-buffer
	{
		Texture *texs[4] = { buffers.albedo, buffers.shading, buffers.velocity, buffers.normal};
		CORE_RENDER->SetRenderTextures(4, texs, buffers.depth);
		CORE_RENDER->SetDepthTest(1);
		CORE_RENDER->Clear();
		{
			drawMeshes(PASS::DEFERRED, scene.meshes);
		}
		CORE_RENDER->SetRenderTextures(4, nullptr, nullptr);

		CORE_RENDER->SetDepthTest(0);
	}

	// Color reprojection
	if (colorReprojection)
	{
		Shader* shader = GetShader("reprojection.hlsl", planeMesh.get());
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
	
			CORE_RENDER->Draw(planeMesh.get(), 1);
			CORE_RENDER->BindTextures(2, nullptr);
	
			CORE_RENDER->SetRenderTextures(1, nullptr, nullptr);
		}
	}

	// Lights
	{
		CORE_RENDER->SetDepthTest(0);
		Texture *rts[2] = {buffers.diffuseLight, buffers.specularLight};
		CORE_RENDER->SetRenderTextures(2, rts, nullptr);
		CORE_RENDER->Clear();

		Shader *shader = GetShader("deferred_light.hlsl", planeMesh.get());
		if (shader && scene.lights.size())
		{
			CORE_RENDER->SetShader(shader);

			shader->SetVec4Parameter("camera_position", &cameraWorldPos_);
			shader->SetMat4Parameter("camera_view_projection_inv", &cameraViewProjectionInvMat_);

			CORE_RENDER->SetBlendState(BLEND_FACTOR::ONE, BLEND_FACTOR::ONE_MINUS_SRC_ALPHA);
			Texture *texs[4] = {buffers.normal, buffers.shading, buffers.albedo, buffers.depth};
			CORE_RENDER->BindTextures(4, texs);

			for (RenderLight &renderLight : scene.lights)
			{
				vec4 lightColor(renderLight.light->GetIntensity());
				vec4 dir = vec4(renderLight.worldDirection);

				shader->SetVec4Parameter("light_color", &lightColor);
				shader->SetVec4Parameter("light_direction", &dir);
				shader->FlushParameters();

				CORE_RENDER->Draw(planeMesh.get(), 1);
			}
			CORE_RENDER->BindTextures(4, nullptr);
			CORE_RENDER->SetBlendState(BLEND_FACTOR::NONE, BLEND_FACTOR::NONE);
		}

		CORE_RENDER->SetRenderTextures(2, nullptr, nullptr);
	}

	// Composite
	{
		compositeMaterial->SetDef("specular_quality", specualrQuality);
		
		if (auto shader = compositeMaterial->GetShader(planeMesh.get()))
		{
			Texture *rts[1] = { buffers.color };
			CORE_RENDER->SetRenderTextures(1, rts, nullptr);
			CORE_RENDER->SetShader(shader);

			vec4 environment_resolution;
			environment_resolution.x = float(environmentTexture.get()->GetWidth());
			environment_resolution.y = float(environmentTexture.get()->GetHeight());
			environment_resolution.z = float(environmentTexture.get()->GetMipmaps());
			shader->SetVec4Parameter("environment_resolution", &environment_resolution);

			vec4 environment_intensity;
			environment_intensity.x = diffuseEnvironemnt;
			environment_intensity.y = specularEnvironemnt;
			shader->SetVec4Parameter("environment_intensity", &environment_intensity);

			shader->SetVec4Parameter("camera_position", &cameraWorldPos_);

			shader->SetMat4Parameter("camera_view_projection_inv", &cameraViewProjectionInvMat_);

			shader->FlushParameters();

			CORE_RENDER->SetDepthTest(0);

			constexpr int tex_count = 7;
			Texture *texs[tex_count] = {
				buffers.albedo,
				buffers.normal,
				buffers.shading,
				buffers.diffuseLight,
				buffers.specularLight,
				buffers.depth,
				environmentTexture.get()
			};

			CORE_RENDER->BindTextures(tex_count, texs);
			{
				CORE_RENDER->Draw(planeMesh.get(), 1);
			}
			CORE_RENDER->BindTextures(tex_count, nullptr);
		}
	}

	// TAA
	if (taa)
		if (Shader *shader = GetShader("taa.hlsl", planeMesh.get()))
		{
			Texture* taaOut = GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8);

			Texture* texs_rt[1] = { taaOut };
			CORE_RENDER->SetRenderTextures(1, texs_rt, nullptr);
	
			Texture* texs[2] = { buffers.color, buffers.colorReprojected};
			CORE_RENDER->BindTextures(2, texs);
			CORE_RENDER->SetShader(shader);
	
			CORE_RENDER->Draw(planeMesh.get(), 1);
			CORE_RENDER->BindTextures(2, nullptr);
	
			CORE_RENDER->SetRenderTextures(1, nullptr, nullptr);

			std::swap(taaOut, buffers.color);
			ReleaseRenderTexture(taaOut);
		}

	// Restore default render target
	Texture *rts[1] = {CORE_RENDER->GetSurfaceColorTexture()};
	CORE_RENDER->SetRenderTextures(1, rts, CORE_RENDER->GetSurfaceDepthTexture());

	// Final copy
	{
		finalPostMaterial->SetDef("view_mode", (int)viewMode);

		if (auto shader = finalPostMaterial->GetShader(planeMesh.get()))
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
			CORE_RENDER->Draw(planeMesh.get(), 1);
			CORE_RENDER->BindTextures(tex_bind, nullptr);
		}
	}

	//renderGrid();

	// Wireframe
	if (wireframeModels && IsWireframe())
	{
		Texture* wireframe_depth = CORE_RENDER->GetSurfaceDepthTexture();

		const bool DepthTest = true;
		CORE_RENDER->SetDepthTest(DepthTest);

		// WTF? Why original depth buffer don't without TAA
		if (/*taa &&*/ DepthTest)
		{
			if (Shader *shader = GetShader("depth_indentation.hlsl", planeMesh.get()))
			{
				CORE_RENDER->SetDepthFunc(DEPTH_FUNC::ALWAYS);
				CORE_RENDER->SetBlendState(BLEND_FACTOR::NONE, BLEND_FACTOR::NONE);

				wireframe_depth = GetRenderTexture(w, h, TEXTURE_FORMAT::D24S8);
				CORE_RENDER->SetRenderTextures(1, nullptr, wireframe_depth);

				Texture* texs[1] = { CORE_RENDER->GetSurfaceDepthTexture() };
				CORE_RENDER->BindTextures(1, texs);
			
				CORE_RENDER->SetShader(shader);
				CORE_RENDER->Draw(planeMesh.get(), 1);
			
				CORE_RENDER->BindTextures(1, nullptr);

				CORE_RENDER->SetDepthFunc(DEPTH_FUNC::LESS_EQUAL);
			}

			// remove jitter form proj matrix
			cameraViewProjMat_ = ProjMat * ViewMat;
			cameraViewProjectionInvMat_ = cameraViewProjMat_.Inverse();
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

			Shader* shader = GetShader("msaa_resolve.hlsl", planeMesh.get());
			CORE_RENDER->SetShader(shader);

			CORE_RENDER->BindTextures(1, &msaaTex, BIND_TETURE_FLAGS::PIXEL);
			CORE_RENDER->Draw(planeMesh.get(), 1);
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

	// Vectors
	if (renderVectors.size())
	{
		CORE_RENDER->SetDepthTest(1);

		if (auto shader = GetShader("primitive.hlsl", lineMesh.get()))
		{
			CORE_RENDER->SetShader(shader);

			for (auto &v : renderVectors)
			{
				shader->SetVec4Parameter("main_color", &v.color);
				mat4 transform(0.0f);
				transform.el_2D[0][0] = v.v.x;
				transform.el_2D[1][0] = v.v.y;
				transform.el_2D[2][0] = v.v.z;
				mat4 MVP = prev.cameraViewProjMat_ * transform;
				shader->SetMat4Parameter("MVP", &MVP);
				shader->FlushParameters();

				CORE_RENDER->Draw(lineMesh.get(), 1);
			}
		}
	}

	RenderGUI();

	buffers.depth = nullptr;
	ReleaseRenderTexture(buffers.color);
	ReleaseRenderTexture(buffers.albedo);
	ReleaseRenderTexture(buffers.diffuseLight);
	ReleaseRenderTexture(buffers.specularLight);
	ReleaseRenderTexture(buffers.normal);
	ReleaseRenderTexture(buffers.shading);
	ReleaseRenderTexture(buffers.velocity);
	if (colorReprojection)
		ReleaseRenderTexture(buffers.colorReprojected);


	ExchangePrevRenderTexture(colorPrev, buffers.color);
}

Texture* Render::GetPrevRenderTexture(PREV_TEXTURES id, uint width, uint height, TEXTURE_FORMAT format)
{
	// TODO: do prev texture per window

	assert(id != PREV_TEXTURES::UNKNOWN);

	auto it = std::find_if(prevRenderTextures.begin(), prevRenderTextures.end(), [id, width, height, format](const RenderTexture& tex)
		{
			return tex.id == id && width == tex.width && height == tex.height && format == tex.format;
		});

	if (it != prevRenderTextures.end())
	{
		it->frame = _core->frame();
		return it->pointer.get();
	}

	TEXTURE_CREATE_FLAGS flags = TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET | TEXTURE_CREATE_FLAGS::COORDS_WRAP | TEXTURE_CREATE_FLAGS::FILTER_POINT;
	SharedPtr<Texture> tex = RES_MAN->CreateTexture(width, height, TEXTURE_TYPE::TYPE_2D, format, flags);

	prevRenderTextures.push_back({ _core->frame(), 0, width, height, format, 0, tex, id });

	return tex.get();
}

void Render::ExchangePrevRenderTexture(Texture* prev, Texture* some)
{
	int renderTex = -1;
	int prevTex = -1;

	for (auto i = 0; i < renderTextures.size(); i++)
		if (renderTextures[i].pointer.get() == some)
			renderTex = i;

	for (auto i = 0; i < prevRenderTextures.size(); i++)
		if (prevRenderTextures[i].pointer.get() == prev)
			prevTex = i;

	if (renderTex != -1 && prevTex != -1)
	{
		SharedPtr<Texture> ptr = renderTextures[renderTex].pointer;
		renderTextures[renderTex].pointer = prevRenderTextures[prevTex].pointer;
		prevRenderTextures[prevTex].pointer = ptr;
	}
	else
		LogWarning("ExchangePrevRenderTexture(): unable find textures");
}

void Render::renderGrid()
{
	CORE_RENDER->SetDepthTest(1);

	if (auto shader = GetShader("primitive.hlsl", gridMesh.get()))
	{
		CORE_RENDER->SetShader(shader);

		shader->SetMat4Parameter("MVP", &cameraViewProjMat_);
		shader->SetVec4Parameter("main_color", &vec4(0.3f, 0.3f, 0.3f, 1.0f));
		shader->FlushParameters();

		CORE_RENDER->Draw(gridMesh.get(), 1);
	}
}

void Render::drawMeshes(PASS pass, std::vector<RenderMesh>& meshes)
{
	for(RenderMesh &renderMesh : meshes)
	{
		Material *mat = renderMesh.mat;
		if (!mat)
			continue;

		Shader *shader = nullptr;

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

		mat4 MVP = cameraViewProjMat_ * renderMesh.worldTransformMat;
		mat4 MVP_prev = cameraPrevViewProjMatRejittered_ * renderMesh.worldTransformMatPrev;
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

void Render::Init()
{
	environmentTexture = RES_MAN->CreateStreamTexture(TEXTURES_DIR"output_skybox.dds", TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS);
	fontTexture = RES_MAN->CreateStreamTexture(TEXTURES_DIR"font.dds", TEXTURE_CREATE_FLAGS::NONE);
	planeMesh = RES_MAN->CreateStreamMesh("std#plane");
	gridMesh = RES_MAN->CreateStreamMesh("std#grid");
	lineMesh = RES_MAN->CreateStreamMesh("std#line");

	uint8 data[4] = {255u, 255u, 255u, 255u};
	whiteTexture = new Texture(unique_ptr<ICoreTexture>(CORE_RENDER->CreateTexture(&data[0], 1, 1, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::RGBA8, TEXTURE_CREATE_FLAGS::NONE, false)));
	assert(whiteTexture);

	MaterialManager* mm = _core->GetMaterialManager();

	compositeMaterial = mm->CreateInternalMaterial("composite");
	assert(compositeMaterial);

	finalPostMaterial = mm->CreateInternalMaterial("final_post");
	assert(finalPostMaterial);

	Log("Render initialized");
}

void Render::Update()
{
	renderTextures.erase(std::remove_if(renderTextures.begin(), renderTextures.end(),
	[&](const RenderTexture& r) -> bool
	{
		return r.free == 1 && (_core->frame() - r.frame) > 3;
	}),
	renderTextures.end());

	prevRenderTextures.erase(std::remove_if(prevRenderTextures.begin(), prevRenderTextures.end(),
	[&](const RenderTexture& r) -> bool
	{
		return (_core->frame() - r.frame) > 3;
	}),
	prevRenderTextures.end());

	renderVectors.clear();
}

void Render::Free()
{
	delete whiteTexture;
	environmentTexture.release();
	records.clear();
	lineMesh.release();
	fontTexture.release();
	planeMesh.release();
	gridMesh.release();
	runtimeShaders.clear();
	renderTextures.clear();
	prevRenderTextures.clear();
}


