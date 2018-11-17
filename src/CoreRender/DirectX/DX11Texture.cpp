#include "pch.h"
#include "DX11Texture.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

DX11Texture::~DX11Texture()
{
	if (_resource) { _resource->Release(); _resource = nullptr;	}
	if (_shaderView) { _shaderView->Release(); _shaderView = nullptr;	}
	if (_renderTargetView) { _renderTargetView->Release(); _renderTargetView = nullptr;	}
}

