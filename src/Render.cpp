#include "pch.h"
#include "Render.h"
#include "Core.h"
#include "SceneManager.h"
#include "simplecpp.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

/////////////////////////
// Render
/////////////////////////

void Render::_update()
{
	auto before = _texture_pool.size();

	_texture_pool.erase(std::remove_if(_texture_pool.begin(), _texture_pool.end(), [&](const TexturePoolable& r) -> bool
	{
		return r.free == 1 && (_pCore->frame() - r.frame) > 3;
	}),
    _texture_pool.end());

	if (before != _texture_pool.size())
		LOG_FORMATTED("Render::_update() textures removed. was = %i, now = %i", before, _texture_pool.size());
}

IShader* Render::_get_shader(const ShaderRequirement &req)
{
	IShader *pShader = nullptr;

	auto it = _shaders_pool.find(req);

	if (it != _shaders_pool.end())
	{
		WRL::ComPtr<IShader>& shaderPtr = it->second;
		return shaderPtr.Get();
	}
	else
	{
		const auto process_shader = [&](const char *&ppTextOut, const char *ppTextIn, const string& fileNameIn, const string&& fileNameOut, int type) -> void
		{
			simplecpp::DUI dui;

			if (_is_opengl())
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
			if (_is_opengl())
				size += 13;

			char *tmp = new char[size + 1];
			if (_is_opengl())
			{
				strncpy(tmp + 0, "#version 420\n", 13);
				strncpy(tmp + 13, out.c_str(), size - 13);
			} else
				strncpy(tmp, out.c_str(), size);

			tmp[size] = '\0';

			ppTextOut = tmp;
		};

		const char *text;

		WRL::ComPtr<IShaderFile> tergetShader;

		if (req.pass == RENDER_PASS::ID)
			tergetShader = _idShader;
		else
			tergetShader = _forwardShader;

		tergetShader->GetText(&text);

		const char *pFileIn;
		tergetShader->GetFile(&pFileIn);
		string fileIn = pFileIn;

		const char *textVertOut; 
		const char *textFragOut;

		process_shader(textVertOut, text, fileIn, "out_v.shader", 0);
		process_shader(textFragOut, text, fileIn, "out_f.shader", 1);

		bool compiled = SUCCEEDED(_pResMan->CreateShader(&pShader, textVertOut, nullptr, textFragOut)) && pShader != nullptr;

		if (!compiled)
		{
			LOG_FATAL("Render::_get_shader(): can't compile standard shader\n");
			_shaders_pool.emplace(req, WRL::ComPtr<IShader>(nullptr));
		}
		else
			_shaders_pool.emplace(req, WRL::ComPtr<IShader>(pShader));
	}

	return pShader;
}

bool Render::_is_opengl()
{
	const char *gapi;
	_pCoreRender->GetName(&gapi);
	return (strcmp("GLCoreRender", gapi) == 0);	
}

void Render::_get_render_mesh_vec(vector<RenderMesh>& meshes_vec)
{
	SceneManager *sm = (SceneManager*)_pSceneMan;	

	for (tree<IGameObject*>::iterator it = sm->_gameobjects.begin(); it != sm->_gameobjects.end(); ++it)
	{
		IGameObject *go = *it;		
		IModel *model = dynamic_cast<IModel*>(go);
		if (model)
		{
			uint meshes;
			model->GetNumberOfMesh(&meshes);

			for (auto j = 0u; j < meshes; j++)
			{
				uint id;
				model->GetID(&id);

				IMesh *mesh = nullptr;
				model->GetMesh(&mesh, j);

				mat4 mat;
				model->GetModelMatrix(&mat);

				meshes_vec.push_back({id, mesh, mat});
			}
		}
	}
}

