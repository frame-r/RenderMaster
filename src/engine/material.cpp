#include "pch.h"
#include "material.h"
#include "core.h"
#include "console.h"
#include "render.h"
#include "shader.h"
#include "resource_manager.h"
#include "material_manager.h"
#include "filesystem.h"
#include "thirdparty/pugixml/pugixml.hpp"
#include <sstream>

#define MAX_DEFINES_COUNT 10


void GenericMaterial::LoadXML()
{
	if (!FS->FileExist(path_.c_str()))
	{
		LogCritical("Material::LoadXML(): %s file not found", path_.c_str());
		return;
	}

	File f = FS->OpenFile(path_.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);

	size_t fileSize = (uint)f.FileSize();

	unique_ptr<char[]> txt = unique_ptr<char[]>(new char[fileSize + 1]);
	txt[fileSize] = '\0';

	f.Read((uint8*)txt.get(), fileSize);

	pugi::xml_document doc;
	doc.load_buffer(txt.get(), fileSize);

	pugi::xml_node mat = doc.child("generic_material");

	//
	// id
	auto mat_id = mat.attribute("id");
	id_ = mat_id.as_string();


	auto loadPass = [this](pugi::xml_node node, string& shader, PASS pass) -> void
	{
		//
		// defs
		for (pugi::xml_node def = node.child("def"); def; def = def.next_sibling("def"))
		{
			string id  = def.attribute("id").as_string();

			auto &d = defs_[id] = {};
			d.id = id;
			d.pass = pass;
			d.defaulValue = std::stoi(def.child_value());

			auto getEndingNumber = [](const string & str) -> int
			{
				size_t last_index = str.find_last_not_of("0123456789");
				string result = str.substr(last_index + 1);
				if (result.empty())
					return -1;
				return std::stoi(result);
			};

			for (int i = 0; i < MAX_DEFINES_COUNT; i++)
				d.values.push_back("");

			for (pugi::xml_attribute attr = def.first_attribute(); attr; attr = attr.next_attribute())
			{
				int i = getEndingNumber(attr.name());
				if (i == - 1 || i > MAX_DEFINES_COUNT)
					continue;

				d.values[i] = attr.value();
			}
		}

		//
		// params
		for (pugi::xml_node par = node.child("param"); par; par = par.next_sibling("param"))
		{
			string id = par.attribute("id").as_string();

			Param p;
			p.id = id;
			p.pass = pass;
			p.type = PARAM_TYPE::FLOAT;

			pugi::xml_attribute par_type = par.attribute("type");
			if (par_type)
				if (par_type.as_string() == string("color")) p.type = PARAM_TYPE::COLOR;
	
			string default_val = par.child_value();

			switch (p.type)
			{
				case PARAM_TYPE::FLOAT: if (!default_val.empty()) p.defaultValue.x = (float)std::stoi(default_val); break;
				case PARAM_TYPE::COLOR: sscanf(default_val.c_str(), "%f %f %f %f", &p.defaultValue.x, &p.defaultValue.y, &p.defaultValue.z, &p.defaultValue.w); break;
				default: LogWarning("Material::LoadXML(): %s default value for param=% is not supported", id_.c_str(), id.c_str());
			}

			nameToIndexMap_[id] = params_.size();
			params_.emplace_back(p);
		}

		//
		// textures
		for (pugi::xml_node par = node.child("texture"); par; par = par.next_sibling("texture"))
		{
			MaterialTexture tex{};
			tex.id = par.attribute("id").as_string();
			tex.slot = par.attribute("slot").as_int();
			tex.pass = pass;
			tex.path = par.child_value();

			if (par.attribute("condition"))
			{
				tex.hasCondition = 1;
				tex.condition = par.attribute("condition").as_string();
			}
			textures_.emplace_back(tex);
		}


		if (node.child("shader"))
			shader = node.child("shader").child_value();
	};

	
	loadPass(mat, shader_, PASS::ALL);

	if (mat.child("pass_deferred"))
		loadPass(mat.child("pass_deferred"), deferredShader_, PASS::DEFERRED);
	if (mat.child("pass_id"))
		loadPass(mat.child("pass_id"), idShader_, PASS::ID);
}

