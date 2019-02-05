#pragma once
#include "Common.h"
#include "dx_objects.inl"

namespace WRL = Microsoft::WRL;


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
		//
		GLint x = 0, y = 0;
		GLint width = 0, heigth = 0;

		// Shader
		//
		ShaderPtr shader;

		// Mesh
		//
		MeshPtr mesh;

		// Textures
		//
		TexturePtr texShaderBindings[16]; // slot -> texture

		// Framebuffer
		RenderTargetPtr renderTarget;

		// Clear
		//
		FLOAT clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		FLOAT depthClearColor = 1.0f;
		UINT8 stencilClearColor = 0;
	};

	State _state;
	std::stack<State> _statesStack;

	IResourceManager *_pResMan = nullptr;

	int _MSAASamples = 1;
	int _VSyncOn = 1;

	bool createDefaultBuffers(uint w, uint h);
	void destroyDefaultBuffers();

	WRL::ComPtr<ID3DBlob> createShader(ID3D11DeviceChild *&poiterOut, SHADER_TYPE type, const char *src, HRESULT& err);
	UINT MSAAquality(DXGI_FORMAT format, int MSAASamples);
	
public:

	ID3D11Device* getDevice() { return _device.Get(); }
	ID3D11DeviceContext* getContext() { return _context.Get(); }

	DX11CoreRender();
	virtual ~DX11CoreRender();

	API_RESULT Init(const WindowHandle* handle, int MSAASamples, int VSyncOn) override;
	API_RESULT Free() override;
	API_RESULT MakeCurrent(const WindowHandle* handle) override;
	API_RESULT SwapBuffers() override;

	API_RESULT CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API_RESULT CreateShader(OUT ICoreShader **pShader, const char *vertText, const char *fragText, const char *geomText) override;
	API_RESULT CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) override;
	API_RESULT CreateRenderTarget(OUT ICoreRenderTarget **pRenderTarget) override;
	API_RESULT CreateStructuredBuffer(OUT ICoreStructuredBuffer **pStructuredBuffer, uint size, uint elementSize) override;

	API_VOID PushStates() override;
	API_VOID PopStates() override;

	API_VOID BindTexture(uint slot, ITexture* texture) override;
	API_VOID UnbindAllTextures() override;
	API_VOID SetCurrentRenderTarget(IRenderTarget *pRenderTarget) override;
	API_VOID RestoreDefaultRenderTarget() override;
	API_VOID SetShader(IShader *pShader) override;
	API_VOID SetMesh(IMesh* mesh) override;
	API_VOID SetStructuredBufer(uint slot, IStructuredBuffer* buffer) override;
	API_VOID Draw(IMesh *mesh, uint instances) override;
	API_VOID SetDepthTest(int enabled) override;
	API_VOID SetBlendState(BLEND_FACTOR src, BLEND_FACTOR dest) override;
	API_VOID SetViewport(uint w, uint h) override;
	API_VOID GetViewport(OUT uint* w, OUT uint* h) override;
	API_VOID Clear() override;

	API_VOID ReadPixel2D(ICoreTexture *tex, OUT void *out, OUT uint* readPixelBytes, uint x, uint y) override;
	API_VOID BlitRenderTargetToDefault(IRenderTarget *pRenderTarget) override;
	
	API_RESULT GetName(OUT const char **pNameOut) override;
};

