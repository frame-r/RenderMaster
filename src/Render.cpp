#include "Render.h"
#include "Core.h"
#include <list>

using std::string;
using std::list;
using std::vector;
using strit = string::iterator;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

/////////////////////////
// Preprocessor
/////////////////////////

enum DIRECTIVE
{
	UNKNOWN,
	IFDEF,
	ELSE,
	ELSEIF,
	ENDIF
};

class Preprocessor
{
	list<string> defines;

	DIRECTIVE get_directive(strit it, strit str_end);
	string get_next_str(strit& it, strit str_end);
	bool evaluate_define_value(strit& it, strit str_end);
	void move_it_to_end_dirictive(strit& it, strit str_end);

public:

	void set_define(const string& def);
	void erase_define(const string& def);
	bool define_exist(const string& def);
	void run(list<string>& text);
};

void Preprocessor::set_define(const string& def)
{
	defines.push_back(def);
}

void Preprocessor::erase_define(const string& def)
{
	defines.remove(def);
}

bool Preprocessor::define_exist(const string& def)
{
	return find(defines.begin(), defines.end(), def) != defines.end();
}

DIRECTIVE Preprocessor::get_directive(strit str_it, strit str_end)
{
	string str = string(str_it, str_end);

	if (str.compare(0, 5, "ifdef") == 0)
		return IFDEF;
	else if (str.compare(0, 4, "else") == 0)
		return ELSE;
	else if (str.compare(0, 4, "elif") == 0)
		return ELSEIF;
	else if (str.compare(0, 5, "endif") == 0)
		return ENDIF;

	return UNKNOWN;
}


string Preprocessor::get_next_str(strit& it, strit str_end)
{
	while (it != str_end && *it == ' ') it++; //skip whitespace
	while (it != str_end && *it == '\n') it++; //skip EOL

	if (it == str_end) return string();

	if (*it == '&' && *(it + 1) == '&')
	{
		it += 2;
		return string("&&");
	}

	if (*it == '|' && *(it + 1) == '|')
	{
		it += 2;
		return string("||");
	}

	if (*it == '!' || isalpha(*it) || *it == '_')
	{
		strit def_begin = it;
		it++;
		while (it != str_end && (isalnum(*it) || *it == '_')) it++;
		return string(def_begin, it);
	}

	assert(false); // invalid string
	return string();
}

bool Preprocessor::evaluate_define_value(strit& it, strit str_end)
{
	bool l_operand = false;
	bool prev_was_operation = false;
	bool op_is_and = false;

	while (true)
	{
		string str = get_next_str(it, str_end);
		if (str.empty()) break;

		if (str == "&&")
		{
			prev_was_operation = true;
			op_is_and = true;
		}
		else if (str == "||")
		{
			prev_was_operation = true;
			op_is_and = false;
		}
		else
		{
			bool value;

			if (str[0] == '!')
			{
				string def = string(str.begin() + 1, str.end());
				value = !define_exist(def);
			}
			else
				value = define_exist(str);


			if (prev_was_operation)
			{
				if (op_is_and)
					l_operand = l_operand && value;
				else
					l_operand = l_operand || value;
				prev_was_operation = false;
			}
			else
			{
				l_operand = value;
			}
		}
	}

	return l_operand;
}

void Preprocessor::move_it_to_end_dirictive(strit& it, strit str_end)
{
	while (it != str_end && *it != ' ') it++;
}