void GenericMaterial::Clear()
{
	id_.clear();
	params_.clear();
	defs_.clear();
	shader_.clear();
	deferredShader_.clear();
	idShader_.clear();
}

void Material::initializeFromParent()
{
	runtimeParams_.clear();
	for (auto& p : parent_->params_)
		runtimeParams_.push_back(p.defaultValue);

	runtimeTextures_.clear();
	for (auto& p : parent_->textures_)
		runtimeTextures_[p.id] = { p.path, RES_MAN->CreateStreamTexture(p.path.c_str(), TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS | TEXTURE_CREATE_FLAGS::FILTER_ANISOTROPY_8X) };
}

void Material::UploadShaderParameters(Shader *shader, PASS pass)
{
	for (auto i = 0; i < runtimeParams_.size(); i++)
	{
		GenericMaterial::Param& p = parent_->params_[i];
		if (p.pass != pass)
			continue;

		if(p.type == GenericMaterial::PARAM_TYPE::FLOAT)
			shader->SetFloatParameter(p.id.c_str(), runtimeParams_[i].x);
		else if(p.type == GenericMaterial::PARAM_TYPE::COLOR)
			shader->SetVec4Parameter(p.id.c_str(), &runtimeParams_[i]);
	}
}

void Material::BindShaderTextures(Shader* shader, PASS pass)
{
	Texture* texs[16]{};
	int maxSlots = 0;

	for (GenericMaterial::MaterialTexture& tex : parent_->textures_)
	{
		int slot = tex.slot;
		RuntimeTexture& t = runtimeTextures_[tex.id];

		if (tex.pass != pass)
			continue;

		if (tex.hasCondition)
		{
			if (!GetDef(tex.condition.c_str()))
				continue;
		}

		texs[slot] = t.ptr.get();
		
		if (slot + 1 > maxSlots)
			maxSlots = slot + 1;

		if (texs[slot])
		{
			string shader_par = "uv_transform_" + tex.id;
			shader->SetVec4Parameter(shader_par.c_str(), &t.uv);
		}
	}

	if (maxSlots)
	{
		shader->FlushParameters();
		CORE_RENDER->BindTextures(maxSlots, &texs[0]);
	}
}

Material::Material(const std::string& id, GenericMaterial* mat) : id_(id), parent_(mat)
{
	path_ = id_ + USER_MATERIAL_EXT;
	initializeFromParent();
}