void Render::_draw_meshes(const mat4& ViewProjMat, vector<RenderMesh>& meshes, RENDER_PASS pass)
{
	for(RenderMesh &renderMesh : meshes)
	{
		INPUT_ATTRUBUTE attribs;
		renderMesh.mesh->GetAttributes(&attribs);		

		IShader *shader = _get_shader({attribs, pass});

		if (!shader)
			continue;

		_pCoreRender->SetMesh(renderMesh.mesh);
		_pCoreRender->SetShader(shader);		

		mat4 MVP = ViewProjMat * renderMesh.modelMat;
		shader->SetMat4Parameter("MVP", &MVP);

		shader->SetMat4Parameter("NM", &mat4());		

		if (pass == RENDER_PASS::ID)
		{
			shader->SetUintParameter("model_id", renderMesh.model_id);
		} else if (pass == RENDER_PASS::FORWARD)
		{
			shader->SetVec4Parameter("main_color", &vec4(1.0f, 1.0f, 1.0f, 1.0f));
			shader->SetVec4Parameter("nL", &(vec4(1.0f, -2.0f, 3.0f, 0.0f).Normalized()));
		}

		shader->FlushParameters();		

		_pCoreRender->Draw(renderMesh.mesh);
	}
}

ITexture* Render::_get_render_target_texture_2d(uint width, uint height, TEXTURE_FORMAT format)
{
	for (TexturePoolable &tp : _texture_pool)
	{
		if (tp.free)
		{
			if (width == tp.width && height == tp.height && format == tp.format)
			{
				tp.free = 0;
				tp.frame = _pCore->frame();
				return tp.tex.Get();
			}
		}
	}

	TEXTURE_CREATE_FLAGS flags = TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET | TEXTURE_CREATE_FLAGS::COORDS_WRAP | TEXTURE_CREATE_FLAGS::FILTER_POINT;

	ITexture *tex;
	_pResMan->CreateTexture(&tex, width, height, TEXTURE_TYPE::TYPE_2D, format, flags);

	_texture_pool.push_back({_pCore->frame(), 0, width, height, format, WRL::ComPtr<ITexture>(tex)});

	return tex;
}
void Render::_release_texture_2d(ITexture *tex)
{
	for (TexturePoolable &tp : _texture_pool)
	{
		if (tp.tex.Get() == tex)
			tp.free = 1;
	}
}

Render::Render(ICoreRender *pCoreRender) : _pCoreRender(pCoreRender)
{
	_pCore->GetSubSystem((ISubSystem**)&_pSceneMan, SUBSYSTEM_TYPE::SCENE_MANAGER);
	_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
	_pCore->GetSubSystem((ISubSystem**)&_fsystem, SUBSYSTEM_TYPE::FILESYSTEM);
}

Render::~Render()
{
}

void Render::Init()
{
	_pCoreRender->SetDepthState(1);

	_pCore->AddUpdateCallback(std::bind(&Render::_update, this));


	// Shaders
	IShaderFile *shader;

	_pResMan->LoadShaderFile(&shader, "mesh.shader");
	_forwardShader =  WRL::ComPtr<IShaderFile>(shader);

	_pResMan->LoadShaderFile(&shader, "id.shader");
	_idShader =  WRL::ComPtr<IShaderFile>(shader);

	// Render Targets
	IRenderTarget *RT;
	_pResMan->CreateRenderTarget(&RT);
	_idTexRT = WRL::ComPtr<IRenderTarget>(RT);

	// Meshes
	// get all default meshes and release only for test
	WRL::ComPtr<IMesh> axesMesh;
	WRL::ComPtr<IMesh> axesArrowMesh;
	WRL::ComPtr<IMesh> gridMesh;
	IMesh *mesh;

	_pResMan->LoadMesh(&mesh, "std#axes");
	axesMesh = WRL::ComPtr<IMesh>(mesh);

	_pResMan->LoadMesh(&mesh, "std#axes_arrows");
	axesArrowMesh = WRL::ComPtr<IMesh>(mesh);

	_pResMan->LoadMesh(&mesh, "std#grid");
	gridMesh = WRL::ComPtr<IMesh>(mesh);

	// Create texture for test
	ITexture *tex;
	_pResMan->CreateTexture(&tex, 300, 300, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::R32UI, TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET | TEXTURE_CREATE_FLAGS::COORDS_WRAP | TEXTURE_CREATE_FLAGS::FILTER_POINT);
	tex->AddRef();
	tex->Release();


	LOG("Render initialized");
}

void Render::Free()
{
	_idTexRT.Reset();

	_forwardShader.Reset();
	_idShader.Reset();

	_texture_pool.clear();
	_shaders_pool.clear();	
}

