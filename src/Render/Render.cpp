#include "Pch.h"
#include "Render.h"
#include "Core.h"
#include "ConsoleWindow.h"
#include "SceneManager.h"
#include "simplecpp.h"
#include <memory>

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)
uint widths[256] = {
8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,0
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,8
,4
,4
,5
,8
,7
,11
,10
,3
,4
,4
,5
,9
,3
,5
,3
,5
,7
,7
,7
,7
,7
,7
,7
,7
,7
,7
,3
,3
,9
,8
,9
,6
,12
,9
,8
,8
,9
,8
,7
,9
,10
,3
,7
,8
,8
,12
,10
,10
,8
,10
,9
,9
,8
,9
,9
,13
,8
,9
,7
,4
,5
,4
,9
,5
,3
,7
,8
,6
,8
,7
,4
,8
,7
,3
,3
,6
,3
,11
,7
,8
,8
,8
,4
,6
,4
,7
,6
,9
,6
,6
,6
,4
,3
,4
,9
,8
,7
,8
,3
,7
,5
,9
,5
,5
,5
,16
,7
,4
,12
,8
,7
,8
,8
,3
,3
,5
,5
,5
,7
,13
,4
,10
,6
,4
,12
,8
,6
,7
,4
,4
,7
,7
,7
,7
,3
,6
,5
,12
,5
,7
,9
,5
,12
,5
,5
,9
,5
,5
,4
,8
,6
,3
,3
,5
,6
,7
,12
,12
,12
,6
,8
,8
,8
,8
,8
,8
,11
,8
,7
,7
,7
,7
,3
,3
,3
,3
,9
,10
,10
,10
,10
,10
,10
,9
,10
,9
,9
,9
,9
,7
,7
,7
,7
,7
,7
,7
,7
,7
,11
,6
,7
,7
,7
,7
,3
,3
,3
,3
,7
,7
,8
,8
,8
,8
,8
,9
,8
,7
,7
,7
,7
,6
,8
,6
};

Render::Render(ICoreRender *pCoreRender) : _pCoreRender(pCoreRender)
{
	_pCore->GetSubSystem((ISubSystem**)&_pSceneMan, SUBSYSTEM_TYPE::SCENE_MANAGER);
	_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
	_pCore->GetSubSystem((ISubSystem**)&_fsystem, SUBSYSTEM_TYPE::FILESYSTEM);

	_pCore->consoleWindow()->addCommand("shaders_reload", std::bind(&Render::shaders_reload, this, std::placeholders::_1, std::placeholders::_2));
	
	_pCore->AddProfilerCallback(this);
}

Render::~Render()
{
}

API_RESULT Render::shaders_reload(const char ** args, uint argsNumber)
{
	ShadersReload();
	return S_OK;
}

uint Render::getNumLines()
{
	return 5;
}

string Render::getString(uint i)
{
	switch (i)
	{
		case 0: return "==== Render ====";
		case 1: return "FPS: " + std::to_string(_pCore->FPSlazy());
		case 2: return "Runtime Shaders: " + std::to_string(_runtime_shaders.size());
		case 3: return "Texture pool: " + std::to_string(_render_textures.size());
		case 4: return "";
	}
	assert(false);
	return "";
}

void Render::renderForward(RenderBuffers& buffers, vector<RenderMesh>& meshes)
{
	renderTarget->SetColorTexture(0, buffers.colorHDR.Get());
	renderTarget->SetDepthTexture(buffers.depth.Get());
	_pCoreRender->SetCurrentRenderTarget(renderTarget.Get());
	{
		_pCoreRender->Clear();

		drawMeshes(meshes, "mesh.shader", RENDER_PASS::FORWARD);
	}
	_pCoreRender->RestoreDefaultRenderTarget();
}

void Render::renderEnginePost(RenderBuffers& buffers)
{
	//_pCoreRender->PushStates();

	INPUT_ATTRUBUTE attribs;
	_quad->GetAttributes(&attribs);

	IShader *shader = getShader({"engine_post.shader", attribs});
	_pCoreRender->SetShader(shader);

	_pCoreRender->BindTexture(0, buffers.colorHDR.Get());

	_pCoreRender->SetDepthTest(0);

	renderTarget->SetColorTexture(0, buffers.color.Get());
	_pCoreRender->SetCurrentRenderTarget(renderTarget.Get());_pCoreRender->SetCurrentRenderTarget(renderTarget.Get());
	{
		_pCoreRender->Draw(_quad.Get());
	}
	_pCoreRender->RestoreDefaultRenderTarget();

	_pCoreRender->UnbindAllTextures();
	_pCoreRender->SetDepthTest(1);

	//_pCoreRender->PopStates();
}

