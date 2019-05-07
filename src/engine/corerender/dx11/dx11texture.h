#pragma once
#include "common.h"
#include "icorerender.h"

class DX11Texture : public ICoreTexture
{
	ID3D11Resource *_resource = nullptr;
	ID3D11SamplerState *_sampler = nullptr;
	ID3D11ShaderResourceView *_shaderView = nullptr;
	ID3D11RenderTargetView *_renderTargetView = nullptr;
	ID3D11DepthStencilView *_depthStencilView = nullptr;
	uint _width;
	uint _height;
	uint _mipmaps;
	TEXTURE_FORMAT _format;
	D3D11_TEXTURE2D_DESC _desc;
	size_t bytes;

public:

	DX11Texture(ID3D11Resource* pResourceIn, ID3D11SamplerState *pSmpler, ID3D11ShaderResourceView* pShaderViewIn, ID3D11RenderTargetView *pRendertargetView, ID3D11DepthStencilView *pDepthStencilView, TEXTURE_FORMAT formatIn);
	virtual ~DX11Texture();

	UINT width() {return _width; }
	UINT height() {return _height; }
	TEXTURE_FORMAT format() {return _format; }
	D3D11_TEXTURE2D_DESC desc() { return _desc; }
	ID3D11Resource *resource() { return _resource; }
	ID3D11SamplerState *sampler() { return _sampler; }
	ID3D11ShaderResourceView *srView() { return _shaderView; }
	ID3D11RenderTargetView *rtView() { return _renderTargetView; }
	ID3D11DepthStencilView *dsView() { return _depthStencilView; }

	// ICoreTexture
	auto GetVideoMemoryUsage() -> size_t override;
	auto GetWidth() -> int override { return _width; }
	auto GetHeight() -> int override { return _height; }
	auto GetMipmaps() -> int override { return _mipmaps; }
	auto ReadPixel2D(void *data, int x, int y) -> int override;
};

