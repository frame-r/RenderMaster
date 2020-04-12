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
#include "render_paths/render_path_realtime.h"
#include "render_paths/render_path_pathtracing.h"
#include "thirdparty/simplecpp/SimpleCpp.h"
#include "crc.h"
#include <memory>
#include <sstream>

namespace {
	crc32 crc;
}


struct ShaderInstance
{
	SharedPtr<Shader> shader;
	std::string path;
	int64_t time;
	std::set<std::string> includes;
};

static std::unordered_map<string, ShaderInstance> runtimeShaders; // defines -> Shader


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

static void process_shader(INPUT_ATTRUBUTE attrib, const vector<string>* defines, const char*& ppTextOut, const char* ppTextIn,
					const string& fullPath, const string&& fileNameOut, int type, set<string>& includes)
{
	simplecpp::DUI dui;

	if (defines)
		copy(begin(*defines), end(*defines), back_inserter(dui.defines));

	if ((int)(attrib & INPUT_ATTRUBUTE::NORMAL)) dui.defines.push_back("ENG_INPUT_NORMAL");
	if ((int)(attrib & INPUT_ATTRUBUTE::TEX_COORD)) dui.defines.push_back("ENG_INPUT_TEXCOORD");
	if ((int)(attrib & INPUT_ATTRUBUTE::COLOR)) dui.defines.push_back("ENG_INPUT_COLOR");

	if (type == 0)
		dui.defines.push_back("ENG_SHADER_VERTEX");
	else if (type == 1)
		dui.defines.push_back("ENG_SHADER_PIXEL");
	else if (type == 2)
		dui.defines.push_back("ENG_SHADER_GEOMETRY");
	else if (type == 3)
		dui.defines.push_back("ENG_SHADER_COMPUTE");

	simplecpp::OutputList outputList;
	std::vector<std::string> files;
	string textIn = ppTextIn;
	std::stringstream f(textIn);
	std::map<std::string, simplecpp::TokenList*> included;
	simplecpp::TokenList rawtokens(f, files, fullPath, &outputList);
	simplecpp::TokenList outputTokens(files);

	simplecpp::preprocess(outputTokens, rawtokens, files, included, dui, &outputList);

	for (auto& i : included)
		includes.insert(i.first);

	const string out = outputTokens.stringify();
	auto size = out.size();
	char* tmp = new char[size + 1];
	strncpy(tmp, out.c_str(), size);
	tmp[size] = '\0';
	ppTextOut = tmp;
};