void Render::RenderFrame(const ICamera *pCamera)
{
	uint w, h;
	_pCoreRender->GetViewport(&w, &h);
	float aspect = (float)w / h;

	const_cast<ICamera*>(pCamera)->GetViewProjectionMatrix(&ViewProjMat, aspect);
	const_cast<ICamera*>(pCamera)->GetViewMatrix(&ViewMat);

	mat4 camModel;
	const_cast<ICamera*>(pCamera)->GetModelMatrix(&camModel);
	cameraWorldPos = camModel.Column3(3);

	vector<RenderMesh> meshes = getRenderMeshes();

	RenderBuffers buffers = initBuffers(w, h);

	renderForward(buffers, meshes);

	renderEnginePost(buffers);

#if 0
	///////////////////////////////
	// ID pass
	///////////////////////////////
	//
	// Render all models ID (only for debug picking)
	//
	{
		renderTarget->SetColorTexture(0, buffers.id.Get());
		renderTarget->SetDepthTexture(buffers.depth.Get());

		_pCoreRender->SetCurrentRenderTarget(renderTarget.Get());
		{
			_pCoreRender->Clear();

			_draw_meshes(ViewMat, ViewProjMat, meshes, RENDER_PASS::ID);
		}

		#if 0
			IInput *i;
			_pCore->GetSubSystem((ISubSystem**)&i, SUBSYSTEM_TYPE::INPUT);
			int pr;
			i->IsMoisePressed(&pr, MOUSE_BUTTON::LEFT);
			if (pr)
			{
				uint curX, curY;
				i->GetMousePos(&curX, &curY);

				ICoreTexture *coreTex;
				tex->GetCoreTexture(&coreTex);
				uint data;
				uint read;
				_pCoreRender->ReadPixel2D(coreTex, &data, &read, curX, curY);
				if (read > 0)
				{
					LOG_FORMATTED("Id = %i", data);
				}
			}
		#endif

		_pCoreRender->RestoreDefaultRenderTarget();
		renderTarget->UnbindAll();
	}
#endif

	_pCoreRender->BlitRenderTargetToDefault(renderTarget.Get());
	renderTarget->UnbindAll();

	releaseBuffers(buffers);
}

void Render::RenderPassIDPass(const ICamera *pCamera, ITexture *tex, ITexture *depthTex)
{
	uint w, h;
	tex->GetWidth(&w);
	tex->GetHeight(&h);
	float aspect = (float)w / h;

	mat4 ViewProjMat;
	const_cast<ICamera*>(pCamera)->GetViewProjectionMatrix(&ViewProjMat, aspect);

	mat4 ViewMat;
	const_cast<ICamera*>(pCamera)->GetViewMatrix(&ViewMat);
	
	vector<RenderMesh> meshes = getRenderMeshes();

	renderTarget->SetColorTexture(0, tex);
	renderTarget->SetDepthTexture(depthTex);

	_pCoreRender->SetCurrentRenderTarget(renderTarget.Get());
	{
		_pCoreRender->Clear();

		drawMeshes(meshes, "id.shader", RENDER_PASS::ID);
	}
	_pCoreRender->RestoreDefaultRenderTarget();

	renderTarget->UnbindAll();
}

