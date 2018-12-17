#include "pch.h"
#include "DX11Texture.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

DX11Texture::DX11Texture(ID3D11Resource *pResourceIn, ID3D11SamplerState *pSampler, ID3D11ShaderResourceView * pShaderViewIn, ID3D11RenderTargetView * pRendertargetView, ID3D11DepthStencilView *pDepthStencilView, TEXTURE_FORMAT formatIn)
 : _resource(pResourceIn), _sampler(pSampler), _shaderView(pShaderViewIn), _renderTargetView(pRendertargetView), _depthStencilView(pDepthStencilView), _format(formatIn)
{
	// TODO make other types!
	ID3D11Texture2D *tex2D = static_cast<ID3D11Texture2D*>(pResourceIn);
	tex2D->GetDesc(&_desc);
	_width = _desc.Width;
	_height = _desc.Height;
}

DX11Texture::~DX11Texture()
{
	if (_resource) { _resource->Release(); _resource = nullptr;	}
	if (_sampler) { _sampler->Release(); _sampler = nullptr;	}
	if (_shaderView) { _shaderView->Release(); _shaderView = nullptr;	}
	if (_renderTargetView) { _renderTargetView->Release(); _renderTargetView = nullptr;	}
	if (_depthStencilView) { _depthStencilView->Release(); _depthStencilView = nullptr;	}
}

