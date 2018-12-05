#pragma once
#include "Common.h"

namespace WRL = Microsoft::WRL;

struct ConstantBuffer
{
	string name;
	uint bytes = 0;	

	struct ConstantBufferParameter
	{
		string name;
		uint offset = 0;
		uint bytes = 0;
		uint elements = 1; // number of elements in array (if parameter is array)
	};
	vector<ConstantBufferParameter> parameters;

	std::unique_ptr<uint8[]> data;
	bool needFlush = true;

	WRL::ComPtr<ID3D11Buffer> dxBuffer;

public:
	ConstantBuffer(WRL::ComPtr<ID3D11Buffer> dxBufferIn, uint bytesIn, const string& nameIn, const vector<ConstantBufferParameter>& paramsIn) :
		dxBuffer(dxBufferIn), bytes(bytesIn), name(nameIn), parameters(paramsIn)
	{
		data = std::make_unique<uint8[]>(bytesIn);
		memset(data.get(), '\0', bytesIn);
	}
	ConstantBuffer(const ConstantBuffer& r) = delete;
	ConstantBuffer(ConstantBuffer&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		dxBuffer = r.dxBuffer;
		r.dxBuffer = nullptr;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}
	ConstantBuffer& operator=(ConstantBuffer&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		dxBuffer = r.dxBuffer;
		r.dxBuffer = nullptr;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}
	ConstantBuffer& operator=(const ConstantBuffer& r) = delete;
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
	BlendStatePool _blendStatePool{*this};
	RasterizerStatePool _rasterizerStatePool{*this};
	DepthStencilStatePool _depthStencilStatePool{*this};	

	struct State
	{
		// Blending
		//
		D3D11_BLEND_DESC blendStateDesc;
		WRL::ComPtr<ID3D11BlendState> blendState;

		// Rasterizer
		//
		D3D11_RASTERIZER_DESC rasterStateDesc;
		WRL::ComPtr<ID3D11RasterizerState> rasterState;

		// Depth/Stencil
		//
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;

		// Viewport
		// TODO

		// Shader
		//
		WRL::ComPtr<IShader> shader;

		// Mesh
		//
		WRL::ComPtr<IMesh> mesh;

		// Textures
		//
		WRL::ComPtr<ITexture> texShaderBindings[16]; // slot -> texture

		// Framebuffer
		WRL::ComPtr<IRenderTarget> renderTarget;

		// Clear
		//
		FLOAT clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		FLOAT depthClearColor = 1.0f;
		UINT8 stencilClearColor = 0;
	};

	State _state;
	std::stack<State> _statesStack;

	IResourceManager *_pResMan = nullptr;

	int _MSAASamples = 0;
	int _VSyncOn = 1;

	bool create_default_buffers(uint w, uint h);
	void destroy_default_buffers();

	WRL::ComPtr<ID3DBlob> create_shader_by_src(ID3D11DeviceChild *&poiterOut, SHADER_TYPE type, const char *src, HRESULT& err);
	UINT msaa_quality(DXGI_FORMAT format, int MSAASamples);
	
public:

	ID3D11Device* getDevice() { return _device.Get(); }
	ID3D11DeviceContext* getContext() { return _context.Get(); }

	DX11CoreRender();
	virtual ~DX11CoreRender();

	API Init(const WindowHandle* handle, int MSAASamples, int VSyncOn) override;
	API Free() override;
	API MakeCurrent(const WindowHandle* handle) override;
	API SwapBuffers() override;

	API ClearState() override;
	API PushStates() override;
	API PopStates() override;

	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const char *vertText, const char *fragText, const char *geomText) override;
	API CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) override;
	API CreateRenderTarget(OUT ICoreRenderTarget **pRenderTarget) override;

	API SetCurrentRenderTarget(IRenderTarget *pRenderTarget) override;
	API RestoreDefaultRenderTarget() override;
	API SetShader(IShader *pShader) override;
	API SetMesh(IMesh* mesh) override;
	API Draw(IMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API SetViewport(uint w, uint h) override;
	API GetViewport(OUT uint* w, OUT uint* h) override;
	API Clear() override;

	API ReadPixel2D(ICoreTexture *tex, OUT void *out, OUT uint* readPixelBytes, uint x, uint y) override;
	
	API GetName(OUT const char **pNameOut) override;
};