void Render::RenderPassGUI()
{
	INPUT_ATTRUBUTE attribs;
	_quad->GetAttributes(&attribs);

	IShader *shader = getShader({"font.shader", attribs});
	if (!shader)
		return;

	_pCoreRender->SetShader(shader);

	uint w, h;
	_pCoreRender->GetViewport(&w, &h);

	shader->SetFloatParameter("invHeight2", 2.0f / h);
	shader->SetFloatParameter("invWidth2", 2.0f / w);
	shader->FlushParameters();

	_pCoreRender->BindTexture(0, fontTexture.Get());

	_pCoreRender->SetBlendState(BLEND_FACTOR::ONE, BLEND_FACTOR::ONE_MINUS_SRC_ALPHA);
	_pCoreRender->SetDepthTest(0);

	float offsetVert = 0.0f;
	for (int i = 0; i < _pCore->ProfilerRecords(); i++)
	{
		if (i >= _records.size())
			_records.push_back(RenderProfileRecord());

		RenderProfileRecord &r = _records[i];

		string fps = _pCore->GetProfilerRecord(i);

		if (fps.size() > 0)
		{
			if (!r.buffer || r.length < fps.size())
			{
				r.length = fps.size();
				r.buffer = _pResMan->CreateStructuredBuffer((uint)r.length * sizeof(charr), sizeof(charr));
				r.bufferData = unique_ptr<charr[]>(new charr[r.length]);
			}

			float offset = 0.0f;
			for (size_t i = 0u; i < fps.size(); i++)
			{
				float w = static_cast<float>(widths[fps[i]]);
				r.bufferData[i].data[0] = w;
				r.bufferData[i].data[1] = offset;
				r.bufferData[i].data[2] = offsetVert;
				r.bufferData[i].id = static_cast<uint>(fps[i]);
				offset += w;
			}

			std::hash<string> hashFn;
			size_t newHash = hashFn(fps);
			if (newHash != r.txtHash)
			{
				r.txtHash = newHash;
				r.buffer->SetData(reinterpret_cast<uint8*>(&r.bufferData[0].data[0]), fps.size() * sizeof(charr));
			}

			_pCoreRender->SetStructuredBufer(1, r.buffer.Get());
			_pCoreRender->Draw(_quad.Get(), (uint)fps.size());
		}

		offsetVert -= 17.0f;
	}
	_pCoreRender->UnbindAllTextures();
	_pCoreRender->SetStructuredBufer(1, nullptr);

	_pCoreRender->SetDepthTest(1);
	_pCoreRender->SetBlendState(BLEND_FACTOR::NONE, BLEND_FACTOR::NONE);
}

void Render::_update()
{
	_render_textures.erase(std::remove_if(_render_textures.begin(), _render_textures.end(),
		[&](const RenderTexture& r) -> bool
		{
			return r.free == 1 && (_pCore->frame() - r.frame) > 3;
		}),
	_render_textures.end());
}

IShader* Render::getShader(const ShaderRequirement &req)
{
	IShader *pShader = nullptr;

	auto it = _runtime_shaders.find(req);

	if (it != _runtime_shaders.end())
	{
		ShaderPtr& shaderPtr = it->second;
		return shaderPtr.Get();
	}
	else
	{
		const auto process_shader = [&](const char *&ppTextOut, const char *ppTextIn, const string& fileNameIn, const string&& fileNameOut, int type) -> void
		{
			simplecpp::DUI dui;

			const char *gapi;
			_pCoreRender->GetName(&gapi);
			bool is_opengl = (strcmp("GLCoreRender", gapi) == 0);

			if (is_opengl)
				dui.defines.push_back("ENG_OPENGL");
			else
				dui.defines.push_back("ENG_DIRECTX11");

			if (type == 0)
				dui.defines.push_back("ENG_SHADER_VERTEX");
			else if (type == 1)
				dui.defines.push_back("ENG_SHADER_PIXEL");
			else if (type == 2)
				dui.defines.push_back("ENG_SHADER_GEOMETRY");

			if ((int)(req.attributes & INPUT_ATTRUBUTE::NORMAL)) dui.defines.push_back("ENG_INPUT_NORMAL");
			if ((int)(req.attributes & INPUT_ATTRUBUTE::TEX_COORD)) dui.defines.push_back("ENG_INPUT_TEXCOORD");
			if ((int)(req.attributes & INPUT_ATTRUBUTE::COLOR)) dui.defines.push_back("ENG_INPUT_COLOR");

			// TODO: move to Common.h or filesystem
			const char *pString;
			_pCore->GetInstalledDir(&pString);
			string installedDir = string(pString);
			string fullPath = installedDir + '\\' + SHADER_DIR + '\\' + fileNameIn;

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

			// Workaround for opengl because C preprocessor eats up unknown derictive "#version 420"
			if (is_opengl)
				size += 13;

			char *tmp = new char[size + 1];
			if (is_opengl)
			{
				strncpy(tmp + 0, "#version 420\n", 13);
				strncpy(tmp + 13, out.c_str(), size - 13);
			} else
				strncpy(tmp, out.c_str(), size);

			tmp[size] = '\0';

			ppTextOut = tmp;
		};

		const char *text;

		intrusive_ptr<ITextFile> targetShader = _pResMan->LoadTextFile(req.path);

		targetShader->GetText(&text);

		const char *pFileIn;
		targetShader->GetFile(&pFileIn);
		string fileIn = pFileIn;

		const char *textVertOut;
		const char *textFragOut;

		process_shader(textVertOut, text, fileIn, "out_v.shader", 0);
		process_shader(textFragOut, text, fileIn, "out_f.shader", 1);

		bool compiled = SUCCEEDED(_pResMan->_CreateShader(&pShader, textVertOut, nullptr, textFragOut)) && pShader != nullptr;

		if (!compiled)
		{
			LOG_FATAL("Render::_get_shader(): can't compile standard shader\n");
			_runtime_shaders.emplace(req, ShaderPtr(nullptr));
		}
		else
			_runtime_shaders.emplace(req, ShaderPtr(pShader));
	}
	return pShader;
}

