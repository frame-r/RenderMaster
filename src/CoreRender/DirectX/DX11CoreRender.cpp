#include "pch.h"
#include "DX11CoreRender.h"
#include "Core.h"
#include "DX11Shader.h"
#include "DX11Mesh.h"
#include "DX11Texture.h"

using WRL::ComPtr;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

vector<ConstantBuffer> ConstantBufferPool;

// By default in DirectX (and OpenGL) CPU-GPU transfer implemented in column-major style.
// We change this behaviour only here globally for all shaders by flag "D3DCOMPILE_PACK_MATRIX_ROW_MAJOR"
// to match C++ math lib wich keeps matrix in rom_major style.
#ifndef NDEBUG
	#define SHADER_COMPILE_FLAGS (D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG)
#else
	#define SHADER_COMPILE_FLAGS (D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3)
#endif

const char *get_shader_profile(SHADER_TYPE type);
const char *get_main_function(SHADER_TYPE type);
const char* dgxgi_to_hlsl_type(DXGI_FORMAT f);

DX11Mesh *getDX11Mesh(IMesh *mesh)
{
	ICoreMesh *cm = getCoreMesh(mesh);
	return static_cast<DX11Mesh*>(cm);
}

DX11Texture *getDX11Texture(ITexture *tex)
{
	ICoreTexture *ct = getCoreTexture(tex);
	return static_cast<DX11Texture*>(ct);
}

DX11Shader *getDX11Shader(IShader *shader)
{
	ICoreShader *cs = getCoreShader(shader);
	return static_cast<DX11Shader*>(cs);
}

DX11RenderTarget *getDX11RenderTarget(IRenderTarget *rt)
{
	ICoreRenderTarget *crt = getCoreRenderTarget(rt);
	return static_cast<DX11RenderTarget*>(crt);
}


DX11CoreRender::DX11CoreRender(){}
DX11CoreRender::~DX11CoreRender(){}

API DX11CoreRender::Init(const WindowHandle* handle, int MSAASamples, int VSyncOn)
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
		hr = _device.As(&dxgiDevice);
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

	_VSyncOn = VSyncOn;

	// MSAA
	{
		_MSAASamples = MSAASamples;

		while (_MSAASamples > 1)
		{
			UINT quality = msaa_quality(DXGI_FORMAT_R8G8B8A8_UNORM, _MSAASamples);
			UINT quality_ds = msaa_quality(DXGI_FORMAT_D24_UNORM_S8_UINT, _MSAASamples);

			if (quality && quality_ds) break;			

			_MSAASamples /= 2;
		}

		if (_MSAASamples != MSAASamples)
		{
			string need_msaa = msaa_to_string(MSAASamples);
			string actially_msaa = msaa_to_string(_MSAASamples);
			LOG_WARNING_FORMATTED("DX11CoreRender::Init() DirectX doesn't support %s MSAA. Now using %s MSAA", need_msaa.c_str(), actially_msaa.c_str());
		}
	}


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
		sd.SampleDesc.Count =_MSAASamples;
		sd.SampleDesc.Quality = _MSAASamples == 0 ? 1 : msaa_quality(DXGI_FORMAT_R8G8B8A8_UNORM, _MSAASamples);
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
		sd.SampleDesc.Count = _MSAASamples;
		sd.SampleDesc.Quality = _MSAASamples == 0 ? 1 : msaa_quality(DXGI_FORMAT_R8G8B8A8_UNORM, _MSAASamples);
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(_device.Get(), &sd, &_swapChain);
	}

	// We block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

	if (FAILED(hr))
		return hr;

	create_default_buffers(width, height);

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
	_state.rasterState = _rasterizerStatePool.FetchDefaultState();
	_context->RSSetState(_state.rasterState.Get());
	_state.rasterState->GetDesc(&_state.rasterStateDesc);

	// Debug
	//ComPtr<ID3D11RasterizerState> _rasterState;
	//_context->RSGetState(_rasterState.GetAddressOf());
	//D3D11_RASTERIZER_DESC desc;
	//_rasterState->GetDesc(&desc);

	// Depth Stencil state
	_state.depthStencilState = _depthStencilStatePool.FetchDefaultState();
	_context->OMSetDepthStencilState(_state.depthStencilState.Get(), 0);
	_state.depthStencilState->GetDesc(&_state.depthStencilDesc);

	// Debug
	//ComPtr<ID3D11DepthStencilState> _depthStencilState;
	//UINT sref = 0;
	//_context->OMGetDepthStencilState(_depthStencilState.GetAddressOf(), &sref);
	//D3D11_DEPTH_STENCIL_DESC dss;
	//_depthStencilState->GetDesc(&dss);

	// Blend State
	_state.blendState = _blendStatePool.FetchDefaultState();
	_context->OMSetBlendState(_state.blendState.Get(), nullptr, 0xffffffff);
	_state.blendState->GetDesc(&_state.blendStateDesc);

	LOG("DX11CoreRender initalized");

	return S_OK;
}

