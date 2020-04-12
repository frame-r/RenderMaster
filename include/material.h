#pragma once
#include "common.h"
#include <map>

//
// Generic material
//

struct GenericMaterial
{
	std::string path_;
	std::string id_;


	//
	// Parameters
	enum class PARAM_TYPE
	{
		FLOAT,
		COLOR
	};

	struct Param
	{
		std::string id;
		PASS pass;
		PARAM_TYPE type;
		vec4 defaultValue;
	};
	std::map<std::string, size_t> nameToIndexMap_;
	std::vector<Param> params_;


	//
	// Defines
	struct Def
	{
		std::string id;
		PASS pass;
		std::vector<std::string> values;
		int defaulValue{};
	};
	std::map<std::string, Def> defs_;


	//
	// Textures
	struct MaterialTexture
	{
		std::string id;
		int slot;
		PASS pass;
		std::string path;
		int hasCondition{0};
		std::string condition;
	};
	std::vector<MaterialTexture> textures_;


	//
	// Shaders
	std::string shader_;
	std::string deferredShader_;
	std::string idShader_;
	std::string wireframeShader_;
	std::string forwardShader_;

public:
	GenericMaterial(std::string path) : path_(path){}

	void LoadXML();
	void Clear();
	bool HasDef(const std::string& id) { return defs_.find(id) != defs_.end(); }
	bool HasParam(const std::string& id) { return nameToIndexMap_.find(id) != nameToIndexMap_.end(); }
};


//
// User material
//

class Material
{

	std::string id_;
	std::string path_;

	GenericMaterial *parent_{};

	std::vector<std::string> currentDefinesVec_;
	std::string currentDefinesString_;

	//
	// Runtime data
	//

	std::map<std::string, int> runtimeDefs_; // define -> value
	std::vector<vec4> runtimeParams_; // param -> value

	struct RuntimeTexture
	{
		std::string path;
		StreamPtr<Texture> ptr;
		vec4 uv{1, 1, 0, 0};
	};
	std::map<std::string, RuntimeTexture> runtimeTextures_; // name -> texure

	void initializeFromParent();
	void generateDefines();

public:
	void UploadShaderParameters(Shader* shader, PASS pass);
	void BindShaderTextures(Shader* shader, PASS pass);

public:
	Material(const std::string& id, GenericMaterial *mat); // runtime
	Material(const std::string& path) : path_(path) {} // loading at sturtup. need LoadXML()

	auto DLLEXPORT SaveXML() -> void;
	auto DLLEXPORT LoadXML() -> void;

	auto DLLEXPORT GetId() -> const char * { return id_.c_str(); }
	auto DLLEXPORT GetGenericMaterial() -> GenericMaterial* { return parent_; }

	auto DLLEXPORT SetParamFloat(const char* name, float) -> void;
	auto DLLEXPORT GetParamFloat(const char* name) -> float;
	auto DLLEXPORT SetParamFloat4(const char* name, const vec4& value) -> void;
	auto DLLEXPORT GetParamFloat4(const char* name) -> vec4;
	auto DLLEXPORT SetDef(const char* name, int value) -> void;
	auto DLLEXPORT GetDef(const char* name) -> int;
	auto DLLEXPORT SetTexture(const char *name, const char* path) -> void;
	auto DLLEXPORT GetTexture(const char *name) -> const char*;
	auto DLLEXPORT SetUV(const char *name, const vec4& uv) -> void;
	auto DLLEXPORT GetUV(const char *name) -> vec4;

	// TODO: make one function GetShader(PASS)
	auto DLLEXPORT GetShader(Mesh *mesh = nullptr) -> Shader*;
	auto DLLEXPORT GetDeferredShader(Mesh *mesh = nullptr) -> Shader*;
	auto DLLEXPORT GetIdShader(Mesh *mesh = nullptr) -> Shader*;
	auto DLLEXPORT GetWireframeShader(Mesh *mesh = nullptr) -> Shader*;
	auto DLLEXPORT GetForwardShader(Mesh *mesh = nullptr) -> Shader*;

};