auto Render::GetShader(const char *name, Mesh *mesh, const vector<string>* defines, LOAD_SHADER_FLAGS flags) ->Shader*
{
	SharedPtr<Shader> shader;

	INPUT_ATTRUBUTE attrib = INPUT_ATTRUBUTE::UNKNOWN;

	if (mesh)
		attrib = mesh->GetAttributes();

	string shaderKey = string(name) + '-';

	if (defines)
		for(const string& def : *defines)
			shaderKey += def;

	if ((int)(attrib & INPUT_ATTRUBUTE::NORMAL)) shaderKey += "ENG_INPUT_NORMAL";
	if ((int)(attrib & INPUT_ATTRUBUTE::TEX_COORD)) shaderKey += "ENG_INPUT_TEXCOORD";
	if ((int)(attrib & INPUT_ATTRUBUTE::COLOR)) shaderKey += "ENG_INPUT_COLOR";

	auto it = runtimeShaders.find(shaderKey);

	if (it != runtimeShaders.end())
	{
		return it->second.shader.get();
	}
	else
	{
		string fullPath = _core->GetDataPath() + '\\' + string(SHADER_DIR) + name;
		File f = FS->OpenFile(fullPath.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
		size_t size = f.FileSize();

		char* textRaw = new char[size + 1];
		*(textRaw + size) = '\0';

		f.Read((uint8*)textRaw, size);

		set<string> includes;

		const char *textVertParced;
		process_shader(attrib, defines, textVertParced, textRaw, fullPath, "out_v.shader", 0, includes);

		const char *textFragParced;
		process_shader(attrib, defines, textFragParced, textRaw, fullPath, "out_f.shader", 1, includes);

		const char *textGeomParced = nullptr;
		if (bool(flags & LS_GEOMETRY))
			process_shader(attrib, defines, textGeomParced, textRaw, fullPath, "out_g.shader", 2, includes);

		ShaderInstance runtime;
		runtime.shader = RES_MAN->CreateShader(textVertParced, textGeomParced, textFragParced);
		runtime.time = FS->GetTime(fullPath);
		runtime.path = fullPath;
		runtime.includes = std::move(includes);

		shader = runtime.shader;

		if (!shader)
			LogCritical("Render::GetShader(): can't compile %s standard shader", name);

		runtimeShaders.emplace(shaderKey, runtime);
	}
	return shader.get();
}

auto DLLEXPORT Render::GetComputeShader(const char* name, const vector<string>* defines) -> Shader*
{
	SharedPtr<Shader> shader;

	string shaderKey = string(name);

	if (defines)
	for (const string& def : *defines)
		shaderKey += def;

	auto it = runtimeShaders.find(shaderKey);

	if (it != runtimeShaders.end())
	{
		return it->second.shader.get();
	}
	else
	{
		string fileIn = _core->GetDataPath() + '\\' + string(SHADER_DIR) + name;
		File f = FS->OpenFile(fileIn.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
		size_t size = f.FileSize();

		char* textRaw = new char[size + 1];
		*(textRaw + size) = '\0';

		f.Read((uint8*)textRaw, size);

		set<string> includes;

		const char* textParced;
		process_shader(INPUT_ATTRUBUTE::UNKNOWN, defines, textParced, textRaw, fileIn, "out_c.shader", 3, includes);

		ShaderInstance runtime;
		runtime.shader = RES_MAN->CreateComputeShader(textParced);
		runtime.time = FS->GetTime(fileIn);
		runtime.path = fileIn;
		runtime.includes = std::move(includes);

		shader = runtime.shader;

		if (!shader)
			LogCritical("Render::GetShader(): can't compile %s standard shader", name);

		runtimeShaders.emplace(shaderKey, runtime);
	}
	return shader.get();

}

auto DLLEXPORT Render::ReloadShaders() -> void
{
	Log("Reloading shaders...");

	for (auto iter = runtimeShaders.begin(); iter != runtimeShaders.end(); )
	{
		if (iter->second.time != FS->GetTime(iter->second.path)) {
			iter = runtimeShaders.erase(iter);
		}
		else {
			++iter;
		}
	}
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
					r.buffer = RES_MAN->CreateStructuredBuffer((uint)r.length * sizeof(charr), sizeof(charr), BUFFER_USAGE::CPU_WRITE);
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

		meshesVec.emplace_back(RenderMesh{model->GetId(), mesh, model->GetMaterial(), model, model->GetWorldTransform(), model->GetWorldTransformPrev()});
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

		RenderLight &rl = scene.lights.emplace_back();
		rl.light = l;
		rl.transform = l->GetWorldTransform();
		rl.type = l->GetLightType();
		rl.worldDirection = worldDir;
		rl.intensity = l->GetIntensity();

		if (rl.type == LIGHT_TYPE::AREA)
			scene.areaLights.push_back(rl);
	}
	scene.hasWorldLight = scene.lights.size() > 0;

	if (scene.hasWorldLight)
		scene.sun_direction = scene.lights[0].worldDirection;
	else
		scene.sun_direction = vec4(0, 0, 1, 0);

	return scene;
}

//auto DLLEXPORT Render::DrawMeshes(PASS pass) -> void
//{
//	vector<RenderMesh> meshes = getRenderMeshes();
//	drawMeshes(pass, meshes);
//}

auto DLLEXPORT Render::GetRenderTexture(uint width, uint height, TEXTURE_FORMAT format, int msaaSamples, TEXTURE_TYPE type, bool mips) -> Texture *
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

	TEXTURE_CREATE_FLAGS flags = TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET | TEXTURE_CREATE_FLAGS::COORDS_WRAP | TEXTURE_CREATE_FLAGS::FILTER_TRILINEAR;
	if (mips)
		flags = flags | TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS;

	switch (msaaSamples)
	{
		case 0:
		case 1: break;
		case 2: flags = flags | TEXTURE_CREATE_FLAGS::MSAA_2x; break;
		case 4: flags = flags | TEXTURE_CREATE_FLAGS::MSAA_4x; break;
		case 8: flags = flags | TEXTURE_CREATE_FLAGS::MSAA_8x; break;
		default: LogWarning("Render::GetRenderTexture(): Unknown number of MSAA samples"); break;
	}

	SharedPtr<Texture> tex = RES_MAN->CreateTexture(width, height, type, format, flags);

	renderTextures.push_back({_core->frame(), 0, width, height, format, msaaSamples, tex });

	return tex.get();
}