API DX11CoreRender::Free()
{
	ConstantBufferPool.clear();

	for (auto &callback : _onCleanBroadcast)
		callback();

	_context->ClearState();

	destroy_default_buffers();
	_swapChain = nullptr;

	_context = nullptr;

	LOG("DX11CoreRender::Free()");

	// Debug
	//{
	//	ComPtr<ID3D11Debug> pDebug;
	//	_device.As(&pDebug);	
	//	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);	
	//}

	_device = nullptr;

	return S_OK;
}

API DX11CoreRender::MakeCurrent(const WindowHandle* handle)
{
	return E_NOTIMPL;
}

API DX11CoreRender::SwapBuffers()
{
	_swapChain->Present(_VSyncOn, 0);
	return S_OK;
}

API DX11CoreRender::ClearState()
{
	_state = State();
	// TODO: filling field by default states
	_context->ClearState();
	return S_OK;
}

API DX11CoreRender::PushStates()
{
	_statesStack.push(_state);
	return S_OK;
}

API DX11CoreRender::PopStates()
{
	State& state = _statesStack.top();
	_statesStack.pop();

	static RasterHash rasterEq;
	static BlendHash blendEq;
	static DepthStencilHash depthStenciEq;

	if (!rasterEq.operator()(state.rasterStateDesc, _state.rasterStateDesc))
	{
		_context->RSSetState(state.rasterState.Get());

		_state.rasterStateDesc = state.rasterStateDesc;
		_state.rasterState = state.rasterState;
	}

	if (!blendEq.operator()(state.blendStateDesc, _state.blendStateDesc))
	{
		_context->OMSetBlendState(state.blendState.Get(), nullptr, 0xffffffff);

		_state.blendStateDesc = state.blendStateDesc;
		_state.blendState = state.blendState;
	}

	if (!depthStenciEq.operator()(state.depthStencilDesc, _state.depthStencilDesc))
	{
		_context->OMSetDepthStencilState(state.depthStencilState.Get(), 0);

		_state.depthStencilDesc = state.depthStencilDesc;
		_state.depthStencilState = state.depthStencilState;
	}

	_state = state;

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

	vector<D3D11_INPUT_ELEMENT_DESC> layout{{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}};
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
	string src;
	src = "struct VS_INPUT { ";

	for (int i = 0; i < layout.size(); i++)
	{
		const D3D11_INPUT_ELEMENT_DESC& el = layout[i];
		src += dgxgi_to_hlsl_type(el.Format) + string(" v") + std::to_string(i) + (i == 0 ? " : POSITION" : " : TEXCOORD") + std::to_string(el.SemanticIndex) + ";";
	}
	src += "}; struct VS_OUTPUT { float4 position : SV_POSITION; }; VS_OUTPUT mainVS(VS_INPUT input) { VS_OUTPUT o; o.position = float4(0,0,0,0); return o; } float4 PS( VS_OUTPUT input) : SV_Target { return float4(0,0,0,0); }";

	ComPtr<ID3DBlob> errorBuffer;
	ComPtr<ID3DBlob> shaderBuffer;

	auto hr = D3DCompile(src.c_str(), src.size(), "", NULL, NULL, "mainVS", get_shader_profile(SHADER_TYPE::SHADER_VERTEX), SHADER_COMPILE_FLAGS, 0, &shaderBuffer, &errorBuffer);

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

API DX11CoreRender::CreateShader(OUT ICoreShader **pShader, const char *vertText, const char *fragText, const char *geomText)
{
	HRESULT err;
	
	ID3D11VertexShader *vs = nullptr;
	auto vb = create_shader_by_src((ID3D11DeviceChild*&)vs, SHADER_TYPE::SHADER_VERTEX, vertText, err);
	if (!vs)
	{
		*pShader = nullptr;
		return err;
	}

	ID3D11PixelShader *fs = nullptr;
	auto fb = create_shader_by_src((ID3D11DeviceChild*&)fs, SHADER_TYPE::SHADER_FRAGMENT, fragText, err);
	if (!fs)
	{
		vs->Release();
		*pShader = nullptr;
		return err;
	}


	ID3D11GeometryShader *gs = nullptr;
	ComPtr<ID3DBlob> gb;
	if (geomText)
	{
		gb = create_shader_by_src((ID3D11DeviceChild*&)gs, SHADER_TYPE::SHADER_GEOMETRY, geomText, err);
		if (!gs && geomText)
		{
			vs->Release();
			fs->Release();
			*pShader = nullptr;
			return err;
		}
	}

	ShaderInitData vi = {vs, (unsigned char *)vb->GetBufferPointer(), vb->GetBufferSize()};
	ShaderInitData fi = {fs, (unsigned char *)fb->GetBufferPointer(), fb->GetBufferSize()};
	ShaderInitData gi = {gs, (gs ? (unsigned char *)(gb->GetBufferPointer()) : nullptr), (gs ? gb->GetBufferSize() : 0)};

	*pShader = new DX11Shader(vi, fi, gi);

	return S_OK;
}

DXGI_FORMAT resource_format(TEXTURE_FORMAT format)
{
	switch (format)
	{
		case TEXTURE_FORMAT::R8:		return DXGI_FORMAT_R8_UNORM;
		case TEXTURE_FORMAT::RG8:		return DXGI_FORMAT_R8G8_UNORM;
		case TEXTURE_FORMAT::RGB8:
		case TEXTURE_FORMAT::RGBA8:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TEXTURE_FORMAT::R16F:		return DXGI_FORMAT_R16_FLOAT;
		case TEXTURE_FORMAT::RG16F:		return DXGI_FORMAT_R16G16_FLOAT;
		case TEXTURE_FORMAT::RGB16F:
		case TEXTURE_FORMAT::RGBA16F:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TEXTURE_FORMAT::R32F:		return DXGI_FORMAT_R32_FLOAT;
		case TEXTURE_FORMAT::RG32F:		return DXGI_FORMAT_R32G32_FLOAT;
		case TEXTURE_FORMAT::RGB32F:
		case TEXTURE_FORMAT::RGBA32F:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TEXTURE_FORMAT::R32UI:		return DXGI_FORMAT_R32_UINT;
		case TEXTURE_FORMAT::DXT1:		return DXGI_FORMAT_BC1_UNORM;
		case TEXTURE_FORMAT::DXT3:		return DXGI_FORMAT_BC2_UNORM;
		case TEXTURE_FORMAT::DXT5:		return DXGI_FORMAT_BC3_UNORM;
		case TEXTURE_FORMAT::D24S8:		return DXGI_FORMAT_R24G8_TYPELESS;
	}

	LOG_WARNING("eng_to_d3d11_format(): unknown format\n");
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT srv_format(TEXTURE_FORMAT format)
{
	switch (format)
	{
	case TEXTURE_FORMAT::R8:		return DXGI_FORMAT_R8_UNORM;
	case TEXTURE_FORMAT::RG8:		return DXGI_FORMAT_R8G8_UNORM;
	case TEXTURE_FORMAT::RGB8:
	case TEXTURE_FORMAT::RGBA8:		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TEXTURE_FORMAT::R16F:		return DXGI_FORMAT_R16_FLOAT;
	case TEXTURE_FORMAT::RG16F:		return DXGI_FORMAT_R16G16_FLOAT;
	case TEXTURE_FORMAT::RGB16F:
	case TEXTURE_FORMAT::RGBA16F:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case TEXTURE_FORMAT::R32F:		return DXGI_FORMAT_R32_FLOAT;
	case TEXTURE_FORMAT::RG32F:		return DXGI_FORMAT_R32G32_FLOAT;
	case TEXTURE_FORMAT::RGB32F:
	case TEXTURE_FORMAT::RGBA32F:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case TEXTURE_FORMAT::R32UI:		return DXGI_FORMAT_R32_UINT;
	case TEXTURE_FORMAT::DXT1:		return DXGI_FORMAT_BC1_UNORM;
	case TEXTURE_FORMAT::DXT3:		return DXGI_FORMAT_BC2_UNORM;
	case TEXTURE_FORMAT::DXT5:		return DXGI_FORMAT_BC3_UNORM;
	case TEXTURE_FORMAT::D24S8:		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	}

	LOG_WARNING("eng_to_d3d11_format(): unknown format\n");
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT dsv_format(TEXTURE_FORMAT format)
{
	switch (format)
	{
	case TEXTURE_FORMAT::D24S8:		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	}

	LOG_WARNING("dsv_format(): unknown format\n");
	return DXGI_FORMAT_UNKNOWN;
}

UINT bind_flags(TEXTURE_CREATE_FLAGS flags, TEXTURE_FORMAT format)
{
	UINT bind_flags = D3D11_BIND_SHADER_RESOURCE;

	//if (is_color_format(format))
	//	bind_flags = D3D11_BIND_SHADER_RESOURCE;

	if (int(flags & TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET))
	{
		if (is_color_format(format))
			bind_flags |= D3D11_BIND_RENDER_TARGET;
		else
			bind_flags |= D3D11_BIND_DEPTH_STENCIL;
	}

	return bind_flags;
}

API DX11CoreRender::CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented)
{
	D3D11_TEXTURE2D_DESC texture_desc;
	texture_desc.Width = width;
	texture_desc.Height = height;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = resource_format(format);
	texture_desc.SampleDesc.Count = 1; // TODO: MSAA textures
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = bind_flags(flags, format);
	texture_desc.CPUAccessFlags = 0;
	texture_desc.MiscFlags = 0;

	ID3D11Texture2D *tex = nullptr;
	if (FAILED(_device->CreateTexture2D(&texture_desc, NULL, &tex)))
	{
		LOG_FATAL("DX11CoreRender::CreateTexture(): can't create texture\n");
		*pTexture = nullptr;
		return E_FAIL;
	}

	// shader resource view
	ID3D11ShaderResourceView *srv = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
	shader_resource_view_desc.Format = srv_format(format);
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.TextureCube.MostDetailedMip = 0;
	shader_resource_view_desc.TextureCube.MipLevels = texture_desc.MipLevels;
	
	if (FAILED(_device->CreateShaderResourceView(tex, &shader_resource_view_desc, &srv)))
	{
		tex->Release();
		LOG_FATAL("DX11CoreRender::CreateTexture(): can't create shader resource view\n");
		*pTexture = nullptr;
		return E_FAIL;
	}

	// render target (color\depth stencil)
	ID3D11RenderTargetView *rtv = nullptr;
	ID3D11DepthStencilView *dsv = nullptr;

	if (int(flags & TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET))
	{
		if (is_color_format(format))
		{
			D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
			render_target_view_desc.Format = texture_desc.Format;
			render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			render_target_view_desc.Texture2D.MipSlice = 0;

			if (FAILED(_device->CreateRenderTargetView(tex, &render_target_view_desc, &rtv)))
			{
				tex->Release();
				srv->Release();
				LOG_FATAL("DX11CoreRender::CreateTexture(): can't create render target view\n");
				*pTexture = nullptr;
				return E_FAIL;
			}
		} else
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
			dsv_desc.Flags = 0;
			dsv_desc.Format = dsv_format(format);
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Texture2D.MipSlice = 0;

			if (FAILED(_device->CreateDepthStencilView(tex, &dsv_desc, &dsv)))
			{
				tex->Release();
				srv->Release();
				LOG_FATAL("DX11CoreRender::CreateTexture(): can't create depth stencil view\n");
				*pTexture = nullptr;
				return E_FAIL;
			}
		}
	}

	DX11Texture *dxTex = new DX11Texture(tex, srv, rtv, dsv, format);
	*pTexture = dxTex;

	return S_OK;
}

API DX11CoreRender::CreateRenderTarget(OUT ICoreRenderTarget **pRenderTarget)
{
	*pRenderTarget = new DX11RenderTarget();
	return S_OK;
}

API DX11CoreRender::SetCurrentRenderTarget(IRenderTarget *pRenderTarget)
{
	assert(pRenderTarget && "DX11CoreRender::SetCurrentRenderTarget(): pRenderTarget can not be null. To set default framebuffer use DX11CoreRender::RestoreDefaultRenderTarget()");

	if (_state.renderTarget.Get() == pRenderTarget)
		return S_OK;

	_state.renderTarget = ComPtr<IRenderTarget>(pRenderTarget);

	DX11RenderTarget *dxrt = getDX11RenderTarget(pRenderTarget);
	dxrt->bind(_context.Get());

	return S_OK;
}

API DX11CoreRender::RestoreDefaultRenderTarget()
{
	if (_state.renderTarget.Get() == nullptr)
		return S_OK;

	_state.renderTarget = ComPtr<IRenderTarget>();

	_context->OMSetRenderTargets(1, _defaultRenderTargetView.GetAddressOf(), _defaultDepthStencilView.Get());

	return S_OK;
}


API DX11CoreRender::SetShader(IShader* pShader)
{
	if (_state.shader.Get() == pShader)
		return S_OK;

	_state.shader = ComPtr<IShader>(pShader);

	DX11Shader *dxShader = getDX11Shader(pShader);

	dxShader->bind();

	return S_OK;
}

API DX11CoreRender::SetMesh(IMesh* mesh)
{
	if (_state.mesh.Get() == mesh)
		return S_OK;

	_state.mesh = ComPtr<IMesh>(mesh);

	const DX11Mesh *dxMesh = getDX11Mesh(mesh);

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

//API DX11CoreRender::SetConstantBuffer(IConstantBuffer *pBuffer, uint slot)
//{
//	const DX11ConstantBuffer * dxBuffer = getDX11ConstantBuffer(pBuffer);
//	ID3D11Buffer *buf = dxBuffer->nativeBuffer();
//
//	_context->VSSetConstantBuffers(slot, 1, &buf);
//	_context->PSSetConstantBuffers(slot, 1, &buf);
//	_context->GSSetConstantBuffers(slot, 1, &buf);
//
//	return S_OK;
//}
//
//API DX11CoreRender::SetConstantBufferData(IConstantBuffer *pBuffer, const void *pData)
//{
//	const DX11ConstantBuffer * dxBuffer = getDX11ConstantBuffer(pBuffer);
//	ID3D11Buffer *buf = dxBuffer->nativeBuffer();
//
//	_context->UpdateSubresource(buf, 0, nullptr, pData, 0, 0);
//
//	return S_OK;
//}

API DX11CoreRender::Draw(IMesh* mesh)
{
	assert(_state.shader.Get() && "DX11CoreRender::Draw(): shader not set");

	_state.mesh = ComPtr<IMesh>(mesh);

	DX11Mesh *dxMesh = getDX11Mesh(mesh);

	if (dxMesh->indexBuffer())
		_context->DrawIndexed(dxMesh->indexNumber(), 0, 0);
	else
		_context->Draw(dxMesh->vertexNumber(), 0);

	return S_OK;
}

API DX11CoreRender::SetDepthState(int enabled)
{
	if (_state.depthStencilDesc.DepthEnable != enabled)
	{
		_state.depthStencilDesc.DepthEnable = enabled;
		_state.depthStencilState = _depthStencilStatePool.FetchState(_state.depthStencilDesc);
		_context->OMSetDepthStencilState(_state.depthStencilState.Get(), 0);
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

		destroy_default_buffers();
		
		if (FAILED(_swapChain->ResizeBuffers(1, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)))
			return E_ABORT;

		create_default_buffers(w, h);

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
	if (_state.renderTarget.Get() == nullptr) // default
	{
		_context->ClearRenderTargetView(_defaultRenderTargetView.Get(), _state.clearColor);
		_context->ClearDepthStencilView(_defaultDepthStencilView.Get(), D3D11_CLEAR_DEPTH, _state.depthClearColor, _state.stencilClearColor);
	} else
	{
		DX11RenderTarget *dxRT = getDX11RenderTarget(_state.renderTarget.Get());
		dxRT->clear(_context.Get(), _state.clearColor, _state.depthClearColor, _state.stencilClearColor);
	}

	return S_OK;
}

void DX11CoreRender::destroy_default_buffers()
{
	_defaultRenderTargetTex = nullptr;
	_defaultRenderTargetView = nullptr;
	_defaultDepthStencilTex = nullptr;
	_defaultDepthStencilView = nullptr;
}

UINT DX11CoreRender::msaa_quality(DXGI_FORMAT format, int MSAASamples)
{
	HRESULT hr;
	UINT maxQuality;
	hr = _device->CheckMultisampleQualityLevels(format, MSAASamples, &maxQuality);
	if (maxQuality > 0) maxQuality--;
	return maxQuality;
}

bool DX11CoreRender::create_default_buffers(uint w, uint h)
{
	if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)_defaultRenderTargetTex.GetAddressOf())))
		return false;

	if (FAILED(_device->CreateRenderTargetView(_defaultRenderTargetTex.Get(), nullptr, _defaultRenderTargetView.GetAddressOf())))
		return false;

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = w;
	descDepth.Height = h;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = _MSAASamples;
	descDepth.SampleDesc.Quality = _MSAASamples == 0 ? 1 : msaa_quality(DXGI_FORMAT_D24_UNORM_S8_UINT, _MSAASamples);
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	if (FAILED(_device->CreateTexture2D(&descDepth, nullptr, _defaultDepthStencilTex.GetAddressOf())))
		return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = _MSAASamples > 0 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	if (FAILED(_device->CreateDepthStencilView(_defaultDepthStencilTex.Get(), &descDSV, _defaultDepthStencilView.GetAddressOf())))
		return false;

	_context->OMSetRenderTargets(1, _defaultRenderTargetView.GetAddressOf(), _defaultDepthStencilView.Get());

	return true;
}

