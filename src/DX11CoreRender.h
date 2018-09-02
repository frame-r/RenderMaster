#pragma once
#include "Common.h"
#include <wrl/module.h>
#include <d3d11_2.h>

namespace WRL = Microsoft::WRL;


class DX11ConstantBuffer final: public IUniformBuffer
{
	WRL::ComPtr<ID3D11Buffer> buffer;

public:

	DX11ConstantBuffer(ID3D11Buffer *bufferIn) : buffer(bufferIn) {}

	ID3D11Buffer *nativeBuffer() const { return buffer.Get(); }

	API Free() override;
	API GetType(OUT RES_TYPE *type) override;
};

inline bool operator <(const D3D11_RASTERIZER_DESC& l, const D3D11_RASTERIZER_DESC& r)
{
	return std::tie(l.AntialiasedLineEnable, l.CullMode, l.DepthBias, l.DepthBiasClamp, l.DepthClipEnable, l.FillMode, l.FrontCounterClockwise, l.MultisampleEnable, l.ScissorEnable, l.SlopeScaledDepthBias) <
		std::tie(r.AntialiasedLineEnable, r.CullMode, r.DepthBias, r.DepthBiasClamp, r.DepthClipEnable, r.FillMode, r.FrontCounterClockwise, r.MultisampleEnable, r.ScissorEnable, r.SlopeScaledDepthBias);
}

inline bool operator <(const D3D11_DEPTH_STENCILOP_DESC& l, const D3D11_DEPTH_STENCILOP_DESC& r)
{
	return std::tie(l.StencilDepthFailOp, l.StencilFailOp, l.StencilFunc, l.StencilPassOp) <
		std::tie(r.StencilDepthFailOp, r.StencilFailOp, r.StencilFunc, r.StencilPassOp);
}

inline bool operator <(const D3D11_DEPTH_STENCIL_DESC& l, const D3D11_DEPTH_STENCIL_DESC& r)
{
	return std::tie(l.DepthEnable, l.BackFace, l.DepthFunc, l.DepthWriteMask, l.FrontFace, l.StencilEnable, l.StencilReadMask, l.StencilWriteMask) <
		std::tie(r.DepthEnable, r.BackFace, r.DepthFunc, r.DepthWriteMask, r.FrontFace, r.StencilEnable, r.StencilReadMask, r.StencilWriteMask);
}


class DX11CoreRender final : public ICoreRender
{
	WRL::ComPtr<ID3D11DeviceContext> _context;
	WRL::ComPtr<ID3D11Device> _device;

	// TODO: make map HWND -> {IDXGISwapChain, ID3D11RenderTargetView} for support multiple windows
	WRL::ComPtr<IDXGISwapChain> _swapChain;

	WRL::ComPtr<ID3D11Texture2D> _renderTargetTex;
	WRL::ComPtr<ID3D11RenderTargetView> _renderTargetView;

	WRL::ComPtr<ID3D11Texture2D> _depthStencilTex;
	WRL::ComPtr<ID3D11DepthStencilView> _depthStencilView;

	std::vector<std::function<void()>> _onCleanBroadcast;

	template <typename TDesc, typename TState>
	class BaseStatePool
	{
	protected:
		std::map<TDesc, WRL::ComPtr<TState>> _pool;
		DX11CoreRender &_parent;

		virtual API CreateDXState(const TDesc *pRasterizerDesc, _COM_Outptr_opt_  TState **ppRasterizerState) = 0;

		template <typename T, T TDesc::*Memeber>
		WRL::ComPtr<TState> _getModifedState(TDesc& state, T& value)
		{
			state.*Memeber = value;

			auto it = _pool.find(state);

			if (it != _pool.end())
				return it->second;
			
			WRL::ComPtr<TState> ret;

			if (FAILED(CreateDXState(&state, ret.GetAddressOf())))
				return WRL::ComPtr<TState>();

			_pool[state] = ret;

			return ret;
		}

		void _free() { _pool.clear(); }

	public:
		BaseStatePool(DX11CoreRender& parent) : _parent(parent)
		{
			parent._onCleanBroadcast.push_back(std::bind(&BaseStatePool::_free, this));
		}
	};