void Render::RenderFrame(const ICamera *pCamera)
{
	mat4 ViewProjMat;

	uint w, h;
	_pCoreRender->GetViewport(&w, &h);
	float aspect = (float)w / h;

	const_cast<ICamera*>(pCamera)->GetViewProjectionMatrix(&ViewProjMat, aspect);
	
	vector<RenderMesh> meshes;
	_get_render_mesh_vec(meshes);


	// ID pass
	// Render all models ID (for picking)
	// to R32UI texture
	//
	ITexture *tex = _get_render_target_texture_2d(w, h, TEXTURE_FORMAT::R32UI);
	ITexture *depthTex = _get_render_target_texture_2d(w, h, TEXTURE_FORMAT::D24S8);

	_idTexRT->SetColorTexture(0, tex);
	_idTexRT->SetDepthTexture(depthTex);

	_pCoreRender->SetCurrentRenderTarget(_idTexRT.Get());
	{
		_pCoreRender->Clear();

		_draw_meshes(ViewProjMat, meshes, RENDER_PASS::ID);
	}

	//
	//IInput *i;
	//_pCore->GetSubSystem((ISubSystem**)&i, SUBSYSTEM_TYPE::INPUT);
	//int pr;
	//i->IsMoisePressed(&pr, MOUSE_BUTTON::LEFT);
	//if (pr)
	//{
	//	uint curX, curY;
	//	i->GetMousePos(&curX, &curY);

	//	ICoreTexture *coreTex;
	//	tex->GetCoreTexture(&coreTex);
	//	uint data;
	//	uint read;
	//	_pCoreRender->ReadPixel2D(coreTex, &data, &read, curX, curY);
	//	if (read > 0)
	//	{
	//		LOG_FORMATTED("Id = %i", data);
	//	}
	//}
	
	_pCoreRender->RestoreDefaultRenderTarget();
	_idTexRT->UnbindAll();

	_release_texture_2d(tex);
	_release_texture_2d(depthTex);

	// Forward pass
	// to default framebuffer
	//
	{
		_pCoreRender->Clear();

		_draw_meshes(ViewProjMat, meshes, RENDER_PASS::FORWARD);
	}
}

API Render::RenderPassIDPass(const ICamera *pCamera, ITexture *tex, ITexture *depthTex)
{
	mat4 ViewProjMat;

	uint w, h;
	//_pCoreRender->GetViewport(&w, &h);
	tex->GetWidth(&w);
	tex->GetHeight(&h);

	float aspect = (float)w / h;

	const_cast<ICamera*>(pCamera)->GetViewProjectionMatrix(&ViewProjMat, aspect);
	
	vector<RenderMesh> meshes;
	_get_render_mesh_vec(meshes);

	// ID pass
	// Render all models ID (for picking)
	// to R32UI texture
	//
	_idTexRT->SetColorTexture(0, tex);
	_idTexRT->SetDepthTexture(depthTex);

	_pCoreRender->SetCurrentRenderTarget(_idTexRT.Get());
	{
		_pCoreRender->Clear();

		_draw_meshes(ViewProjMat, meshes, RENDER_PASS::ID);
	}
	_idTexRT->UnbindAll();
	_pCoreRender->RestoreDefaultRenderTarget();

	return S_OK;
}

API Render::GetRenderTexture2D(OUT ITexture **texOut, uint width, uint height, TEXTURE_FORMAT format)
{
	ITexture *tex = _get_render_target_texture_2d(width, height, format);
	*texOut = tex;
	return S_OK;
}

API Render::ReleaseRenderTexture2D(ITexture *texIn)
{
	_release_texture_2d(texIn);
	return S_OK;
}

API Render::ShadersReload()
{
	LOG("Shaders reloading...");
	_idShader->Reload();
	_forwardShader->Reload();
	_shaders_pool.clear();
	return S_OK;
}

API Render::PreprocessStandardShader(IShader** pShader, const ShaderRequirement* shaderReq)
{
	*pShader = _get_shader(*shaderReq);
	return S_OK;
}

API Render::GetName(const char** pName)
{
	*pName = "Render";
	return S_OK;
}