API DX11CoreRender::GetName(OUT const char **pTxt)
{
	*pTxt = "DX11CoreRender";
	return S_OK;
}

ComPtr<ID3DBlob> DX11CoreRender::create_shader_by_src(ID3D11DeviceChild *&poiterOut, SHADER_TYPE type, const char* src, HRESULT& err)
{
	ID3D11DeviceChild *ret = nullptr;
	ComPtr<ID3DBlob> error_buffer;
	ComPtr<ID3DBlob> shader_buffer;

	auto hr = D3DCompile(src, strlen(src), "", NULL, NULL, get_main_function(type), get_shader_profile(type), SHADER_COMPILE_FLAGS, 0, shader_buffer.GetAddressOf(), error_buffer.GetAddressOf());

	if (FAILED(hr))
	{
		const char *type_str = nullptr;
		switch (type)
		{
		case SHADER_TYPE::SHADER_VERTEX: type_str = "vertex"; err = E_VERTEX_SHADER_FAILED_COMPILE; break;
		case SHADER_TYPE::SHADER_GEOMETRY: type_str = "geometry"; err = E_GEOM_SHADER_FAILED_COMPILE; break;
		case SHADER_TYPE::SHADER_FRAGMENT: type_str = "fragment"; err = E_FRAGMENT_SHADER_FAILED_COMPILE; break;
		}

		if (error_buffer)
		{
			LOG_FATAL_FORMATTED("DX11CoreRender::create_shader_by_src() failed to compile %s shader. Error:", type_str);
			LOG_FATAL_FORMATTED("%s", (char*)error_buffer->GetBufferPointer());
		}
	}
	else
	{
		unsigned char *data = (unsigned char *)shader_buffer->GetBufferPointer();
		int size = (int)shader_buffer->GetBufferSize();
		HRESULT res = E_FAIL;

		switch (type)
		{
		case SHADER_TYPE::SHADER_VERTEX:
			res = _device->CreateVertexShader(data, size, NULL, (ID3D11VertexShader**)&ret);
			break;
		case SHADER_TYPE::SHADER_GEOMETRY:
			res = _device->CreateGeometryShader(data, size, NULL, (ID3D11GeometryShader**)&ret);
			break;
		case SHADER_TYPE::SHADER_FRAGMENT:
			res = _device->CreatePixelShader(data, size, NULL, (ID3D11PixelShader**)&ret);
			break;
		}

		poiterOut = ret;
	}

	return shader_buffer;
}

