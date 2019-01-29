#pragma once
#include "Pch.h"
#include "Core.h"
#include "Texture.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

BASE_RESOURCE_IMPLEMENTATION(Texture, _pCore, RemoveRuntimeTexture, RemoveSharedTexture)

API Texture::GetCoreTexture(ICoreTexture **texOut)
{
	*texOut = _coreTexture.get();
	return S_OK;
}

Texture::Texture(unique_ptr<ICoreTexture> tex)
{
	_coreTexture = std::move(tex);
}

Texture::Texture(unique_ptr<ICoreTexture> tex, const string& filePath)
{
	_coreTexture = std::move(tex);
	_file = filePath;
}

API Texture::GetWidth(OUT uint * w)
{
	_coreTexture->GetWidth(w);
	return S_OK;
}

API Texture::GetHeight(OUT uint * h)
{
	_coreTexture->GetHeight(h);
	return S_OK;
}

API Texture::GetFormat(OUT TEXTURE_FORMAT * formatOut)
{
	return S_OK;
}
