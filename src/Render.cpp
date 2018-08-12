#include "Render.h"
#include "Core.h"
#include "SceneManager.h"
#include "Preprocessor.h"
#include <list>
using std::string;
using std::list;
using std::vector;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)



/////////////////////////
// Render
/////////////////////////

void Render::_export_shader_to_file(list<string>& text, const string&& file)
{
	IFile *pFile;
	
	_fsystem->OpenFile(&pFile, file.c_str(), FILE_OPEN_MODE::WRITE);

	for (auto& ll : text)
	{
		pFile->Write((uint8 *)ll.c_str(), (uint)ll.size());
	}

	pFile->CloseAndFree();
}

ICoreShader* Render::_get_shader(const ShaderRequirement &req)
{
	ICoreShader *pShader = nullptr;
	auto it = _shaders_pool.find(req);

	if (it != _shaders_pool.end())
	{
		pShader = it->second;
	}
	else
	{
		const auto process_shader = [&](const char *&ppTextOut, const char *ppTextIn, const string&& fileName, int type) -> void
		{
			const char **textOut;
			int numLinesOut;
			split_by_eol(textOut, numLinesOut, ppTextIn);
			list<string> lines = make_lines_list(textOut);
			delete_char_pp(textOut);

			list<string> lines_lang; 
			if (isOpenGL())
				lines_lang = get_file_content("language_gl.h");
			else
				lines_lang = get_file_content("language_dx11.h");
			
			lines.insert(lines.begin(), lines_lang.begin(), lines_lang.end());

			Preprocessor proc;
			if (isOpenGL())
				proc.SetDefine("ENG_OPENGL");
			else
				proc.SetDefine("ENG_DIRECTX11");
			if (type == 0)
				proc.SetDefine("ENG_SHADER_VERTEX");
			else if (type == 1)
				proc.SetDefine("ENG_SHADER_PIXEL");
			else if (type == 2)
				proc.SetDefine("ENG_SHADER_GEOMETRY");
			if ((int)(req.attributes & INPUT_ATTRUBUTE::NORMAL)) proc.SetDefine("ENG_INPUT_NORMAL");
			if ((int)(req.attributes & INPUT_ATTRUBUTE::TEX_COORD)) proc.SetDefine("ENG_INPUT_TEXCOORD");
			if ((int)(req.attributes & INPUT_ATTRUBUTE::COLOR)) proc.SetDefine("ENG_INPUT_COLOR");
			if (req.alphaTest) proc.SetDefine("ENG_ALPHA_TEST");

			proc.Run(lines);

			// save to file
			//_export_shader_to_file(l, std::forward<const string>(fileName));
			//LOG_FORMATTED("Render::_get_shader(): shader exported to \"%s\"", fileName.c_str());

			ppTextOut = make_char_p(lines);
		};

		ShaderText tmp;

		process_shader(tmp.pVertText, pStandardShaderText.pVertText, "out_v.shader", 0);
		process_shader(tmp.pFragText, pStandardShaderText.pFragText, "out_f.shader", 1);

		bool compiled = SUCCEEDED(_pCoreRender->CreateShader(&pShader, &tmp)) && pShader != nullptr;

		if (!compiled)
			LOG_FATAL("Render::_get_shader(): can't compile standard shader\n");
		else
		{
			_pResMan->AddToList(pShader);
			_shaders_pool.emplace(req, pShader);
		}

		delete[] tmp.pVertText;
		delete[] tmp.pFragText;
	}

	return pShader;
}

bool Render::isOpenGL()
{
	const char *gapi;
	_pCoreRender->GetName(&gapi);
	return (strcmp("GLCoreRender", gapi) == 0);	
}

void Render::_create_render_mesh_vec(vector<TRenderMesh>& meshes_vec)
{
	SceneManager *sm = (SceneManager*)_pSceneMan;	

	for (tree<IGameObject*>::iterator it = sm->_gameobjects.begin(); it != sm->_gameobjects.end(); ++it)
	{
		IGameObject *go = *it;
		
		RES_TYPE type;
		go->GetType(&type);

		if (type == RES_TYPE::MODEL)
		{
			IModel *model = dynamic_cast<IModel*>(go);

			uint meshes;
			model->GetNumberOfMesh(&meshes);

			for (auto j = 0u; j < meshes; j++)
			{
				ICoreMesh *mesh{ nullptr };
				model->GetMesh(&mesh, j);

				mat4 mat;
				model->GetModelMatrix(&mat);

				meshes_vec.push_back({mesh, mat});
			}
		}
	}
}

void Render::_sort_meshes(vector<TRenderMesh>& meshes)
{
	// not impl
}

Render::Render(ICoreRender *pCoreRender) : _pCoreRender(pCoreRender)
{
	_pCore->GetSubSystem((ISubSystem**)&_pSceneMan, SUBSYSTEM_TYPE::SCENE_MANAGER);
	_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
	_pCore->GetSubSystem((ISubSystem**)&_fsystem, SUBSYSTEM_TYPE::FILESYSTEM);
}

Render::~Render()
{
	delete pStandardShaderText.pVertText;
	delete pStandardShaderText.pGeomText;
	delete pStandardShaderText.pFragText;
}

void Render::Init()
{
	_pResMan->LoadShaderText(&pStandardShaderText, "mesh_vertex", nullptr, "mesh_fragment");

	//dbg
	_get_shader({INPUT_ATTRUBUTE::TEX_COORD | INPUT_ATTRUBUTE::NORMAL, false});
	_get_shader({INPUT_ATTRUBUTE::TEX_COORD | INPUT_ATTRUBUTE::NORMAL, true});

	LOG("Render initialized");
}

void Render::RenderFrame(const ICamera *pCamera)
{
	vector<TRenderMesh> _meshes;
	
	_pCoreRender->Clear();

	uint w, h;
	_pCoreRender->GetViewport(&w, &h);

	float aspect = (float)w / h;

	mat4 VP;
	const_cast<ICamera*>(pCamera)->GetViewProjectionMatrix(&VP, aspect);

	_create_render_mesh_vec(_meshes);
	_sort_meshes(_meshes);

	for(auto &renderMesh : _meshes)
	{
		INPUT_ATTRUBUTE a;
		renderMesh.mesh->GetAttributes(&a);		
		
		ICoreShader *shader = _get_shader({a, false});
		if (!shader)
			continue;

		_pCoreRender->SetShader(shader);
		
		mat4 MVP = VP * renderMesh.modelMat;

		_pCoreRender->SetUniform("MVP", &MVP.el_1D[0], shader, SHADER_VARIABLE_TYPE::MATRIX4X4);

		vec4 main_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		_pCoreRender->SetUniform("main_color", &main_color.x, shader, SHADER_VARIABLE_TYPE::VECTOR4);

		if ((int)(a & INPUT_ATTRUBUTE::NORMAL))
		{
			mat4 NM;
			_pCoreRender->SetUniform("NM", &NM.el_1D[0], shader, SHADER_VARIABLE_TYPE::MATRIX4X4);

			vec3 nL = vec3(1.0f, 0.0f, 1.0f).Normalized();
			_pCoreRender->SetUniform("nL", &nL.x, shader, SHADER_VARIABLE_TYPE::VECTOR3);
		}

		_pCoreRender->Draw(renderMesh.mesh);
	}
}

API Render::GetShader(ICoreShader** pShader, const ShaderRequirement* shaderReq)
{
	*pShader = _get_shader(*shaderReq);
	return S_OK;
}

API Render::GetName(const char** pName)
{
	*pName = "Render";
	return S_OK;
}
