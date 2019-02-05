#pragma once
#include "Common.h"

class DX11Texture : public ICoreTexture
{
	ID3D11Resource *_resource = nullptr;
	ID3D11SamplerState *_sampler = nullptr;
	ID3D11ShaderResourceView *_shaderView = nullptr;
	ID3D11RenderTargetView *_renderTargetView = nullptr;
	ID3D11DepthStencilView *_depthStencilView = nullptr;
	uint _width;
	uint _height;
	TEXTURE_FORMAT _format;
	D3D11_TEXTURE2D_DESC _desc;

public:

	DX11Texture(ID3D11Resource* pResourceIn, ID3D11SamplerState *pSmpler, ID3D11ShaderResourceView* pShaderViewIn, ID3D11RenderTargetView *pRendertargetView, ID3D11DepthStencilView *pDepthStencilView, TEXTURE_FORMAT formatIn);
	virtual ~DX11Texture();

	API_RESULT GetWidth(OUT uint *w) override { *w = _width; return S_OK; }
	API_RESULT GetHeight(OUT uint *h) override { *h = _height; return S_OK; }
	API_RESULT GetFormat(OUT TEXTURE_FORMAT *formatOut) override { *formatOut = _format; return S_OK; }

	UINT width() {return _width; }
	UINT height() {return _height; }
	TEXTURE_FORMAT format() {return _format; }
	D3D11_TEXTURE2D_DESC desc() { return _desc; }
	ID3D11Resource *resource() { return _resource; }
	ID3D11SamplerState *sampler() { return _sampler; }
	ID3D11ShaderResourceView *srView() { return _shaderView; }
	ID3D11RenderTargetView *rtView() { return _renderTargetView; }
	ID3D11DepthStencilView *dsView() { return _depthStencilView; }
};