void Material::SaveXML()
{
	pugi::xml_document doc;
	pugi::xml_node node = doc.append_child("material");

	node.append_attribute("id") = id_.c_str();
	node.append_attribute("generic_material") = parent_->id_.c_str();

	for (auto const& [d, val] : runtimeDefs_)
	{
		GenericMaterial::Def& genDef = parent_->defs_[d];

		if (genDef.defaulValue == val)
			continue;

		pugi::xml_node def = node.append_child("def");
		def.append_attribute("id") = d.c_str();
		def.append_child(pugi::node_pcdata).set_value(std::to_string(val).c_str());
	}

	for (auto i = 0; i < runtimeParams_.size(); i++)
	{
		GenericMaterial::Param& genParam = parent_->params_[i];

		if (genParam.defaultValue.Aproximately(runtimeParams_[i]))
			continue;

		pugi::xml_node param = node.append_child("param");
		param.append_attribute("id") = genParam.id.c_str();

		char buf[40]{};
		if (genParam.type == GenericMaterial::PARAM_TYPE::COLOR)
			sprintf(buf, "%f %f %f %f", runtimeParams_[i].x, runtimeParams_[i].y, runtimeParams_[i].z, runtimeParams_[i].w);
		else if (genParam.type == GenericMaterial::PARAM_TYPE::FLOAT)
			sprintf(buf, "%f", runtimeParams_[i].x);

		param.append_child(pugi::node_pcdata).set_value(buf);
	}

	for (const auto& [name, value] : runtimeTextures_)
	{
		auto it = std::find_if(parent_->textures_.begin(), parent_->textures_.end(),
		[name](const GenericMaterial::MaterialTexture& tex)-> bool
		{
			return tex.id == name;
		});

		if (it == parent_->textures_.end())
			continue;

		if (it->path == value.path && value.uv.Aproximately(vec4(1,1,0,0)))
			continue;

		pugi::xml_node tex_node = node.append_child("texture");

		tex_node.append_attribute("id") = name.c_str();

		char buf[40];
		sprintf(buf, "%f %f %f %f", value.uv.x, value.uv.y, value.uv.z, value.uv.w);
		tex_node.append_attribute("uv") = buf;

		tex_node.append_child(pugi::node_pcdata).set_value(value.path.c_str());
	}

	std::stringstream ss;
	doc.save(ss);
	
	File f = FS->OpenFile(path_.c_str(), FILE_OPEN_MODE::WRITE | FILE_OPEN_MODE::BINARY);

	f.WriteStr(ss.str().c_str());
}

auto DLLEXPORT Material::LoadXML() -> void
{
	if (!FS->FileExist(path_.c_str()))
	{
		LogCritical("Material::LoadXML(): %s file not found", path_.c_str());
		return;
	}

	File f = FS->OpenFile(path_.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);

	size_t fileSize = (uint)f.FileSize();

	unique_ptr<char[]> txt = unique_ptr<char[]>(new char[fileSize + 1]);
	txt[fileSize] = '\0';

	f.Read((uint8*)txt.get(), fileSize);

	pugi::xml_document doc;
	doc.load_buffer(txt.get(), fileSize);

	pugi::xml_node mat = doc.child("material");

	id_ = mat.attribute("id").value();
	string genericmat = mat.attribute("generic_material").value();

	MaterialManager *mm = _core->GetMaterialManager();
	parent_ = mm->GetGenericMaterial(genericmat.c_str());

	initializeFromParent();

	for (pugi::xml_node def = mat.child("def"); def; def = def.next_sibling("def"))
	{
		string id = def.attribute("id").as_string();
		if (!parent_->HasDef(id))
			continue;

		runtimeDefs_[id] = std::stoi(def.child_value());
	}

	for (pugi::xml_node def = mat.child("param"); def; def = def.next_sibling("param"))
	{
		if (!parent_->HasParam(def.attribute("id").as_string()))
			continue;

		size_t id = parent_->nameToIndexMap_[def.attribute("id").as_string()];
		GenericMaterial::Param &p = parent_->params_[id];

		vec4 vec = p.defaultValue;

		string val = def.child_value();
		switch (parent_->params_[id].type)
		{
			case GenericMaterial::PARAM_TYPE::FLOAT: vec.x = std::stof(val);  break;
			case GenericMaterial::PARAM_TYPE::COLOR: sscanf(val.c_str(), "%f %f %f %f", &vec.x, &vec.y, &vec.z, &vec.w); break;
		}

		runtimeParams_[id] = vec;
	}

	for (pugi::xml_node i = mat.child("texture"); i; i = i.next_sibling("texture"))
	{
		string id = i.attribute("id").as_string();
		runtimeTextures_[id].path = i.child_value();
		runtimeTextures_[id].ptr = RES_MAN->CreateStreamTexture(runtimeTextures_[id].path.c_str(), TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS | TEXTURE_CREATE_FLAGS::FILTER_ANISOTROPY_8X);

		vec4& uv = runtimeTextures_[id].uv;
		sscanf(i.attribute("uv").as_string(), "%f %f %f %f", &uv.x, &uv.y, &uv.z, &uv.w);
	}

	generateDefines();
}