	class RasterizerStatePool : public BaseStatePool<D3D11_RASTERIZER_DESC, ID3D11RasterizerState>
	{
	public:
		RasterizerStatePool(DX11CoreRender& parent) : BaseStatePool(parent) {}

		virtual API CreateDXState(const D3D11_RASTERIZER_DESC *pRasterizerDesc, _COM_Outptr_opt_  ID3D11RasterizerState **ppRasterizerState) override
		{
			return _parent._device->CreateRasterizerState(pRasterizerDesc, ppRasterizerState);
		}

		WRL::ComPtr<ID3D11RasterizerState> GetDefaultState()
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

			auto result = _parent._device->CreateRasterizerState(&rasterDesc, ret.GetAddressOf());
			if (FAILED(result))
				return WRL::ComPtr<ID3D11RasterizerState>();

			_pool[rasterDesc] = ret;

			return ret;
		}

		WRL::ComPtr<ID3D11RasterizerState> ModifyCullMode(D3D11_CULL_MODE &cullMode)
		{
			return _getModifedState<D3D11_CULL_MODE, &D3D11_RASTERIZER_DESC::CullMode>(_parent._currentState.rasterState, cullMode);
		}

		WRL::ComPtr<ID3D11RasterizerState> ModifyFillMode(D3D11_FILL_MODE &fillMode)
		{
			return _getModifedState<D3D11_FILL_MODE, &D3D11_RASTERIZER_DESC::FillMode>(_parent._currentState.rasterState, fillMode);
		}

	}_rasterizerStatePool{*this};

	class DepthStencilStatePool : public BaseStatePool<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState>
	{
	public:
		DepthStencilStatePool(DX11CoreRender& parent) : BaseStatePool(parent) {}

		virtual API CreateDXState(const D3D11_DEPTH_STENCIL_DESC *desc, _COM_Outptr_opt_  ID3D11DepthStencilState **ppState) override
		{
			return _parent._device->CreateDepthStencilState(desc, ppState);
		}

		WRL::ComPtr<ID3D11DepthStencilState> GetDefaultState()
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

			auto result = _parent._device->CreateDepthStencilState(&dsDesc, ret.GetAddressOf());
			if (FAILED(result))
				return WRL::ComPtr<ID3D11DepthStencilState>();

			_pool[dsDesc] = ret;

			return ret;
		}

		WRL::ComPtr<ID3D11DepthStencilState> ModifyDepthState(BOOL &enabled)
		{
			return _getModifedState<BOOL, &D3D11_DEPTH_STENCIL_DESC::DepthEnable>(_parent._currentState.depthState, enabled);
		}


	}_depthStencilStatePool{*this};


	struct State
	{
		D3D11_RASTERIZER_DESC rasterState;
		D3D11_DEPTH_STENCIL_DESC depthState;
		//D3D11_BLEND_DESC blendState;
	};

	State _currentState;
	std::stack<State> _stateStack;

	enum
	{
		TYPE_VERTEX,
		TYPE_GEOMETRY,
		TYPE_FRAGMENT,
	};

	bool create_viewport_buffers(uint w, uint h);
	void destroy_viewport_buffers();
	
	ID3D11DeviceChild* create_shader_by_src(int type, const char *src);
	const char* get_shader_profile(int type);
	const char* get_main_function(int type);

	IResourceManager *_pResMan{nullptr};

public:

	DX11CoreRender();
	virtual ~DX11CoreRender();
	
	API Init(const WinHandle* handle) override;
	API PushStates() override;
	API PopStates() override;
	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const ShaderText *shaderDesc) override;
	API SetShader(const ICoreShader *pShader) override;
	API CreateUniformBuffer(OUT IUniformBuffer **pBuffer, uint size) override;
	API SetUniform(IUniformBuffer *pBuffer, const void *pData) override;
	API SetUniformBufferToShader(IUniformBuffer *pBuffer, uint slot) override;
	API SetUniform(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type) override;
	API SetUniformArray(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type, uint number) override;
	API SetMesh(const ICoreMesh* mesh) override;
	API Draw(ICoreMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API MakeCurrent(const WinHandle* handle) override;
	API SetViewport(uint w, uint h) override;
	API GetViewport(OUT uint* w, OUT uint* h) override;
	API Clear() override;
	API SwapBuffers() override;
	API Free() override;
	API GetName(OUT const char **pTxt) override;
};

