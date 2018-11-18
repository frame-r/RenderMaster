#pragma once
#include "Pch.h"
#include "ShaderText.h"
#include "Core.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

SHARED_ONLY_RESOURCE_IMPLEMENTATION(ShaderText, _pCore, RemoveSharedShaderText)

ShaderText::~ShaderText()
{
	delete[] text;
}

API ShaderText::SetText(const char * textIn)
{
	if (text) delete[] text;
	text = textIn;
	return S_OK;
}

API ShaderText::Reload()
{
	IResourceManager *irm = getResourceManager(_pCore);
	ResourceManager *rm = static_cast<ResourceManager*>(irm);
	rm->ReloadShaderText(this);

	return S_OK;
}
