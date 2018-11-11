#pragma once
#include "Common.h"

namespace WRL = Microsoft::WRL;

class DX11ConstantBuffer final: public IUniformBuffer
{
	WRL::ComPtr<ID3D11Buffer> buffer;

public:

	DX11ConstantBuffer(ID3D11Buffer *bufferIn) : buffer(bufferIn) {}
	virtual ~DX11ConstantBuffer() { Free(); }

	API Free();

	ID3D11Buffer *nativeBuffer() const { return buffer.Get(); }
};


//
// We define hash-functions and equality operators for: 
// D3D11_RASTERIZER_DESC
// D3D11_DEPTH_STENCIL_DESC
// D3D11_BLEND_DESC
//
// Because we store states in unordered_map "Description" -> "State ptr".
// For example:
// D3D11_RASTERIZER_DESC -> ID3D11RasterizerState*

struct RasterHash
{
	union CompactDesc
	{
		uint64_t u64;
		struct
		{
			unsigned int fillmode : 2;
			unsigned int cullmode : 2;
			unsigned int frontccw : 1;
			unsigned int depthclip : 1;
			unsigned int scissor : 1;
			unsigned int msaa : 1;
			unsigned int antialias : 1;
			unsigned int depthbias : 23;
			float        misc;
		};
	};

	uint64_t operator()(const D3D11_RASTERIZER_DESC & desc) const
	{
		CompactDesc cd;

		cd.fillmode = desc.FillMode;
		cd.cullmode = desc.CullMode;
		cd.frontccw = desc.FrontCounterClockwise;
		cd.depthclip = desc.DepthClipEnable;
		cd.scissor = desc.ScissorEnable;
		cd.msaa = desc.MultisampleEnable;
		cd.antialias = desc.AntialiasedLineEnable;
		cd.depthbias = desc.DepthBias;
		cd.misc = desc.DepthBiasClamp + desc.SlopeScaledDepthBias;

		return cd.u64;
	}

	bool operator()(const D3D11_RASTERIZER_DESC& a, const D3D11_RASTERIZER_DESC& b) const
	{
		return 0 == ::memcmp(&a, &b, sizeof(a));
	}
};

struct DepthStencilHash
{
	union CompactDesc
	{
		uint64_t u64;
		struct
		{
			unsigned int depth : 1;
			unsigned int write : 1;
			unsigned int zfunc : 3;
			unsigned int stencil : 1;
			unsigned int ff_fail : 3;
			unsigned int ff_zfail : 3;
			unsigned int ff_pass : 3;
			unsigned int ff_func : 3;
			unsigned int bf_fail : 3;
			unsigned int bf_zfail : 3;
			unsigned int bf_pass : 3;
			unsigned int bf_func : 3;
			unsigned int nouse : 2;
			unsigned int srmask : 16;
			unsigned int swmask : 16;
		};
	};

	uint64_t operator()(const D3D11_DEPTH_STENCIL_DESC & desc) const
	{
		CompactDesc cd;
		cd.depth = desc.DepthEnable;
		cd.write = desc.DepthWriteMask;
		cd.zfunc = desc.DepthFunc - 1;
		cd.stencil = desc.StencilEnable;
		cd.ff_fail = desc.FrontFace.StencilFailOp - 1;
		cd.ff_zfail = desc.FrontFace.StencilDepthFailOp - 1;
		cd.ff_pass = desc.FrontFace.StencilPassOp - 1;
		cd.ff_func = desc.FrontFace.StencilFunc - 1;
		cd.bf_fail = desc.BackFace.StencilFailOp - 1;
		cd.bf_zfail = desc.BackFace.StencilDepthFailOp - 1;
		cd.bf_pass = desc.BackFace.StencilPassOp - 1;
		cd.bf_func = desc.BackFace.StencilFunc - 1;
		cd.nouse = 0;
		cd.srmask = desc.StencilReadMask;
		cd.swmask = desc.StencilWriteMask;
		return cd.u64;
	}

	bool operator()(const D3D11_DEPTH_STENCIL_DESC& a, const D3D11_DEPTH_STENCIL_DESC& b) const
	{
		return 0 == memcmp(&a, &b, sizeof(a));
	}
};

struct BlendHash
{
	union CompactDesc
	{
		uint64_t u64;
		struct
		{
			uint64_t a2c : 1;
			uint64_t ibe : 1;
			uint64_t be : 8;
			uint64_t sb : 5;
			uint64_t db : 5;
			uint64_t bo : 3;
			uint64_t sba : 5;
			uint64_t dba : 5;
			uint64_t boa : 3;
			uint64_t mask : 28;
		};
	};