vector<Render::RenderMesh> Render::getRenderMeshes()
{
	vector<Render::RenderMesh> meshesVec;
	SceneManager *scene = (SceneManager*)_pSceneMan;

	for (tree<IGameObject*>::iterator it = scene->_gameobjects.begin(); it != scene->_gameobjects.end(); ++it)
	{
		IModel *model = dynamic_cast<IModel*>(*it);
		if (model)
		{
			uint id;
			model->GetID(&id);

			IMesh *mesh;
			model->GetMesh(&mesh);

			mat4 M;
			model->GetModelMatrix(&M);

			IMaterial *mat;
			model->GetMaterial(&mat);

			vec4 baseColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			vec4 shading;

			if (mat)
			{
				mat->GetBaseColor(&baseColor);
				mat->GetMetallic(&shading.x);
				mat->GetRoughness(&shading.y);
			}

			meshesVec.emplace_back(RenderMesh{id, mesh, mat, baseColor, shading, M});
		}
	}
	return meshesVec;
}

void Render::setShaderMeshParameters(RENDER_PASS pass, RenderMesh *mesh, IShader *shader)
{
	if (mesh)
	{
		mat4 MVP = ViewProjMat * mesh->modelMat;
		shader->SetMat4Parameter("MVP", &MVP);

		mat4 M = mesh->modelMat;
		shader->SetMat4Parameter("M", &M);

		mat4 NM = M.Inverse().Transpose();
		shader->SetMat4Parameter("NM", &NM);
	}

	if (pass == RENDER_PASS::ID)
	{
		shader->SetUintParameter("model_id", mesh->modelId);
	}
	else if (pass == RENDER_PASS::FORWARD)
	{
		const vec4 lightPosition = vec4(1.0f, -2.0f, 3.0f, 0.0f);

		shader->SetVec4Parameter("main_color", &mesh->baseColor);
		shader->SetVec4Parameter("shading", &mesh->shading);
		shader->SetVec4Parameter("nL_world", &(lightPosition.Normalized()));
		shader->SetVec4Parameter("camera_position", &cameraWorldPos);
		shader->SetVec4Parameter("light_position", &lightPosition);
	}

	shader->FlushParameters();
}

void Render::drawMeshes(vector<RenderMesh>& meshes, const char *s, RENDER_PASS pass)
{
	for(RenderMesh &renderMesh : meshes)
	{
		INPUT_ATTRUBUTE attribs;
		renderMesh.mesh->GetAttributes(&attribs);

		IShader *shader = getShader({s, attribs});
		if (!shader)
			continue;
		
		_pCoreRender->SetShader(shader);
		setShaderMeshParameters(pass, &renderMesh, shader);

		if (bool(attribs & INPUT_ATTRUBUTE::TEX_COORD))
			_pCoreRender->BindTexture(0, whiteTexture.Get());

		_pCoreRender->Draw(renderMesh.mesh);

		if (bool(attribs & INPUT_ATTRUBUTE::TEX_COORD))
			_pCoreRender->BindTexture(0, nullptr);
	}
}