void Preprocessor::run(list<string>& text)
{
	struct TxtBlock
	{
		list<string>::iterator list_it1;
		strit str_it1;
		list<string>::iterator list_it2;
		strit str_it2;
	};

	bool fisrt_block_finished = false;
	bool second_block_started = false;
	TxtBlock block_tmp;
	vector<TxtBlock> text_to_remove;

	vector<list<string>::iterator> lines_to_delete_tab;

	for (auto it = text.begin(); it != text.end(); it++)
	{
		for (auto str_it = it->begin(); str_it != it->end(); )
		{
			DIRECTIVE directive = UNKNOWN;

			if (*str_it == '#')
			{
				directive = get_directive(str_it + 1, it->end());
			}

			switch (directive)
			{
			case UNKNOWN: str_it++; break;

			case IFDEF:
				block_tmp.list_it1 = it;
				block_tmp.str_it1 = str_it;

				move_it_to_end_dirictive(str_it, it->end());

				if (evaluate_define_value(str_it, it->end()))
				{
					block_tmp.list_it2 = it;
					block_tmp.str_it2 = str_it;
					fisrt_block_finished = true;
					text_to_remove.push_back(block_tmp);
				}
				break;

			case ELSE:
				if (fisrt_block_finished && !second_block_started)
				{
					block_tmp.list_it2++;
					for (list<string>::iterator i = block_tmp.list_it2; i != it; i++)
						lines_to_delete_tab.push_back(i);

					block_tmp.list_it1 = it;
					block_tmp.str_it1 = str_it;
					second_block_started = true;
					move_it_to_end_dirictive(str_it, it->end());
				}
				else if (fisrt_block_finished && second_block_started)
				{
					move_it_to_end_dirictive(str_it, it->end());
				}
				else
				{
					move_it_to_end_dirictive(str_it, it->end());

					block_tmp.list_it2 = it;
					block_tmp.str_it2 = str_it;
					text_to_remove.push_back(block_tmp);
					fisrt_block_finished = true;
				}
				break;

			case ELSEIF:
				if (fisrt_block_finished && !second_block_started)
				{
					block_tmp.list_it2++;
					for (list<string>::iterator i = block_tmp.list_it2; i != it; i++)
						lines_to_delete_tab.push_back(i);

					block_tmp.list_it1 = it;
					block_tmp.str_it1 = str_it;
					second_block_started = true;
					move_it_to_end_dirictive(str_it, it->end());
					evaluate_define_value(str_it, it->end());
				}
				else if (fisrt_block_finished && second_block_started)
				{
					move_it_to_end_dirictive(str_it, it->end());
					evaluate_define_value(str_it, it->end());
				}
				else // fisrt_block_finished == false
				{
					move_it_to_end_dirictive(str_it, it->end());

					if (evaluate_define_value(str_it, it->end()))
					{
						block_tmp.list_it2 = it;
						block_tmp.str_it2 = str_it;
						fisrt_block_finished = true;
						text_to_remove.push_back(block_tmp);
					}
				}
				break;

			case ENDIF:
				if (fisrt_block_finished && !second_block_started)
				{
					block_tmp.list_it2++;
					for (list<string>::iterator i = block_tmp.list_it2; i != it; i++)
						lines_to_delete_tab.push_back(i);


					block_tmp.list_it1 = it;
					block_tmp.str_it1 = str_it;
				}

				move_it_to_end_dirictive(str_it, it->end());

				block_tmp.list_it2 = it;
				block_tmp.str_it2 = str_it;

				text_to_remove.push_back(block_tmp);

				fisrt_block_finished = false;
				second_block_started = false;

				break;

			default: str_it++; break;
			}
		}
	}

	// remove one tab
	for (auto& str_it : lines_to_delete_tab)
	{
		if (!str_it->empty())
			if (str_it->at(0) == '\t')
				str_it->erase(0, 1);
	}
	
	// remove text blocks
	for (auto& txt_block : text_to_remove)
	{
		if (txt_block.list_it1 == txt_block.list_it2) // one line
		{
			if (txt_block.str_it1 == txt_block.list_it1->begin() && txt_block.str_it2 == txt_block.list_it2->end())
				text.erase(txt_block.list_it1);
			else
				txt_block.list_it1->erase(txt_block.str_it1, txt_block.str_it2);
		}
		else
		{
			int lines = (int)distance(txt_block.list_it1, txt_block.list_it2);

			auto it1 = txt_block.list_it1;
			++it1;
			auto it2 = txt_block.list_it2;

			// remove begining of line
			if (txt_block.str_it1 == txt_block.list_it1->begin())
				text.erase(txt_block.list_it1);
			else
				txt_block.list_it1->erase(txt_block.str_it1, txt_block.list_it1->end());

			// delete whole lines
			if (lines > 1)
				text.erase(it1, it2);

			// remove ending of line
			if (txt_block.str_it2 == txt_block.list_it2->end())
				text.erase(txt_block.list_it2);
			else
				txt_block.list_it2->erase(txt_block.list_it2->begin(), txt_block.str_it2);
		}
	}

	// remove empty lines
	vector<list<string>::iterator> empty_lines;

	for (auto it = text.begin(); it != text.end(); ++it)
		if (*it == "" | *it == "\t" || *it == "\n")
			empty_lines.push_back(it);

	for (auto& line : empty_lines)
		text.erase(line);

}

/////////////////////////
// Render
/////////////////////////

list<string> make_lines_list(const char **text)
{
	list<string> ret;

	char **c = const_cast<char**>(text);

	while (*c)
	{
		ret.push_back(string(*c));
		c++;
	}

	return ret;
}

