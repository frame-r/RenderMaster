#pragma once

struct ConstantBuffer
{
	string name;
	size_t bytes = 0u;
	bool needFlush = true;
	WRL::ComPtr<ID3D11Buffer> buffer;
	std::unique_ptr<uint8[]> data;

	struct Parameter
	{
		string name;
		size_t offset = 0u;
		size_t bytes = 0u;
		size_t elements = 1u; // number of elements in array (if parameter is array)
	};
	vector<Parameter> parameters;

public:
	ConstantBuffer(WRL::ComPtr<ID3D11Buffer> dxBufferIn, int bytesIn, const string& nameIn, const vector<Parameter>& paramsIn) :
		buffer(dxBufferIn), bytes(bytesIn), name(nameIn), parameters(paramsIn)
	{
		data = std::make_unique<uint8[]>(bytesIn);
		memset(data.get(), 0, bytesIn);
	}
	ConstantBuffer(const ConstantBuffer& r) = delete;
	ConstantBuffer(ConstantBuffer&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		buffer = r.buffer;
		r.buffer = nullptr;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}
	ConstantBuffer& operator=(ConstantBuffer&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		buffer = r.buffer;
		r.buffer = nullptr;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}
	ConstantBuffer& operator=(const ConstantBuffer& r) = delete;
};

//class DX11RenderTarget : public ICoreRenderTarget
//{
//	TexturePtr _colors[8];
//	TexturePtr _depth;
//
//	void _getColors(ID3D11RenderTargetView **arrayOut, uint& targetsNum);
//	void _getDepth(ID3D11DepthStencilView **depth);
//
//public:
//
//	DX11RenderTarget(){}
//	virtual ~DX11RenderTarget();
//
//	void bind(ID3D11DeviceContext *ctx, ID3D11DepthStencilView *standardDepthBuffer);
//	void clear(ID3D11DeviceContext *ctx, FLOAT* color, FLOAT Depth, UINT8 stencil);
//	ITexture *texColor(uint slot) { return _colors[slot].Get(); }
//	ITexture *texDepth() { return _depth.Get(); }
//
//	API_RESULT SetColorTexture(uint slot, ITexture *tex) override;
//	API_RESULT SetDepthTexture(ITexture *tex) override;
//	API_RESULT UnbindColorTexture(uint slot) override;
//	API_RESULT UnbindAll() override;
//};