ITexture* Render::getRenderTargetTexture2d(uint width, uint height, TEXTURE_FORMAT format)
{
	for (RenderTexture &tex : _render_textures)
	{
		if (tex.free)
		{
			if (width == tex.width && height == tex.height && format == tex.format)
			{
				tex.free = 0;
				tex.frame = _pCore->frame();
				return tex.tex.Get();
			}
		}
	}

	TEXTURE_CREATE_FLAGS flags = TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET | TEXTURE_CREATE_FLAGS::COORDS_WRAP | TEXTURE_CREATE_FLAGS::FILTER_POINT;
	TexturePtr tex = _pResMan->CreateTexture(width, height, TEXTURE_TYPE::TYPE_2D, format, flags);

	_render_textures.push_back({_pCore->frame(), 0, width, height, format, tex});

	return tex.Get();
}
void Render::releaseTexture2d(ITexture *tex)
{
	for (RenderTexture &tp : _render_textures)
	{
		if (tp.tex.Get() == tex)
			tp.free = 1;
	}
}

RenderBuffers Render::initBuffers(uint w, uint h)
{
	RenderBuffers ret;

	ret.width = w;
	ret.height = h;
	ret.color = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::RGBA8);
	ret.colorHDR = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::RGBA16F);
	ret.depth = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::D24S8);
	ret.directLight = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::RGBA16F);
	ret.normal = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::RGBA8);
	ret.shading = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::RGBA8);
	ret.id = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::R32UI);

	return ret;
}

void Render::releaseBuffers(RenderBuffers& buffers)
{
	releaseTexture2d(buffers.color.Get());		buffers.color = nullptr;
	releaseTexture2d(buffers.colorHDR.Get());	buffers.colorHDR = nullptr;
	releaseTexture2d(buffers.depth.Get());		buffers.depth = nullptr;
	releaseTexture2d(buffers.directLight.Get());buffers.directLight = nullptr;
	releaseTexture2d(buffers.normal.Get());		buffers.normal = nullptr;
	releaseTexture2d(buffers.shading.Get());	buffers.shading = nullptr;
	releaseTexture2d(buffers.id.Get());			buffers.id = nullptr;
}

void Render::Init()
{
	_pCoreRender->SetDepthTest(1);

	_pCore->AddUpdateCallback(std::bind(&Render::_update, this));

	// Render Targets
	renderTarget = _pResMan->CreateRenderTarget();

	{
		// Meshes
		// get all default meshes and release only for test

		MeshPtr axesMesh;
		MeshPtr axesArrowMesh;
		MeshPtr gridMesh;

		axesMesh = _pResMan->LoadMesh("std#axes");
		axesArrowMesh = _pResMan->LoadMesh( "std#axes_arrows");
		gridMesh = _pResMan->LoadMesh("std#grid");

		// Create texture for test
		auto tex = _pResMan->CreateTexture(300, 300, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::R32UI, TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET | TEXTURE_CREATE_FLAGS::COORDS_WRAP | TEXTURE_CREATE_FLAGS::FILTER_POINT);
	}

	_quad = _pResMan->LoadMesh("std#plane");
		
	whiteTexture = _pResMan->LoadTexture("std#white_texture", TEXTURE_CREATE_FLAGS());
	fontTexture = _pResMan->LoadTexture("ExportedFont.dds", TEXTURE_CREATE_FLAGS());

	LOG("Render initialized");
}

void Render::Free()
{
	for (auto &r : _records)
		r.buffer.Reset();

	fontTexture.Reset();
	whiteTexture.Reset();
	_quad.Reset();
	renderTarget.Reset();

	_render_textures.clear();
	_runtime_shaders.clear();
	_records.clear();
}

void Render::GetRenderTexture2D(OUT ITexture **texOut, uint width, uint height, TEXTURE_FORMAT format)
{
	ITexture *tex = getRenderTargetTexture2d(width, height, format);
	*texOut = tex;
}

void Render::ReleaseRenderTexture2D(ITexture *texIn)
{
	releaseTexture2d(texIn);
}

void Render::ShadersReload()
{
	LOG("Shaders reloading...");
	_runtime_shaders.clear();
}

void Render::PreprocessStandardShader(IShader** pShader, const ShaderRequirement* shaderReq)
{
	*pShader = getShader(*shaderReq);
}

API_RESULT Render::GetName(const char** pName)
{
	*pName = "Render";
	return S_OK;
}
