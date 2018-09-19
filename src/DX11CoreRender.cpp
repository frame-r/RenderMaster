#include "DX11CoreRender.h"
#include "Core.h"
#include "DX11Shader.h"
#include "DX11Mesh.h"
#include <d3dcompiler.h>

using WRL::ComPtr;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

enum
{
	SHADER_VERTEX,
	SHADER_GEOMETRY,
	SHADER_FRAGMENT,
};

const char *get_shader_profile(int type);
const char *get_main_function(int type);
const char* dgxgi_to_hlsl_type(DXGI_FORMAT f);


DX11CoreRender::DX11CoreRender(){}
DX11CoreRender::~DX11CoreRender(){}

API DX11CoreRender::Init(const WinHandle* handle)
{
	_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	HRESULT hr = S_OK;

	RECT rc;
	HWND hwnd = *handle;
	GetClientRect(hwnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
	#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = 3;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = 4;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		D3D_DRIVER_TYPE g_driverType = driverTypes[driverTypeIndex];
		D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;

		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, _device.GetAddressOf(), &g_featureLevel, _context.GetAddressOf());

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1, D3D11_SDK_VERSION, _device.GetAddressOf(), &g_featureLevel, _context.GetAddressOf());
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	ComPtr<IDXGIFactory1> dxgiFactory;
	{
		ComPtr<IDXGIDevice> dxgiDevice;
		hr = _device->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);
		if (SUCCEEDED(hr))
		{
			ComPtr<IDXGIAdapter> adapter;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), &dxgiFactory);
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	ComPtr<IDXGIFactory2> dxgiFactory2;
	hr = dxgiFactory.As(&dxgiFactory2);
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		//hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
		//if (SUCCEEDED(hr))
		//{
		//	(void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
		//}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		ComPtr<IDXGISwapChain1> g_pSwapChain1;
		hr = dxgiFactory2->CreateSwapChainForHwnd(_device.Get(), hwnd, &sd, nullptr, nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1.As(&_swapChain);
		}
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(_device.Get(), &sd, &_swapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);


	if (FAILED(hr))
		return hr;

	create_viewport_buffers(width, height);

	// Viewport state
	//
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_context->RSSetViewports(1, &vp);


	// Rasterizer state
	//
	auto defaultRasterState = _rasterizerStatePool.FetchDefaultState();
	_context->RSSetState(defaultRasterState.Get());
	defaultRasterState->GetDesc(&_currentState.rasterState);

	// debug
	ComPtr<ID3D11RasterizerState> _rasterState;
	_context->RSGetState(_rasterState.GetAddressOf());
	D3D11_RASTERIZER_DESC desc;
	_rasterState->GetDesc(&desc);


	// Depth Stencil state
	//
	auto defaultDepthStencilState = _depthStencilStatePool.FetchDefaultState();
	_context->OMSetDepthStencilState(defaultDepthStencilState.Get(), 0);
	defaultDepthStencilState->GetDesc(&_currentState.depthState);

	// debug
	ComPtr<ID3D11DepthStencilState> _depthStencilState;
	UINT sref = 0;
	_context->OMGetDepthStencilState(_depthStencilState.GetAddressOf(), &sref);
	D3D11_DEPTH_STENCIL_DESC dss;
	_depthStencilState->GetDesc(&dss);


	// Blend State
	//
	auto defaultBlendState = _blendStatePool.FetchDefaultState();
	_context->OMSetBlendState(defaultBlendState.Get(), nullptr, 0xffffffff);
	defaultBlendState->GetDesc(&_currentState.blendState);


	LOG("DX11CoreRender initalized");

	return S_OK;
}

API DX11CoreRender::Free()
{
	for (auto &callback : _onCleanBroadcast)
		callback();

	_context->ClearState();

	destroy_viewport_buffers();
	_swapChain = nullptr;

	_context = nullptr;

	LOG("DX11CoreRender::Free()");

	// debug
	//ID3D11Debug *pDebug;
	//auto hr = _device->QueryInterface(IID_PPV_ARGS(&pDebug));
	//if (pDebug != nullptr)
	//{
	//	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	//	pDebug->Release();
	//}

	_device = nullptr;

	return S_OK;
}

API DX11CoreRender::MakeCurrent(const WinHandle* handle)
{
	return E_NOTIMPL;
}

API DX11CoreRender::SwapBuffers()
{
	_swapChain->Present(1, 0);
	return S_OK;
}

API DX11CoreRender::PushStates()
{
	_statesStack.push(_currentState);
	return S_OK;
}

