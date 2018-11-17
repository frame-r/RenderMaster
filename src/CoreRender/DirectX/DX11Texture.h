#pragma once
#include "Common.h"

class DX11Texture : public ICoreTexture
{
	ID3D11Resource *_resource = nullptr;
	ID3D11ShaderResourceView *_shaderView = nullptr;
	ID3D11RenderTargetView *_renderTargetView = nullptr;

public:

	DX11Texture(ID3D11Resource* pResourceIn, ID3D11ShaderResourceView* pShaderViewIn, ID3D11RenderTargetView *pRendertargetView) : 
		_resource(pResourceIn), _shaderView(pShaderViewIn), _renderTargetView(pRendertargetView) {}
	virtual ~DX11Texture();
};

