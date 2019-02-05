#pragma once
#include "Pch.h"
#include "Core.h"
#include "Shader.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)


Shader::Shader(unique_ptr<ICoreShader> s, unique_ptr<const char> vertIn, unique_ptr<const char> geomIn, unique_ptr<const char> fragIn)
{
	_coreShader = std::move(s);
	_vertFullText = std::move(vertIn);
	_geomFullText = std::move(geomIn);
	_fragFullText = std::move(fragIn);
}

API_RESULT Shader::GetCoreShader(ICoreShader **shaderOut)
{
	*shaderOut = _coreShader.get();
	return S_OK;
}

API_RESULT Shader::GetVert(OUT const char **textOut)
{
	*textOut = _vertFullText.get();
	return S_OK;
}

API_RESULT Shader::GetGeom(OUT const char **textOut)
{
	*textOut = _geomFullText.get();
	return S_OK;
}

API_RESULT Shader::GetFrag(OUT const char **textOut)
{
	*textOut = _fragFullText.get();
	return S_OK;
}

API_RESULT Shader::SetFloatParameter(const char *name, float value)
{
	return _coreShader->SetFloatParameter(name, value);
}

API_RESULT Shader::SetVec4Parameter(const char *name, const vec4 *value)
{
	return _coreShader->SetVec4Parameter(name, value);
}

API_RESULT Shader::SetMat4Parameter(const char *name, const mat4 *value)
{
	return _coreShader->SetMat4Parameter(name, value);
}

API_RESULT Shader::SetUintParameter(const char *name, uint value)
{
	return _coreShader->SetUintParameter(name, value);
}

API_RESULT Shader::FlushParameters()
{
	return _coreShader->FlushParameters();
}
