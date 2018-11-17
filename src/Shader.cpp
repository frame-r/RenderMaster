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

	delete[] vert;
	delete[] geom;
	delete[] frag;
}

API Shader::GetCoreShader(ICoreShader **shaderOut)
{
	*shaderOut = _coreShader;
	return S_OK;
}

API Shader::GetVert(OUT const char ** textOut)
{
	return S_OK;
}

API Shader::GetGeom(OUT const char ** textOut)
{
	return S_OK;
}

API Shader::GetFrag(OUT const char ** textOut)
{
	return S_OK;
}
