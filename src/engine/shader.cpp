#pragma once
#include "pch.h"
#include "shader.h"

Shader::Shader(unique_ptr<ICoreShader> s, unique_ptr<const char> vertIn, unique_ptr<const char> geomIn, unique_ptr<const char> fragIn)
{
	coreShader_ = std::move(s);
	vertFullText_ = std::move(vertIn);
	geomFullText_ = std::move(geomIn);
	fragFullText_ = std::move(fragIn);
}

auto DLLEXPORT Shader::GetCoreShader() -> ICoreShader*
{
	return coreShader_.get();
}

auto DLLEXPORT Shader::GetVert() -> const char *
{
	return vertFullText_.get();
}

auto DLLEXPORT Shader::GetGeom() -> const char *
{
	return geomFullText_.get();
}

auto DLLEXPORT Shader::GetFrag() -> const char *
{
	return fragFullText_.get();
}

auto DLLEXPORT Shader::SetFloatParameter(const char *name, float value) -> void
{
	return coreShader_->SetFloatParameter(name, value);
}

auto DLLEXPORT Shader::SetVec4Parameter(const char *name, const vec4 *value) -> void
{
	return coreShader_->SetVec4Parameter(name, value);
}

auto DLLEXPORT Shader::SetMat4Parameter(const char *name, const mat4 *value) -> void
{
	return coreShader_->SetMat4Parameter(name, value);
}

auto DLLEXPORT Shader::SetUintParameter(const char *name, uint value) -> void
{
	return coreShader_->SetUintParameter(name, value);
}

auto DLLEXPORT Shader::FlushParameters() -> void
{
	return coreShader_->FlushParameters();
}