API DX11CoreRender::ReadPixel2D(ICoreTexture *tex, OUT void *out, OUT uint *readBytes, uint x, uint y)
{
	DX11Texture *d3dtex = static_cast<DX11Texture*>(tex);
	ID3D11Texture2D *d3dtex2d = static_cast<ID3D11Texture2D*>(d3dtex->resource());

	ID3D11Texture2D* cpuReadTex;
	D3D11_TEXTURE2D_DESC cpuReadTexDesc;
	ZeroMemory(&cpuReadTexDesc, sizeof(cpuReadTexDesc));
	cpuReadTexDesc.Width = d3dtex->width();
	cpuReadTexDesc.Height = d3dtex->height();
	cpuReadTexDesc.MipLevels = 1;
	cpuReadTexDesc.ArraySize = 1;
	cpuReadTexDesc.Format = d3dtex->desc().Format;
	cpuReadTexDesc.SampleDesc.Count = 1;
	cpuReadTexDesc.SampleDesc.Quality = 0;
	cpuReadTexDesc.Usage = D3D11_USAGE_STAGING;
	cpuReadTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	cpuReadTexDesc.MiscFlags = 0;

	_device->CreateTexture2D(&cpuReadTexDesc, nullptr, &cpuReadTex);

	_context->CopyResource(cpuReadTex, d3dtex2d);
	D3D11_MAPPED_SUBRESOURCE mapResource;
	auto hr = _context->Map(cpuReadTex, 0, D3D11_MAP_READ, 0, &mapResource);

	// TODO: make other!
	int byteWidthElement = 4;

	void *src = (char*)mapResource.pData + y * mapResource.RowPitch + x * byteWidthElement;
	memcpy(out, reinterpret_cast<void*>(src), byteWidthElement);

	*readBytes = byteWidthElement;

	//struct Color {float r, g, b, a;};
	//Color* obj;
	//obj=new Color[(mapResource.RowPitch/sizeof(Color)) * Height];
	//memcpy(obj, mapResource.pData,mapResource.RowPitch * Height);

	//if(mousePos.x>0&&mousePos.x<Width&&mousePos.y>0&&mousePos.y<Height)
	//if(obj[(mousePos.y*Width)+(4*mousePos.y)+mousePos.x].r==1.0/*If object that we was pick mouse have 1.0 on red bit we draw little cloud*/)model.rysujOtoczk?(model.dolny,model.gorny);
	
	_context->Unmap(cpuReadTex,0);
	cpuReadTex->Release();
	//delete [] obj;

	return S_OK;
}