	uint64_t operator()(const D3D11_BLEND_DESC& desc) const
	{
		CompactDesc cd;
		cd.a2c = desc.AlphaToCoverageEnable;
		cd.ibe = desc.IndependentBlendEnable;
		cd.be = desc.RenderTarget[0].BlendEnable;
		cd.sb = desc.RenderTarget[0].SrcBlend;
		cd.db = desc.RenderTarget[0].DestBlend;
		cd.bo = desc.RenderTarget[0].BlendOp;
		cd.sba = desc.RenderTarget[0].SrcBlendAlpha;
		cd.dba = desc.RenderTarget[0].DestBlendAlpha;
		cd.boa = desc.RenderTarget[0].BlendOpAlpha;
		cd.mask = desc.RenderTarget[0].RenderTargetWriteMask;
		for (int i = 1; i < 8; ++i)
		{
			cd.be |= desc.RenderTarget[i].BlendEnable << i;
			cd.mask += desc.RenderTarget[i].RenderTargetWriteMask;
		}
		return cd.u64;
	}

	bool operator()(const D3D11_BLEND_DESC& a, const D3D11_BLEND_DESC& b) const
	{
		return 0 == memcmp(&a, &b, sizeof(a));
	}
};


class DX11CoreRender final : public ICoreRender
{
	WRL::ComPtr<ID3D11Device> _device;
	WRL::ComPtr<ID3D11DeviceContext> _context;

	WRL::ComPtr<IDXGISwapChain> _swapChain; // TODO: make map HWND -> {IDXGISwapChain, ID3D11RenderTargetView} for support multiple windows

	WRL::ComPtr<ID3D11Texture2D> _renderTargetTex;
	WRL::ComPtr<ID3D11RenderTargetView> _renderTargetView;
	WRL::ComPtr<ID3D11Texture2D> _depthStencilTex;
	WRL::ComPtr<ID3D11DepthStencilView> _depthStencilView;

	vector<std::function<void()>> _onCleanBroadcast;

	template <typename TDesc, typename TState, typename THashStruct>
	class BaseStatePool
	{
	protected:
		std::unordered_map<TDesc, WRL::ComPtr<TState>, THashStruct, THashStruct> _pool;
		DX11CoreRender &_parent;

	protected:
		void free() { _pool.clear(); }
		virtual API create_actually_state(const TDesc *pRasterizerDesc, _COM_Outptr_opt_  TState **ppRasterizerState) = 0;

	public:
		BaseStatePool(DX11CoreRender& parent) : _parent(parent)
		{
			parent._onCleanBroadcast.push_back(std::bind(&BaseStatePool::free, this));
		}

		WRL::ComPtr<TState> FetchState(TDesc& state)
		{
			auto it = _pool.find(state);

			if (it != _pool.end())
				return it->second;
			
			WRL::ComPtr<TState> ret;

			if (FAILED(create_actually_state(&state, ret.GetAddressOf())))
				return WRL::ComPtr<TState>();

			_pool[state] = ret;

			return ret;
		}
	};

	class RasterizerStatePool : public BaseStatePool<D3D11_RASTERIZER_DESC, ID3D11RasterizerState, RasterHash>
	{
		virtual API create_actually_state(const D3D11_RASTERIZER_DESC *pRasterizerDesc, _COM_Outptr_opt_  ID3D11RasterizerState **ppRasterizerState) override
		{
			return _parent._device->CreateRasterizerState(pRasterizerDesc, ppRasterizerState);
		}

	public:
		RasterizerStatePool(DX11CoreRender& parent) : BaseStatePool(parent) {}

		WRL::ComPtr<ID3D11RasterizerState> FetchDefaultState()
		{
			WRL::ComPtr<ID3D11RasterizerState> ret;

			D3D11_RASTERIZER_DESC rasterDesc;
			rasterDesc.AntialiasedLineEnable = false;
			rasterDesc.CullMode = D3D11_CULL_NONE;
			rasterDesc.DepthBias = 0;
			rasterDesc.DepthBiasClamp = 0.0f;
			rasterDesc.DepthClipEnable = true;
			rasterDesc.FillMode = D3D11_FILL_SOLID;
			rasterDesc.FrontCounterClockwise = false;
			rasterDesc.MultisampleEnable = false;
			rasterDesc.ScissorEnable = false;
			rasterDesc.SlopeScaledDepthBias = 0.0f;

			if (FAILED(_parent._device->CreateRasterizerState(&rasterDesc, ret.GetAddressOf())))
				return WRL::ComPtr<ID3D11RasterizerState>();

			_pool[rasterDesc] = ret;

			return ret;
		}
	};

