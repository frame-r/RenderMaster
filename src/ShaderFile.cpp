#pragma once
#include "Pch.h"
#include "ShaderFile.h"
#include "Core.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

SHARED_ONLY_RESOURCE_IMPLEMENTATION(ShaderFile, _pCore, RemoveSharedShaderFile)

ShaderFile::~ShaderFile()
{
	delete[] text;
}

API ShaderFile::SetText(const char * textIn)
{
	if (text) delete[] text;
	text = textIn;
	return S_OK;
}

API ShaderFile::Reload()
{
	IResourceManager *irm = getResourceManager(_pCore);
	ResourceManager *rm = static_cast<ResourceManager*>(irm);
	rm->ReloadShaderFile(this);

	return S_OK;
}
