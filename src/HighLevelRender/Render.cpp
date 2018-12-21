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

void Render::renderForward(RenderBuffers& buffers, vector<RenderMesh>& meshes)
{
	renderTarget->SetColorTexture(0, buffers.colorHDR.Get());
	renderTarget->SetDepthTexture(buffers.depth.Get());
	_pCoreRender->SetCurrentRenderTarget(renderTarget.Get());
	{
		_pCoreRender->Clear();

		drawMeshes(meshes, RENDER_PASS::FORWARD);
	}
	_pCoreRender->RestoreDefaultRenderTarget();
}

void Render::renderEnginePost(RenderBuffers& buffers)
{
	//_pCoreRender->PushStates();

	INPUT_ATTRUBUTE attribs;
	_postPlane->GetAttributes(&attribs);

	IShader *shader = getShader({attribs, RENDER_PASS::ENGINE_POST});
	_pCoreRender->SetShader(shader);
	setShaderPostParameters(RENDER_PASS::ENGINE_POST, shader);

	_pCoreRender->BindTexture(0, buffers.colorHDR.Get());

	_pCoreRender->SetDepthTest(0);	

	renderTarget->SetColorTexture(0, buffers.color.Get());
	_pCoreRender->SetCurrentRenderTarget(renderTarget.Get());_pCoreRender->SetCurrentRenderTarget(renderTarget.Get());
	{
		_pCoreRender->Draw(_postPlane.Get());
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

	vector<RenderMesh> meshes;
	getRenderMeshes(meshes);

	RenderBuffers buffers = initBuffers(w, h);

	// Forward pass
	//
	renderForward(buffers, meshes);

	// Engine post pass
	//
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

API Render::RenderPassIDPass(const ICamera *pCamera, ITexture *tex, ITexture *depthTex)
{
	uint w, h;
	tex->GetWidth(&w);
	tex->GetHeight(&h);
	float aspect = (float)w / h;

	mat4 ViewProjMat;
	const_cast<ICamera*>(pCamera)->GetViewProjectionMatrix(&ViewProjMat, aspect);

	mat4 ViewMat;
	const_cast<ICamera*>(pCamera)->GetViewMatrix(&ViewMat);
	
	vector<RenderMesh> meshes;
	getRenderMeshes(meshes);

	renderTarget->SetColorTexture(0, tex);
	renderTarget->SetDepthTexture(depthTex);

	_pCoreRender->SetCurrentRenderTarget(renderTarget.Get());
	{
		_pCoreRender->Clear();

		drawMeshes(meshes, RENDER_PASS::ID);
	}
	_pCoreRender->RestoreDefaultRenderTarget();

	renderTarget->UnbindAll();

	return S_OK;
}

void Render::setShaderPostParameters(RENDER_PASS pass, IShader *shader)
{
}

void Render::_update()
{
	auto before = _texture_pool.size();

	_texture_pool.erase(std::remove_if(_texture_pool.begin(), _texture_pool.end(), [&](const TexturePoolable& r) -> bool
	{
		return r.free == 1 && (_pCore->frame() - r.frame) > 3;
	}),
    _texture_pool.end());

	//if (before != _texture_pool.size())
	//	LOG_FORMATTED("Render::_update() textures removed. was = %i, now = %i", before, _texture_pool.size());
}

IShader* Render::getShader(const ShaderRequirement &req)
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

			if (isOpenGL())
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
			if (isOpenGL())
				size += 13;

			char *tmp = new char[size + 1];
			if (isOpenGL())
			{
				strncpy(tmp + 0, "#version 420\n", 13);
				strncpy(tmp + 13, out.c_str(), size - 13);
			} else
				strncpy(tmp, out.c_str(), size);

			tmp[size] = '\0';

			ppTextOut = tmp;
		};

		const char *text;

		WRL::ComPtr<IShaderFile> targetShader;

		switch (req.pass)
		{
			case RENDER_PASS::ID: targetShader = _idShader; break;
			case RENDER_PASS::FORWARD: targetShader = _forwardShader; break;
			case RENDER_PASS::ENGINE_POST: targetShader = _postShader; break;
		}

		targetShader->GetText(&text);

		const char *pFileIn;
		targetShader->GetFile(&pFileIn);
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

bool Render::isOpenGL()
{
	const char *gapi;
	_pCoreRender->GetName(&gapi);
	return (strcmp("GLCoreRender", gapi) == 0);	
}

void Render::getRenderMeshes(vector<RenderMesh>& meshes_vec)
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

void Render::setShaderMeshParameters(RENDER_PASS pass, RenderMesh *mesh, IShader *shader)
{
	if (mesh)
	{
		mat4 MVP = ViewProjMat * mesh->modelMat;
		shader->SetMat4Parameter("MVP", &MVP);

		mat4 M = mesh->modelMat;
		mat4 NM = M.Inverse().Transpose();
		shader->SetMat4Parameter("NM", &NM);
	}

	if (pass == RENDER_PASS::ID)
	{
		shader->SetUintParameter("model_id", mesh->model_id);
	} else if (pass == RENDER_PASS::FORWARD)
	{
		shader->SetVec4Parameter("main_color", &vec4(1.0f, 1.0f, 1.0f, 1.0f));
		shader->SetVec4Parameter("nL_world", &(vec4(1.0f, -2.0f, 3.0f, 0.0f).Normalized()));
	}

	shader->FlushParameters();
}

void Render::drawMeshes(vector<RenderMesh>& meshes, RENDER_PASS pass)
{
	for(RenderMesh &renderMesh : meshes)
	{
		INPUT_ATTRUBUTE attribs;
		renderMesh.mesh->GetAttributes(&attribs);		

		IShader *shader = getShader({attribs, pass});

		if (!shader)
			continue;
		
		_pCoreRender->SetShader(shader);
		setShaderMeshParameters(pass, &renderMesh, shader);		

		_pCoreRender->Draw(renderMesh.mesh);
	}
}

ITexture* Render::getRenderTargetTexture2d(uint width, uint height, TEXTURE_FORMAT format)
{
	for (TexturePoolable &tex : _texture_pool)
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

	ITexture *tex;
	_pResMan->CreateTexture(&tex, width, height, TEXTURE_TYPE::TYPE_2D, format, flags);

	_texture_pool.push_back({_pCore->frame(), 0, width, height, format, WRL::ComPtr<ITexture>(tex)});

	return tex;
}
void Render::releaseTexture2d(ITexture *tex)
{
	for (TexturePoolable &tp : _texture_pool)
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
	ret.directLight = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::RGB16F);
	ret.normal = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::RGB8);
	ret.shading = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::RGB8);
	ret.id = getRenderTargetTexture2d(w, h, TEXTURE_FORMAT::R32UI);

	return ret;
}

