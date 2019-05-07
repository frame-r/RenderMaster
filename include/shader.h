#pragma once
#include "common.h"
#include "icorerender.h"

class Shader final
{
	std::unique_ptr<ICoreShader> coreShader_;
	std::unique_ptr<const char> vertFullText_;
	std::unique_ptr<const char> geomFullText_;
	std::unique_ptr<const char> fragFullText_;

public:
	Shader(std::unique_ptr<ICoreShader> s, std::unique_ptr<const char> vertIn, std::unique_ptr<const char> geomIn, std::unique_ptr<const char> fragIn);

	auto DLLEXPORT GetCoreShader() -> ICoreShader*;
	auto DLLEXPORT GetVert() -> const char*;
	auto DLLEXPORT GetGeom() -> const char*;
	auto DLLEXPORT GetFrag() -> const char*;
	auto DLLEXPORT SetFloatParameter(const char* name, float value) -> void;
	auto DLLEXPORT SetVec4Parameter(const char* name, const vec4 *value) -> void;
	auto DLLEXPORT SetMat4Parameter(const char* name, const mat4 *value) -> void;
	auto DLLEXPORT SetUintParameter(const char* name, uint value) -> void;
	auto DLLEXPORT FlushParameters() -> void;
};