auto DLLEXPORT Material::SetParamFloat(const char* def, float value) -> void
{
	if (!parent_->HasParam(def))
	{
		LogWarning("Material::SetParam(): Unable find '%s' parameter for material '%s'", def, id_.c_str());
		return;
	}
	auto id = parent_->nameToIndexMap_[def];
	runtimeParams_[id].x = value;
}

auto DLLEXPORT Material::SetParamFloat4(const char* def, const vec4& value) -> void
{
	if (!parent_->HasParam(def))
	{
		LogWarning("Material::SetParam(): Unable find '%s' parameter for material '%s'", def, id_.c_str());
		return;
	}

	auto id = parent_->nameToIndexMap_[def];
	runtimeParams_[id] = value;
}

auto DLLEXPORT Material::GetParamFloat4(const char* def) -> vec4
{
	auto id = parent_->nameToIndexMap_[def];
	return runtimeParams_[id];
}

auto DLLEXPORT Material::GetParamFloat(const char* def) -> float
{
	auto id = parent_->nameToIndexMap_[def];
	return runtimeParams_[id].x;
}

void Material::generateDefines()
{
	currentDefinesVec_.clear();
	currentDefinesString_.clear();

	for (auto const& [d, val] : runtimeDefs_)
	{
		assert(val < MAX_DEFINES_COUNT);

		GenericMaterial::Def genDef = parent_->defs_[d];

		if (genDef.values[val].empty())
		{
			if (val > 0)
			{
				currentDefinesString_ += d;
				currentDefinesVec_.push_back(d);
			}
		}
		else
		{
			currentDefinesString_ += genDef.values[val];
			currentDefinesVec_.push_back(genDef.values[val]);
		}

		if (!currentDefinesString_.empty())
			currentDefinesString_ += ',';
	}
}

void Material::SetDef(const char* def, int value)
{
	if (!parent_->HasDef(def))
	{
		LogWarning("Material::SetDef(): Unable find '%s' def for material '%s'", def, id_.c_str());
		return;
	}

	auto it = runtimeDefs_.find(def);
	if (it == runtimeDefs_.end())
	{
		runtimeDefs_[def] = value;
	}
	else
	{
		if (runtimeDefs_[def] == value)
			return;
		runtimeDefs_[def] = value;
	}

	//
	// Update defines

	generateDefines();
}

auto DLLEXPORT Material::GetDef(const char* name) -> int
{
	return runtimeDefs_[name];
}

auto DLLEXPORT Material::SetTexture(const char* name, const char* path) -> void
{
	runtimeTextures_[name] = { string(path), RES_MAN->CreateStreamTexture(path, TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS | TEXTURE_CREATE_FLAGS::FILTER_ANISOTROPY_8X) };
}

auto DLLEXPORT Material::GetTexture(const char* name) -> const char*
{
	return runtimeTextures_[name].path.c_str();
}

auto DLLEXPORT Material::SetUV(const char* name, const vec4& uv) -> void
{
	runtimeTextures_[name].uv = uv;
}

auto DLLEXPORT Material::GetUV(const char* name) -> vec4
{
	return runtimeTextures_[name].uv;
}

Shader* Material::GetShader(Mesh* mesh)
{
	Render *render = _core->GetRender();
	return render->GetShader(parent_->shader_.c_str(), mesh, currentDefinesVec_);
}
Shader* Material::GetDeferredShader(Mesh* mesh)
{
	Render *render = _core->GetRender();
	return render->GetShader(parent_->deferredShader_.c_str(), mesh, currentDefinesVec_);
}
Shader* Material::GetIdShader(Mesh* mesh)
{
	Render *render = _core->GetRender();
	return render->GetShader(parent_->idShader_.c_str(), mesh, currentDefinesVec_);
}