	class DepthStencilStatePool : public BaseStatePool<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState, DepthStencilHash>
	{
		virtual API create_actually_state(const D3D11_DEPTH_STENCIL_DESC *desc, _COM_Outptr_opt_  ID3D11DepthStencilState **ppState) override
		{
			return _parent._device->CreateDepthStencilState(desc, ppState);
		}

	public:
		DepthStencilStatePool(DX11CoreRender& parent) : BaseStatePool(parent) {}

		WRL::ComPtr<ID3D11DepthStencilState> FetchDefaultState()
		{
			WRL::ComPtr<ID3D11DepthStencilState> ret;

			D3D11_DEPTH_STENCIL_DESC dsDesc;

			// Depth test parameters
			dsDesc.DepthEnable = true;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

			// Stencil test parameters
			dsDesc.StencilEnable = false;
			dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

			// Stencil operations if pixel is front-facing
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			// Stencil operations if pixel is back-facing
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			if (FAILED(_parent._device->CreateDepthStencilState(&dsDesc, ret.GetAddressOf())))
				return WRL::ComPtr<ID3D11DepthStencilState>();

			_pool[dsDesc] = ret;

			return ret;
		}
	};

	class BlendStatePool : public BaseStatePool<D3D11_BLEND_DESC, ID3D11BlendState, BlendHash>
	{
		virtual API create_actually_state(const D3D11_BLEND_DESC *desc, _COM_Outptr_opt_  ID3D11BlendState **ppState) override
		{
			return _parent._device->CreateBlendState(desc, ppState);
		}

	public:
		BlendStatePool(DX11CoreRender& parent) : BaseStatePool(parent) {}

		WRL::ComPtr<ID3D11BlendState> FetchDefaultState()
		{
			WRL::ComPtr<ID3D11BlendState> ret;

			D3D11_BLEND_DESC dsDesc;
			dsDesc.AlphaToCoverageEnable = FALSE;
			dsDesc.IndependentBlendEnable = FALSE;

			D3D11_RENDER_TARGET_BLEND_DESC rtDesc;
			rtDesc.BlendEnable = FALSE;
			rtDesc.SrcBlend = D3D11_BLEND_ONE;
			rtDesc.DestBlend = D3D11_BLEND_ZERO;
			rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
			rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
			rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
			rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			rtDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			for (int i = 0; i < 8; ++i)
				dsDesc.RenderTarget[i] = rtDesc;

			if (FAILED(_parent._device->CreateBlendState(&dsDesc, ret.GetAddressOf())))
				return WRL::ComPtr<ID3D11BlendState>();

			_pool[dsDesc] = ret;

			return ret;
		}
	};

	struct State
	{
		D3D11_RASTERIZER_DESC rasterState;
		D3D11_DEPTH_STENCIL_DESC depthState;
		D3D11_BLEND_DESC blendState;

		// TODO: shader, textures, render target
	};

	RasterizerStatePool _rasterizerStatePool{*this};
	DepthStencilStatePool _depthStencilStatePool{*this};
	BlendStatePool _blendStatePool{*this};

	State _currentState;
	std::stack<State> _statesStack;

	IResourceManager *_pResMan{nullptr};

	int _MSAASamples = 0;

	ID3D11DeviceChild* create_shader_by_src(int type, const char *src);
	bool create_viewport_buffers(uint w, uint h);
	void destroy_viewport_buffers();
	UINT msaa_quality(DXGI_FORMAT format, int MSAASamples);
	
public:

	DX11CoreRender();
	virtual ~DX11CoreRender();

	API Init(const WindowHandle* handle, int MSAASamples = 0) override;
	API Free() override;
	API MakeCurrent(const WindowHandle* handle) override;
	API SwapBuffers() override;

	API PushStates() override;
	API PopStates() override;

	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const ShaderText *shaderDesc) override;
	API CreateUniformBuffer(OUT IUniformBuffer **pBuffer, uint size) override;
	API CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) override;
	API SetShader(const ICoreShader *pShader) override;
	API SetMesh(const ICoreMesh* mesh) override;
	API SetUniformBuffer(const IUniformBuffer *pBuffer, uint slot) override;
	API SetUniformBufferData(IUniformBuffer *pBuffer, const void *pData) override;
	API Draw(ICoreMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API SetViewport(uint w, uint h) override;
	API GetViewport(OUT uint* w, OUT uint* h) override;
	API Clear() override;
	
	API GetName(OUT const char **pTxt) override;
};

