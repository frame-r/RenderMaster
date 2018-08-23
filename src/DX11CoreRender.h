#pragma once
#include "Common.h"
#include <wrl/module.h>
#include <d3d11_2.h>

namespace WRL = Microsoft::WRL;


class DX11ConstantBuffer final: public IUniformBuffer
{
	ID3D11Buffer *buffer{nullptr};

public:

	DX11ConstantBuffer(ID3D11Buffer *bufferIn) : buffer(bufferIn) {}

	ID3D11Buffer *nativeBuffer() const { return buffer; }

	API Free() override;
	API GetType(OUT RES_TYPE *type) override;
};

class DX11CoreRender final : public ICoreRender
{

	ID3D11DeviceContext *context{nullptr};
	ID3D11Device *device{nullptr};

	// TODO: make map HWND -> {IDXGISwapChain, ID3D11RenderTargetView} for support multiple windows
	WRL::ComPtr<IDXGISwapChain> swapChain;

	ID3D11RenderTargetView *renderTargetView{nullptr};
	ID3D11Texture2D* g_pDepthStencil{nullptr};
	ID3D11DepthStencilView* depthStencilView{nullptr};

	IResourceManager *_pResMan{nullptr};

	enum
	{
		TYPE_VERTEX,
		TYPE_GEOMETRY,
		TYPE_FRAGMENT,
	};

	ID3D11DeviceChild* _create_shader(int type, const char *src);
	const char* get_shader_profile(int type);
	const char* get_main_function(int type);

public:

	DX11CoreRender();
	~DX11CoreRender();
	
	API Init(const WinHandle* handle) override;
	API PushStates() override;
	API PopStates() override;
	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const ShaderText *shaderDesc) override;
	API SetShader(const ICoreShader *pShader) override;
	API CreateUniformBuffer(OUT IUniformBuffer **pBuffer, uint size) override;
	API SetUniform(IUniformBuffer *pBuffer, const void *pData) override;
	//API SetUniformArray(IUniformBuffer *pBuffer, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type, uint number) override;
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

