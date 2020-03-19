
//
// We define hash-functions and equality operators for: 
// D3D11_RASTERIZER_DESC
// D3D11_DEPTH_STENCIL_DESC
// D3D11_BLEND_DESC
//
// Because we store states in unordered_map "Description" -> "State ptr".
// For example:
// D3D11_RASTERIZER_DESC -> ID3D11RasterizerState*

#define DEFAULT_DEPTH_FUNC D3D11_COMPARISON_LESS_EQUAL

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
			cd.be |= (uint64_t)desc.RenderTarget[i].BlendEnable << i;
			cd.mask += desc.RenderTarget[i].RenderTargetWriteMask;
		}
		return cd.u64;
	}

	bool operator()(const D3D11_BLEND_DESC& a, const D3D11_BLEND_DESC& b) const
	{
		return 0 == memcmp(&a, &b, sizeof(a));
	}
};
template <typename TDesc, typename TState, typename THashStruct>
class BaseStatePool
{
protected:
	std::unordered_map<TDesc, WRL::ComPtr<TState>, THashStruct, THashStruct> _pool;
	DX11CoreRender &_parent;

protected:
	void free() 
	{ 
		//for (auto& s : _pool)
		//{
		//	s.second->Release();
		//}
		_pool.clear(); 
	}
	virtual bool create_actually_state(const TDesc *pRasterizerDesc, TState **ppRasterizerState) = 0;

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
	virtual bool create_actually_state(const D3D11_RASTERIZER_DESC *pRasterizerDesc, _COM_Outptr_opt_  ID3D11RasterizerState **ppRasterizerState) override
	{
		return SUCCEEDED(_parent._device->CreateRasterizerState(pRasterizerDesc, ppRasterizerState));
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
	virtual bool create_actually_state(const D3D11_DEPTH_STENCIL_DESC *desc, ID3D11DepthStencilState **ppState) override
	{
		return SUCCEEDED(_parent._device->CreateDepthStencilState(desc, ppState));
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
		dsDesc.DepthFunc = DEFAULT_DEPTH_FUNC;

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
	virtual bool create_actually_state(const D3D11_BLEND_DESC *desc, ID3D11BlendState **ppState) override
	{
		return SUCCEEDED(_parent._device->CreateBlendState(desc, ppState));
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
