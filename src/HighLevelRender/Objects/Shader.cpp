#pragma once
#include "Pch.h"
#include "Core.h"
#include "Shader.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

RUNTIME_ONLY_RESOURCE_IMPLEMENTATION(Shader, _pCore, RemoveRuntimeShader)

Shader::~Shader()
{
	delete _coreShader;
	_coreShader = nullptr;

	delete[] vertText;
	delete[] geomText;
	delete[] fragText;
}

API Shader::GetCoreShader(ICoreShader **shaderOut)
{
	*shaderOut = _coreShader;
	return S_OK;
}

API Shader::GetVert(OUT const char **textOut)
{
	*textOut = vertText;
	return S_OK;
}

API Shader::GetGeom(OUT const char **textOut)
{
	*textOut = geomText;
	return S_OK;
}

API Shader::GetFrag(OUT const char **textOut)
{
	*textOut = fragText;
	return S_OK;
}

API Shader::SetFloatParameter(const char *name, float value)
{
	return _coreShader->SetFloatParameter(name, value);
}

API Shader::SetVec4Parameter(const char *name, const vec4 *value)
{
	return _coreShader->SetVec4Parameter(name, value);
}

API Shader::SetMat4Parameter(const char *name, const mat4 *value)
{
	return _coreShader->SetMat4Parameter(name, value);
}

API Shader::SetUintParameter(const char *name, uint value)
{
	return _coreShader->SetUintParameter(name, value);
}

API Shader::FlushParameters()
{
	return _coreShader->FlushParameters();
}