void Render::releaseBuffers(RenderBuffers& buffers)
{
	releaseTexture2d(buffers.color.Get());		buffers.color = nullptr;
	releaseTexture2d(buffers.colorHDR.Get());	buffers.colorHDR = nullptr;
	releaseTexture2d(buffers.depth.Get());		buffers.depth = nullptr;
	releaseTexture2d(buffers.directLight.Get());	buffers.directLight = nullptr;
	releaseTexture2d(buffers.normal.Get());		buffers.normal = nullptr;
	releaseTexture2d(buffers.shading.Get());		buffers.shading = nullptr;
	releaseTexture2d(buffers.id.Get());			buffers.id = nullptr;
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
	_pCoreRender->SetDepthTest(1);

	_pCore->AddUpdateCallback(std::bind(&Render::_update, this));

	// Shaders
	IShaderFile *shader;

	_pResMan->LoadShaderFile(&shader, "mesh.shader");
	_forwardShader =  WRL::ComPtr<IShaderFile>(shader);

	_pResMan->LoadShaderFile(&shader, "id.shader");
	_idShader =  WRL::ComPtr<IShaderFile>(shader);

	_pResMan->LoadShaderFile(&shader, "post\\engine_post.shader");
	_postShader =  WRL::ComPtr<IShaderFile>(shader);

	// Render Targets
	IRenderTarget *RT;
	_pResMan->CreateRenderTarget(&RT);
	renderTarget = WRL::ComPtr<IRenderTarget>(RT);

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

	_pResMan->LoadMesh(&mesh, "std#plane");
	_postPlane = WRL::ComPtr<IMesh>(mesh);

	// Create texture for test
	ITexture *tex;
	_pResMan->CreateTexture(&tex, 300, 300, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::R32UI, TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET | TEXTURE_CREATE_FLAGS::COORDS_WRAP | TEXTURE_CREATE_FLAGS::FILTER_POINT);
	tex->AddRef();
	tex->Release();

	LOG("Render initialized");
}

void Render::Free()
{
	_postPlane.Reset();
	renderTarget.Reset();
	_forwardShader.Reset();
	_idShader.Reset();
	_texture_pool.clear();
	_shaders_pool.clear();	
}

API Render::GetRenderTexture2D(OUT ITexture **texOut, uint width, uint height, TEXTURE_FORMAT format)
{
	ITexture *tex = getRenderTargetTexture2d(width, height, format);
	*texOut = tex;
	return S_OK;
}

API Render::ReleaseRenderTexture2D(ITexture *texIn)
{
	releaseTexture2d(texIn);
	return S_OK;
}

API Render::ShadersReload()
{
	LOG("Shaders reloading...");
	_idShader->Reload();
	_forwardShader->Reload();
	_postShader->Reload();
	_shaders_pool.clear();
	return S_OK;
}

API Render::PreprocessStandardShader(IShader** pShader, const ShaderRequirement* shaderReq)
{
	*pShader = getShader(*shaderReq);
	return S_OK;
}

API Render::GetName(const char** pName)
{
	*pName = "Render";
	return S_OK;
}
