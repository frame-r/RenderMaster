#pragma once
#include "Common.h"

namespace WRL = Microsoft::WRL;

class DX11ConstantBuffer final: public ICoreConstantBuffer
{
	WRL::ComPtr<ID3D11Buffer> buffer;

public:

	DX11ConstantBuffer(ID3D11Buffer *bufferIn) : buffer(bufferIn) {}
	virtual ~DX11ConstantBuffer(); 

	ID3D11Buffer *nativeBuffer() const { return buffer.Get(); }
};


class DX11RenderTarget : public ICoreRenderTarget
{
	WRL::ComPtr<ITexture> _colors[8];
	WRL::ComPtr<ITexture> _depth;

	void _get_colors(ID3D11RenderTargetView **arrayOut, uint& targetsNum);
	void _get_depth(ID3D11DepthStencilView **depth);

public:

	DX11RenderTarget() {}
	virtual ~DX11RenderTarget();

	void bind(ID3D11DeviceContext *ctx);
	void clear(ID3D11DeviceContext *ctx, FLOAT* color, FLOAT Depth, UINT8 stencil);

	API SetColorTexture(uint slot, ITexture *tex) override;
	API SetDepthTexture(ITexture *tex) override;
	API UnbindColorTexture(uint slot) override;
	API UnbindAll() override;
};


class DX11CoreRender final : public ICoreRender
{
	WRL::ComPtr<ID3D11Device> _device;
	WRL::ComPtr<ID3D11DeviceContext> _context;

	WRL::ComPtr<IDXGISwapChain> _swapChain; // TODO: make map HWND -> {IDXGISwapChain, ID3D11RenderTargetView} for support multiple windows

	WRL::ComPtr<ID3D11Texture2D> _defaultRenderTargetTex;
	WRL::ComPtr<ID3D11RenderTargetView> _defaultRenderTargetView;
	WRL::ComPtr<ID3D11Texture2D> _defaultDepthStencilTex;
	WRL::ComPtr<ID3D11DepthStencilView> _defaultDepthStencilView;

	vector<std::function<void()>> _onCleanBroadcast;

	#include "states_pools.inl"
	RasterizerStatePool _rasterizerStatePool{*this};
	DepthStencilStatePool _depthStencilStatePool{*this};
	BlendStatePool _blendStatePool{*this};

	struct State
	{
		// Rasterizer
		//
		D3D11_RASTERIZER_DESC rasterState;

		// Depth/Stencil
		//
		D3D11_DEPTH_STENCIL_DESC depthStencilState;

		// Viewport
		// TODO

		// Blending
		//
		D3D11_BLEND_DESC blendState;

		// Shader
		// TODO

		// Textures
		// TODO

		// Framebuffer
		// TODO

		// Clear
		//
		FLOAT clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		FLOAT depth_clear_color = 1.0f;
		UINT8 stencil_clear_color = 0;
	};

	State _state;
	std::stack<State> _statesStack;

	IResourceManager *_pResMan = nullptr;

	int _MSAASamples = 0;
	int _VSyncOn = 1;

	int is_default_rt_bound = 1;
	DX11RenderTarget *current_render_target = nullptr;

	bool create_default_buffers(uint w, uint h);
	void destroy_default_buffers();

	ID3D11DeviceChild* create_shader_by_src(int type, const char *src, HRESULT& err);
	UINT msaa_quality(DXGI_FORMAT format, int MSAASamples);
	
public:

	DX11CoreRender();
	virtual ~DX11CoreRender();

	API Init(const WindowHandle* handle, int MSAASamples, int VSyncOn) override;
	API Free() override;
	API MakeCurrent(const WindowHandle* handle) override;
	API SwapBuffers() override;

	API PushStates() override;
	API PopStates() override;

	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const char *vertText, const char *fragText, const char *geomText) override;
	API CreateConstantBuffer(OUT ICoreConstantBuffer **pBuffer, uint size) override;
	API CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) override;
	API CreateRenderTarget(OUT ICoreRenderTarget **pRenderTarget) override;

	API SetCurrentRenderTarget(ICoreRenderTarget *pRenderTarget) override;
	API RestoreDefaultRenderTarget() override;
	API ReadPixel2D(ICoreTexture *tex, OUT void *out, OUT uint* readPixelBytes, uint x, uint y) override;
	API SetShader(const ICoreShader *pShader) override;
	API SetMesh(const ICoreMesh* mesh) override;
	API SetConstantBuffer(const ICoreConstantBuffer *pBuffer, uint slot) override;
	API SetConstantBufferData(ICoreConstantBuffer *pBuffer, const void *pData) override;
	API Draw(ICoreMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API SetViewport(uint w, uint h) override;
	API GetViewport(OUT uint* w, OUT uint* h) override;
	API Clear() override;
	
	API GetName(OUT const char **pNameOut) override;
};

