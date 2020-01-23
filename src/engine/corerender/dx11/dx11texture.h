#pragma once
#include "common.h"
#include "icorerender.h"

class DX11Texture : public ICoreTexture
{
	ID3D11Resource *_resource = nullptr;
	ID3D11SamplerState *_sampler = nullptr;
	ID3D11ShaderResourceView *_shaderView = nullptr;
	ID3D11RenderTargetView *_renderTargetView{};
	ID3D11DepthStencilView *_depthStencilView = nullptr;
	ID3D11UnorderedAccessView *_unorderedAccessView = nullptr;
	uint _width;
	uint _height;
	uint _mipmaps;
	TEXTURE_FORMAT _format;
	D3D11_TEXTURE2D_DESC _desc;
	size_t bytes;
	TEXTURE_CREATE_FLAGS flags;
	TEXTURE_TYPE type;

	size_t getData(uint8_t* pDataOut, size_t length);

public:

	DX11Texture(ID3D11Resource* pResourceIn, ID3D11SamplerState *pSmpler,
				ID3D11ShaderResourceView* pShaderViewIn, ID3D11RenderTargetView *rtv, ID3D11DepthStencilView *dsv,
				ID3D11UnorderedAccessView *uav, TEXTURE_FORMAT formatIn, TEXTURE_CREATE_FLAGS flags_,
				TEXTURE_TYPE type_);
	virtual ~DX11Texture();

	UINT width() {return _width; }
	UINT height() {return _height; }
	TEXTURE_FORMAT format() {return _format; }
	D3D11_TEXTURE2D_DESC desc() { return _desc; }
	ID3D11Resource *resource() { return _resource; }
	ID3D11SamplerState *sampler() { return _sampler; }
	ID3D11ShaderResourceView *srView() { return _shaderView; }
	ID3D11RenderTargetView* rtView();
	ID3D11DepthStencilView* dsView();
	ID3D11UnorderedAccessView *uavView() { return _unorderedAccessView; }

	// ICoreTexture
	auto GetVideoMemoryUsage() -> size_t override;
	auto GetWidth() -> int override { return _width; }
	auto GetHeight() -> int override { return _height; }
	auto GetMipmaps() -> int override { return _mipmaps; }
	auto ReadPixel2D(void *data, int x, int y) -> int override;
	auto GetData(uint8_t* pDataOut, size_t length) -> void override;
	auto CreateMipmaps() -> void override;
};