const char** make_char_pp(const list<string>& lines)
{
	char **ret = new char*[lines.size() + 1];	 

	memset(ret, 0, (lines.size() + 1) * sizeof(char*));

	int i = 0;
	for (auto it = lines.begin(); it != lines.end(); it++)
	{		
		ret[i] = new char[it->size() + 1];
		strncpy(ret[i], it->c_str(), it->size());
		ret[i][it->size()] = '\0';
		i++;
	}

	return const_cast<const char**>(ret);
}

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
		ShaderText st;

		auto get_shader_preprocessed = [&](const char **&ppTextOut, int &num_lines, const char **ppTextIn, const string&& fileName) -> void
		{
			list<string> l = make_lines_list(ppTextIn);

			Preprocessor proc;

			if ((int)(req.attributes & INPUT_ATTRUBUTE::NORMAL)) proc.set_define("ENG_INPUT_NORMAL");
			if ((int)(req.attributes & INPUT_ATTRUBUTE::TEX_COORD)) proc.set_define("ENG_INPUT_TEXCOORD");
			if ((int)(req.attributes & INPUT_ATTRUBUTE::COLOR)) proc.set_define("ENG_INPUT_COLOR");
			if (req.alphaTest) proc.set_define("ENG_ALPHA_TEST");

			proc.run(l);

			// save to file
			//_export_shader_to_file(l, std::forward<const string>(fileName));

			ppTextOut = make_char_pp(l);
			num_lines = (int)l.size();
		};

		get_shader_preprocessed(st.pVertText, st.vertNumLines, pStandardShaderText.pVertText, "out_v.shader");
		get_shader_preprocessed(st.pFragText, st.fragNumLines, pStandardShaderText.pFragText, "out_f.shader");

		_pCoreRender->CreateShader(&pShader, &st);

		delete_char_pp(st.pVertText);
		delete_char_pp(st.pFragText);

		_pResMan->AddToList(pShader);

		_shaders_pool.emplace(req, pShader);
	}

	return pShader;
}

void Render::_create_render_mesh_vec(vector<TRenderMesh>& meshes_vec)
{
	uint number{0};
	_pSceneMan->GetGameObjectsNumber(&number);

	for (auto i = 0u; i < number; i++)
	{
		IGameObject *go{ nullptr };
		RES_TYPE type;

		_pSceneMan->GetGameObject(&go, i);
		go->GetType(&type);

		if (type == RES_TYPE::MODEL)
		{
			IModel *model = reinterpret_cast<IModel*>(go);

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
	delete_char_pp(pStandardShaderText.pVertText);
	delete_char_pp(pStandardShaderText.pGeomText);
	delete_char_pp(pStandardShaderText.pFragText);
}

void Render::Init()
{
	_pResMan->LoadShaderText(&pStandardShaderText, "mesh_vertex", nullptr, "mesh_fragment");

	//dbg
	_get_shader({INPUT_ATTRUBUTE::TEX_COORD | INPUT_ATTRUBUTE::NORMAL, false});
	_get_shader({INPUT_ATTRUBUTE::TEX_COORD | INPUT_ATTRUBUTE::NORMAL, true});

	_pResMan->GetDefaultResource((IResource**)&_pAxesMesh, DEFAULT_RES_TYPE::AXES);

	LOG("Render initialized");
}

void Render::_draw_axes(const mat4& VP)
{
	INPUT_ATTRUBUTE a;
	_pAxesMesh->GetAttributes(&a);

	ICoreShader *shader = _get_shader({ a, false });
	_pCoreRender->SetShader(shader);

	_pCoreRender->SetUniform("MVP", &VP.el_1D[0], shader, SHADER_VARIABLE_TYPE::MATRIX4X4);

	vec4 main_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	_pCoreRender->SetUniform("main_color", &main_color.x, shader, SHADER_VARIABLE_TYPE::VECTOR4);

	_pCoreRender->SetDepthState(false);

	_pCoreRender->Draw(_pAxesMesh);

	_pCoreRender->SetDepthState(true);
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

		_pCoreRender->SetShader(shader);
		
		mat4 MVP = VP * renderMesh.modelMat;

		_pCoreRender->SetUniform("MVP", &MVP.el_1D[0], shader, SHADER_VARIABLE_TYPE::MATRIX4X4);

		vec4 main_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		_pCoreRender->SetUniform("main_color", &main_color.x, shader, SHADER_VARIABLE_TYPE::VECTOR4);

		if ((int)(a & INPUT_ATTRUBUTE::NORMAL))
		{
			mat4 NM;
			_pCoreRender->SetUniform("NM", &NM.el_1D[0], shader, SHADER_VARIABLE_TYPE::MATRIX4X4);

			vec3 nL = vec3(0.1f, 0.8f, -1.0f).Normalized();
			_pCoreRender->SetUniform("nL", &nL.x, shader, SHADER_VARIABLE_TYPE::VECTOR3);

		}

		_pCoreRender->Draw(renderMesh.mesh);
	}

	_draw_axes(VP);

	_pCoreRender->SwapBuffers();
}