const char *get_shader_profile(SHADER_TYPE type)
{
	switch (type)
	{
		case SHADER_TYPE::SHADER_VERTEX: return "vs_5_0";
		case SHADER_TYPE::SHADER_GEOMETRY: return "gs_5_0";
		case SHADER_TYPE::SHADER_FRAGMENT: return "ps_5_0";
	}
	return nullptr;
}

const char *get_main_function(SHADER_TYPE type)
{
	switch (type)
	{
		case SHADER_TYPE::SHADER_VERTEX: return "mainVS";
		case SHADER_TYPE::SHADER_GEOMETRY: return "mainGS";
		case SHADER_TYPE::SHADER_FRAGMENT: return "mainFS";
	}
	return nullptr;
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

void DX11RenderTarget::_get_colors(ID3D11RenderTargetView **arrayOut, uint &targetsNum)
{
	uint targets = 0;
	for (int i = 0; i < 8; i++)
	{
		if (_colors[i])
		{
			ITexture *tex = _colors[i].Get();
			ICoreTexture *coreTex;
			tex->GetCoreTexture(&coreTex);
			DX11Texture *dxct = static_cast<DX11Texture*>(coreTex);
			arrayOut[i] = dxct->rtView();
			targets++;
		} else
			break;
	}

	targetsNum = targets;
}

void DX11RenderTarget::_get_depth(ID3D11DepthStencilView **depthOut)
{
	ITexture *dtex = _depth.Get();
	ICoreTexture *dcoreTex;
	dtex->GetCoreTexture(&dcoreTex);
	DX11Texture *dxdct = static_cast<DX11Texture*>(dcoreTex);
	*depthOut = dxdct->dsView();
}

DX11RenderTarget::~DX11RenderTarget()
{
}

void DX11RenderTarget::bind(ID3D11DeviceContext *ctx)
{
	ID3D11RenderTargetView *renderTargetViews[8];
	UINT targets;
	_get_colors(renderTargetViews, targets);

	ID3D11DepthStencilView *depthStencilView = nullptr;
	if (_depth)
		_get_depth(&depthStencilView);

	ctx->OMSetRenderTargets(targets, renderTargetViews, depthStencilView);
}

void DX11RenderTarget::clear(ID3D11DeviceContext *ctx, FLOAT* color, FLOAT depth, UINT8 stencil)
{
	ID3D11RenderTargetView *renderTargetViews[8];
	UINT targets;
	_get_colors(renderTargetViews, targets);

	for (UINT i = 0; i < targets; i++)
	{
		ctx->ClearRenderTargetView(renderTargetViews[i], color);
	}

	ID3D11DepthStencilView *depthStencilView = nullptr;
	if (_depth)
	{
		_get_depth(&depthStencilView);
		ctx->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, depth, stencil);
	}
}

API DX11RenderTarget::SetColorTexture(uint slot, ITexture *tex)
{
	assert(slot < 8 && "DX11RenderTarget::SetColorTexture() slot must be 0..7");
	_colors[slot] = ComPtr<ITexture>(tex);
	return S_OK;
}

API DX11RenderTarget::SetDepthTexture(ITexture *tex)
{
	_depth = ComPtr<ITexture>(tex);
	return S_OK;
}

API DX11RenderTarget::UnbindColorTexture(uint slot)
{
	assert(slot < 8 && "DX11RenderTarget::SetColorTexture() slot must be 0..7");
	_colors[slot] = ComPtr<ITexture>();
	return S_OK;
}

API DX11RenderTarget::UnbindAll()
{
	for (auto &rtx : _colors)
		rtx = ComPtr<ITexture>();
	_depth = ComPtr<ITexture>();
	return S_OK;
}
