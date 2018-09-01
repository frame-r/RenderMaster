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

	WRL::ComPtr<ID3D11RasterizerState> _rasterState;
	
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