API DX11CoreRender::PopStates()
{
	State& state = _statesStack.top();
	_statesStack.pop();

	static RasterHash rasterEq;
	static BlendHash blendEq;
	static DepthStencilHash depthStenciEq;

	if (!rasterEq.operator()(state.rasterState, _currentState.rasterState))
	{
		auto s = _rasterizerStatePool.FetchState(state.rasterState);
		_context->RSSetState(s.Get());
	}

	if (!blendEq.operator()(state.blendState, _currentState.blendState))
	{
		auto s = _blendStatePool.FetchState(state.blendState);
		_context->OMSetBlendState(s.Get(), nullptr, 0xffffffff);
	}

	if (!depthStenciEq.operator()(state.depthState, _currentState.depthState))
	{
		auto s = _depthStencilStatePool.FetchState(state.depthState);
		_context->OMSetDepthStencilState(s.Get(), 0);
	}

	_currentState = state;

	return S_OK;
}

API DX11CoreRender::CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode)
{
	assert(dataDesc->colorOffset % 8 == 0 && "");
	assert(dataDesc->colorStride % 8 == 0 && "");
	assert(dataDesc->positionStride % 8 == 0 && "");
	assert(dataDesc->positionOffset % 8 == 0 && "");
	assert(dataDesc->normalOffset % 8 == 0 && "");
	assert(dataDesc->normalStride % 8 == 0 && "");

	const int indexes = indexDesc->format != MESH_INDEX_FORMAT::NOTHING;
	const int normals = dataDesc->normalsPresented;
	const int texCoords = dataDesc->texCoordPresented;
	const int colors = dataDesc->colorPresented;
	const int bytesWidth = 16 + 16 * normals + 8 * texCoords + 16 * colors;
	const int bytes = bytesWidth * dataDesc->numberOfVertex;

	INPUT_ATTRUBUTE attribs = INPUT_ATTRUBUTE::POSITION;
	if (dataDesc->normalsPresented)
		attribs = attribs | INPUT_ATTRUBUTE::NORMAL;
	if (dataDesc->texCoordPresented)
		attribs = attribs | INPUT_ATTRUBUTE::TEX_COORD;
	if (dataDesc->colorPresented)
		attribs = attribs | INPUT_ATTRUBUTE::COLOR;

	ComPtr<ID3DBlob> blob;
	
	//
	// input layout
	ID3D11InputLayout *il = nullptr;
	unsigned int offset = 0;

	std::vector<D3D11_INPUT_ELEMENT_DESC> layout{{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}};
	offset += 16;

	if (normals)
	{
		layout.push_back({"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0});
		offset += 16;
	}

	if (texCoords)
	{
		layout.push_back({"TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0});
		offset += 8;
	}

	if (colors)
	{
		layout.push_back({"TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0});
		offset += 16;
	}
	
	//
	// create dummy shader for CreateInputLayout() 
	std::string src;
	src = "struct VS_INPUT { ";

	for (int i = 0; i < layout.size(); i++)
	{
		const D3D11_INPUT_ELEMENT_DESC& el = layout[i];
		src += dgxgi_to_hlsl_type(el.Format) + std::string(" v") + std::to_string(i) + (i == 0 ? " : POSITION" : " : TEXCOORD") + std::to_string(el.SemanticIndex) + ";";
	}
	src += "}; struct VS_OUTPUT { float4 position : SV_POSITION; }; VS_OUTPUT mainVS(VS_INPUT input) { VS_OUTPUT o; o.position = float4(0,0,0,0); return o; } float4 PS( VS_OUTPUT input) : SV_Target { return float4(0,0,0,0); }";

	ComPtr<ID3DBlob> errorBuffer;
	ComPtr<ID3DBlob> shaderBuffer;

	auto hr = D3DCompile(src.c_str(), src.size(), "", NULL, NULL, "mainVS", get_shader_profile(0), (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS), 0, &shaderBuffer, &errorBuffer);

	if (FAILED(hr))
	{
		if (errorBuffer)
			LOG_FATAL_FORMATTED("DX11CoreRender::createDummyShader() failed to compile shader %s\n", (char*)errorBuffer->GetBufferPointer());
		
		return hr;
	}

	//
	// create input layout
	hr = _device->CreateInputLayout(reinterpret_cast<const D3D11_INPUT_ELEMENT_DESC*>(&layout[0]), (UINT)layout.size(), shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &il);

	if (FAILED(hr))
		return hr;	

	//
	// vertex buffer
	ID3D11Buffer *vb = nullptr;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = bytes;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = dataDesc->pData;

	hr = _device->CreateBuffer(&bd, &initData, &vb);

	if (FAILED(hr))
	{
		vb->Release();
		return hr;
	}

	//
	// index buffer
	ID3D11Buffer *ib = {nullptr};

	if (indexes)
	{
		int idxSize = 0;

		switch (indexDesc->format)
		{
			case MESH_INDEX_FORMAT::INT32: idxSize = 4; break;
			case MESH_INDEX_FORMAT::INT16: idxSize = 2; break;
		}
		const int idxBytes = idxSize * indexDesc->number;

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = idxBytes;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA ibData;
		ibData.pSysMem = indexDesc->pData;
		ibData.SysMemPitch = 0;
		ibData.SysMemSlicePitch = 0;

		hr = _device->CreateBuffer(&bufferDesc, &ibData, &ib);

		if (FAILED(hr))
		{
			vb->Release();
			il->Release();
			return hr;
		}
	}

	*pMesh = new DX11Mesh(vb, ib, il, dataDesc->numberOfVertex, indexDesc->number, indexDesc->format, mode, attribs, bytesWidth);

	return S_OK;
}

API DX11CoreRender::CreateShader(OUT ICoreShader **pShader, const ShaderText *shaderDesc)
{
	ID3D11VertexShader *vs = (ID3D11VertexShader*) create_shader_by_src(SHADER_VERTEX, shaderDesc->pVertText);
	ID3D11PixelShader *fs = (ID3D11PixelShader*) create_shader_by_src(SHADER_FRAGMENT, shaderDesc->pFragText);
	ID3D11GeometryShader *gs = shaderDesc->pGeomText ? (ID3D11GeometryShader*)create_shader_by_src(SHADER_GEOMETRY, shaderDesc->pGeomText) : nullptr;

	*pShader = new DX11Shader(vs, gs, fs);

	return S_OK;
}

API DX11CoreRender::SetShader(const ICoreShader* pShader)
{
	const DX11Shader *dxShader = static_cast<const DX11Shader*>(pShader);

	_context->VSSetShader(dxShader->vs(), nullptr, 0);
	_context->GSSetShader(dxShader->gs(), nullptr, 0);
	_context->PSSetShader(dxShader->fs(), nullptr, 0);

	return S_OK;
}

API DX11CoreRender::CreateUniformBuffer(OUT IUniformBuffer **pBuffer, uint size)
{
	ComPtr<ID3D11Buffer> ret;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	auto hr = _device->CreateBuffer(&bd, nullptr, ret.GetAddressOf());

	*pBuffer = new DX11ConstantBuffer(ret.Get());

	return hr;
}

API DX11CoreRender::SetUniform(IUniformBuffer *pBuffer, const void *pData)
{
	_context->UpdateSubresource(static_cast<const DX11ConstantBuffer *>(pBuffer)->nativeBuffer(), 0, nullptr, pData, 0, 0);

	return S_OK;
}

API DX11CoreRender::SetUniformBufferToShader(IUniformBuffer *pBuffer, uint slot)
{
	ID3D11Buffer *buf = static_cast<DX11ConstantBuffer *>(pBuffer)->nativeBuffer();

	_context->VSSetConstantBuffers(slot, 1, &buf);
	_context->PSSetConstantBuffers(slot, 1, &buf);
	_context->GSSetConstantBuffers(slot, 1, &buf);

	return S_OK;
}

API DX11CoreRender::SetMesh(const ICoreMesh* mesh)
{
	const DX11Mesh *dxMesh = static_cast<const DX11Mesh*>(mesh);

	_context->IASetInputLayout(dxMesh->inputLayout());

	ID3D11Buffer *vb = dxMesh->vertexBuffer();
	UINT offset = 0;
	UINT stride = dxMesh->stride();

	_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

	if (dxMesh->indexBuffer())
		_context->IASetIndexBuffer(dxMesh->indexBuffer(), (dxMesh->indexFormat() == MESH_INDEX_FORMAT::INT16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT), 0);
	
	_context->IASetPrimitiveTopology(dxMesh->topology() == VERTEX_TOPOLOGY::TRIANGLES? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST : D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	return S_OK;
}

API DX11CoreRender::Draw(ICoreMesh* mesh)
{
	if (static_cast<const DX11Mesh*>(mesh)->indexBuffer())
		_context->DrawIndexed(static_cast<const DX11Mesh*>(mesh)->indexNumber(), 0, 0);
	else
		_context->Draw(static_cast<const DX11Mesh*>(mesh)->vertexNumber(), 0);

	return S_OK;
}

API DX11CoreRender::SetDepthState(int enabled)
{
	if (_currentState.depthState.DepthEnable != enabled)
	{
		_currentState.depthState.DepthEnable = enabled;
		ComPtr<ID3D11DepthStencilState> d = _depthStencilStatePool.FetchState(_currentState.depthState);
		_context->OMSetDepthStencilState(d.Get(), 0);
	}

	return S_OK;
}

API DX11CoreRender::SetViewport(uint w, uint h)
{
	D3D11_VIEWPORT v;
	UINT viewportNumber = 1;

	_context->RSGetViewports(&viewportNumber, &v);

	uint old_w = (uint)v.Width;
	uint old_h = (uint)v.Height;

	if (old_w != w || old_h != h)
	{
		v.Height = (float)h;
		v.Width = (float)w;

		destroy_viewport_buffers();
		
		if (FAILED(_swapChain->ResizeBuffers(1, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)))
			return E_ABORT;

		create_viewport_buffers(w, h);

		_context->RSSetViewports(1, &v);
	}

	return S_OK;
}

API DX11CoreRender::GetViewport(OUT uint* wOut, OUT uint* hOut)
{
	D3D11_VIEWPORT v;
	UINT viewportNumber = 1;

	_context->RSGetViewports(&viewportNumber, &v);

	*wOut = (uint)v.Width;
	*hOut = (uint)v.Height;

	return S_OK;
}

API DX11CoreRender::Clear()
{
	static const FLOAT color[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	_context->ClearRenderTargetView(_renderTargetView.Get(), color);
	_context->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	return S_OK;
}

void DX11CoreRender::destroy_viewport_buffers()
{
	_renderTargetTex = nullptr;
	_renderTargetView = nullptr;
	_depthStencilTex = nullptr;
	_depthStencilView = nullptr;
}

bool DX11CoreRender::create_viewport_buffers(uint w, uint h)
{
	if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)_renderTargetTex.GetAddressOf())))
		return false;

	if (FAILED(_device->CreateRenderTargetView(_renderTargetTex.Get(), nullptr, _renderTargetView.GetAddressOf())))
		return false;

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = w;
	descDepth.Height = h;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	if (FAILED(_device->CreateTexture2D(&descDepth, nullptr, _depthStencilTex.GetAddressOf())))
		return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	if (FAILED(_device->CreateDepthStencilView(_depthStencilTex.Get(), &descDSV, _depthStencilView.GetAddressOf())))
		return false;

	_context->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());

	return true;
}

API DX11CoreRender::GetName(OUT const char **pTxt)
{
	*pTxt = "DX11CoreRender";
	return S_OK;
}

ID3D11DeviceChild* DX11CoreRender::create_shader_by_src(int type, const char* src)
{
	ID3D11DeviceChild *ret{nullptr};
	ComPtr<ID3DBlob> error_buffer;
	ComPtr<ID3DBlob> shader_buffer;

#ifndef NDEBUG
	constexpr UINT flags = (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG);
#else
	constexpr UINT flags = (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3);
#endif

	auto hr = D3DCompile(src, strlen(src), "", NULL, NULL, get_main_function(type), get_shader_profile(type), flags, 0, shader_buffer.GetAddressOf(), error_buffer.GetAddressOf());

	if (FAILED(hr))
	{
		if (error_buffer)
			LOG_FATAL_FORMATTED("DX11CoreRender::_create_shader() failed to compile shader %s\n", (char*)error_buffer->GetBufferPointer());
	}
	else
	{
		unsigned char *data = (unsigned char *)shader_buffer->GetBufferPointer();
		int size = (int)shader_buffer->GetBufferSize();
		HRESULT res = S_FALSE;

		switch (type)
		{
		case SHADER_VERTEX:
			res = _device->CreateVertexShader(data, size, NULL, (ID3D11VertexShader**)&ret);
			break;
		case SHADER_GEOMETRY:
			res = _device->CreateGeometryShader(data, size, NULL, (ID3D11GeometryShader**)&ret);
			break;
		case SHADER_FRAGMENT:
			res = _device->CreatePixelShader(data, size, NULL, (ID3D11PixelShader**)&ret);
			break;
		}

		if (ret)
			return ret;
	}

	return nullptr;
}

const char *get_shader_profile(int type)
{
	switch (type)
	{
		case SHADER_VERTEX: return "vs_5_0";
		case SHADER_GEOMETRY: return "gs_5_0";
		case SHADER_FRAGMENT: return "ps_5_0";
	}

	return NULL;
}

const char *get_main_function(int type)
{
	switch (type)
	{
		case SHADER_VERTEX: return "mainVS";
		case SHADER_GEOMETRY: return "mainGS";
		case SHADER_FRAGMENT: return "mainFS";
	}

	return NULL;
}

const char* dgxgi_to_hlsl_type(DXGI_FORMAT f)
{
	switch (f)
	{
	case DXGI_FORMAT_R32_FLOAT:
		return "float";
	case DXGI_FORMAT_R32G32_FLOAT:
		return "float2";
	case DXGI_FORMAT_R32G32B32_FLOAT:
		return "float3";
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return "float4";
	default:
		LOG_FATAL("DX11CoreRender: dgxgi_to_hlsl_type(DXGI_FORMAT f) unknown type f\n");
		assert(false);
		return nullptr;
		break;
	}
}