auto DLLEXPORT Render::ReleaseRenderTexture(Texture* tex) -> void
{
	if (!tex) return;
	std::for_each(renderTextures.begin(), renderTextures.end(), [tex](RenderTexture& t) { if (tex == t.pointer.get()) t.free = 1; });
}

auto DLLEXPORT Render::SetEnvironmentTexturePath(const char* path) -> void
{
	if (path == envirenmentHDRIPath)
		return;

	envirenmentHDRIPath = path;
	environmentHDRI = RES_MAN->CreateStreamTexture(path, TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS);
}

auto DLLEXPORT Render::GetEnvironmentTexturePath() -> const char*
{
	return envirenmentHDRIPath.c_str();
}

auto DLLEXPORT Render::SetEnvironmentType(ENVIRONMENT_TYPE type) -> void
{
	environmentType = static_cast<ENVIRONMENT_TYPE>(type);
}

auto DLLEXPORT Render::SetRenderPath(RENDER_PATH type) -> void
{
	if (type == RENDER_PATH::REALTIME)
		renderpath = realtimeObj;
	else if (type == RENDER_PATH::PATH_TRACING)
		renderpath = pathtracingObj;
	else
		throw std::exception();
	renderPathType = type;
}

void Render::RenderFrame(size_t viewID, const Engine::CameraData& camera, Model** wireframeModels, int modelsNum)
{
	renderpath->FrameBegin(viewID, camera, wireframeModels, modelsNum);
	renderpath->RenderFrame();
	renderpath->FrameEnd();
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

void Render::GetEnvironmentResolution(vec4& environment_resolution)
{
	environment_resolution.x = float(environment->GetWidth());
	environment_resolution.y = float(environment->GetHeight());
	environment_resolution.z = float(environment->GetMipmaps());
}

void Render::GetEnvironmentIntensity(vec4& environment_intensity)
{
	environment_intensity.x = diffuseEnvironemnt;
	environment_intensity.y = specularEnvironemnt;
}

void Render::renderGrid()
{
	//CORE_RENDER->SetDepthTest(1);

	//if (auto shader = GetShader("primitive.hlsl", gridMesh.get()))
	//{
	//	CORE_RENDER->SetShader(shader);

	//	shader->SetMat4Parameter("MVP", &mats.ViewProjMat_);
	//	shader->SetVec4Parameter("main_color", &vec4(0.3f, 0.3f, 0.3f, 1.0f));
	//	shader->FlushParameters();

	//	CORE_RENDER->Draw(gridMesh.get(), 1);
	//}
}

void Render::calculateAtmosphereHash(vec4 sun_direction, AtmosphereHash& hash)
{
	memset(&hash, 0, sizeof(AtmosphereHash));
	memcpy(&hash, &sun_direction, 4 * 3);
}

void Render::updateEnvirenment(RenderScene& scene)
{
	if (environmentType == ENVIRONMENT_TYPE::ATMOSPHERE)
	{
		if (Shader * shader = GetShader("environment_cubemap.hlsl", fullScreen(), nullptr, Render::LS_GEOMETRY))
		{
			environment = environmentAtmosphere;

			AtmosphereHash atmnnNextHash;
			calculateAtmosphereHash(scene.sun_direction, atmnnNextHash);

			if (!(atmnnNextHash == atmosphereHash))
			{
				atmosphereHash = atmnnNextHash;

				CORE_RENDER->SetRenderTextures(1, &environment, nullptr);

				uint w, h;
				CORE_RENDER->GetViewport(&w, &h);
				CORE_RENDER->SetViewport(environmentCubemapSize, environmentCubemapSize, 6);

				CORE_RENDER->SetShader(shader);
				CORE_RENDER->SetDepthTest(0);

				shader->SetVec4Parameter("sun_direction", &scene.sun_direction);
				shader->FlushParameters();

				CORE_RENDER->Draw(planeMesh.get(), 6);

				CORE_RENDER->SetRenderTextures(1, nullptr, nullptr);
				CORE_RENDER->SetViewport(w, h);

				environment->CreateMipmaps();
			}
		}
		else
			environment = environmentHDRI.get();
	}
	else if (environmentType == ENVIRONMENT_TYPE::CUBEMAP)
		environment = environmentHDRI.get();
	else
		environment = blackCubemapTexture;
}

uint32 Render::frameID()
{
	return uint32_t(_core->frame() % maxFrames);
}

uint32 Render::readbackFrameID()
{
	return uint32_t((_core->frame() > (int64_t)maxFrames ? _core->frame() - 3 : 0) % maxFrames);;
}

uint Render::getNumLines()
{
	return renderpath->getNumLines();
}

std::string Render::getString(uint i)
{
	return renderpath->getString(i);
}

void Render::Init()
{
	envirenmentHDRIPath = TEXTURES_DIR"cube_room.dds";
	environmentHDRI = RES_MAN->CreateStreamTexture(envirenmentHDRIPath.c_str(), TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS);

	fontTexture = RES_MAN->CreateStreamTexture(TEXTURES_DIR"font.dds", TEXTURE_CREATE_FLAGS::NONE);
	planeMesh = RES_MAN->CreateStreamMesh("std#plane");
	gridMesh = RES_MAN->CreateStreamMesh("std#grid");
	lineMesh = RES_MAN->CreateStreamMesh("std#line");

	uint8 data[4] = {255u, 255u, 255u, 255u};
	whiteTexture = new Texture(unique_ptr<ICoreTexture>(CORE_RENDER->CreateTexture(&data[0], 1, 1, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::RGBA8, TEXTURE_CREATE_FLAGS::NONE, false)));
	assert(whiteTexture);

	MaterialManager* mm = _core->GetMaterialManager();

	// GPU timers
	CORE_RENDER->CreateGPUTiming(maxFrames, T_TIMERS_NUM);

	_core->AddProfilerCallback(this);

	environmentAtmosphere = GetRenderTexture(environmentCubemapSize, environmentCubemapSize, TEXTURE_FORMAT::RGBA16F, 1, TEXTURE_TYPE::TYPE_CUBE, true);
	blackCubemapTexture = new Texture(unique_ptr<ICoreTexture>(CORE_RENDER->CreateTexture(nullptr, 1, 1, TEXTURE_TYPE::TYPE_CUBE, TEXTURE_FORMAT::RGBA8, TEXTURE_CREATE_FLAGS::NONE, false)));

	realtimeObj = new RenderPathRealtime;
	pathtracingObj = new RenderPathPathTracing;
	SetRenderPath(RENDER_PATH::PATH_TRACING);

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
	delete realtimeObj;
	realtimeObj = nullptr;

	delete pathtracingObj;
	pathtracingObj = nullptr;

	ReleaseRenderTexture(environmentAtmosphere);
	delete whiteTexture;
	delete blackCubemapTexture;
	environmentHDRI.release();
	records.clear();
	lineMesh.release();
	fontTexture.release();
	planeMesh.release();
	gridMesh.release();
	runtimeShaders.clear();
	renderTextures.clear();
	prevRenderTextures.clear();
}

uint32_t Render::RenderScene::getHash()
{
	constexpr size_t approxMeshSize = sizeof(mat4) + 30 /*path*/ + 4;
	constexpr size_t approxLightSize = sizeof(mat4);
	size_t approxSize = approxMeshSize * meshes.size() + approxLightSize * lights.size();
	
	vector<uint8_t> data(approxSize);
	size_t len = 1;

	auto addData = [&len, &data](const void* src, size_t srcLen)
	{
		len += srcLen;
		data.resize(len);
		memcpy(data.data() + len - srcLen, src, srcLen);
	};

	addData(&approxSize, sizeof(approxSize));

	for (size_t i = 0; i < meshes.size(); ++i)
	{
		RenderMesh& r = meshes[i];

		addData(r.mesh->GetPath(), strlen(r.mesh->GetPath()));
		addData(&r.worldTransformMat, sizeof(mat4));

		if (r.mat)
			addData(r.mat->GetId(), strlen(r.mat->GetId()));
	}

	for (size_t i = 0; i < lights.size(); ++i)
	{
		RenderLight& l = lights[i];
		addData(&l.transform, sizeof(mat4));
		addData(&l.intensity, sizeof(float));
	}

	return crc.update(data.data(), data.size());
}
