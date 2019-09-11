#include "pch.h"
#include "dx11corerender.h"
#include "dx_objects.inl"
#include "console.h"
#include "core.h"
#include "dx11mesh.h"
#include "dx11texture.h"
#include "texture.h"
#include "mesh.h"
#include "shader.h"
#include "structured_buffer.h"
#include "dx11shader.h"
#include "dx11structured_buffer.h"
#include "dx_objects.inl"
#include <stack>

using WRL::ComPtr;

#define MIN_DEPTH 0.0f
#define MAX_DEPTH 1.0f
#define MAX_TEXTURE_SLOTS 16
#define MAX_RENDER_TARGETS 8
//#define DEBUG_REPORT_LIVE

// By default in DirectX (and OpenGL) CPU-GPU transfer implemented in column-major style.
// We change this behaviour only here globally for all shaders by flag "D3DCOMPILE_PACK_MATRIX_ROW_MAJOR"
// to match C++ math lib wich keeps matrix in rom_major style.
#ifndef NDEBUG
	#define SHADER_COMPILE_FLAGS (D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG)
#else
	#define SHADER_COMPILE_FLAGS (D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3)
#endif

std::vector<ConstantBuffer> ConstantBufferPool;

static std::stack<DX11CoreRender::State> statesStack_;
static std::unordered_map<WindowHandle, WindowSurface> surfaces_;

void GetSurfaceInfo(
        _In_ size_t width,
        _In_ size_t height,
        _In_ DXGI_FORMAT fmt,
        size_t* outNumBytes,
        _Out_opt_ size_t* outRowBytes,
        _Out_opt_ size_t* outNumRows);

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
		assert(false); // unknown type
		return nullptr;
		break;
	}
}

const char *get_shader_profile(SHADER_TYPE type)
{
	switch (type)
	{
		case SHADER_TYPE::SHADER_VERTEX: return "vs_5_0";
		case SHADER_TYPE::SHADER_GEOMETRY: return "gs_5_0";
		case SHADER_TYPE::SHADER_FRAGMENT: return "ps_5_0";
		case SHADER_TYPE::SHADER_COMPUTE: return "cs_5_0";
	}
	assert(false);
	return nullptr;
}

const char *get_main_function(SHADER_TYPE type)
{
	switch (type)
	{
		case SHADER_TYPE::SHADER_VERTEX: return "mainVS";
		case SHADER_TYPE::SHADER_GEOMETRY: return "mainGS";
		case SHADER_TYPE::SHADER_FRAGMENT: return "mainFS";
		case SHADER_TYPE::SHADER_COMPUTE: return "mainCS";
	}
	assert(false);
	return nullptr;
}

DX11Mesh *getDX11Mesh(Mesh *mesh)
{
	ICoreMesh *cm = mesh->GetCoreMesh();
	return static_cast<DX11Mesh*>(cm);
}

//DX11Texture *getDX11Texture(Texture *tex)
//{
//	ICoreTexture *ct = tex->GetCoreTex(tex);
//	return static_cast<DX11Texture*>(ct);
//}

DX11Shader *getDX11Shader(Shader *shader)
{
	ICoreShader *cs = shader->GetCoreShader();
	return static_cast<DX11Shader*>(cs);
}

UINT DX11CoreRender::MSAAquality(DXGI_FORMAT format, int MSAASamples)
{
	HRESULT hr;
	UINT maxQuality;
	hr = _device->CheckMultisampleQualityLevels(format, MSAASamples, &maxQuality);
	if (maxQuality > 0) maxQuality--;
	return maxQuality;
}

uint DX11CoreRender::getNumLines()
{
	return 6;
}

std::string DX11CoreRender::getString(uint i)
{
	switch (i)
	{
		case 0: return "==== DX11 Core Render ====";
		case 1: return "draw calls: " + std::to_string(oldStat_.drawCalls);
		case 2: return "triangles: " + std::to_string(oldStat_.triangles);
		case 3: return "instances: " + std::to_string(oldStat_.instances);
		case 4: return "clear calls: " + std::to_string(oldStat_.clearCalls);
	}
	return "";
}

void DX11CoreRender::Update()
{
	oldStat_ = stat_;
	stat_.clear();
}

auto DX11CoreRender::Init(const WindowHandle* handle, int MSAASamples, int VSyncOn) -> bool
{
	HRESULT hr = S_OK;
	HWND hwnd = *handle;
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
		return false;

	VSyncOn_ = VSyncOn;

	// MSAA
	{
		MSAASamples_ = MSAASamples;

		while (MSAASamples_ > 1)
		{
			UINT quality = MSAAquality(DXGI_FORMAT_R8G8B8A8_UNORM, MSAASamples_);
			UINT quality_ds = MSAAquality(DXGI_FORMAT_D24_UNORM_S8_UINT, MSAASamples_);

			if (quality && quality_ds) break;

			MSAASamples_ /= 2;
		}

		if (MSAASamples_ != MSAASamples)
		{
			std::string need_msaa = msaa_to_string(MSAASamples);
			std::string actially_msaa = msaa_to_string(MSAASamples_);
			LogWarning("DX11CoreRender::Init() DirectX doesn't support %s MSAA. Now using %s MSAA", need_msaa.c_str(), actially_msaa.c_str());
		}
	}

	if (FAILED(hr))
		return false;

	// Rasterizer state
	state_.rasterState = _rasterizerStatePool.FetchDefaultState();
	_context->RSSetState(state_.rasterState.Get());
	state_.rasterState->GetDesc(&state_.rasterStateDesc);

	// Depth Stencil state
	state_.depthStencilState = _depthStencilStatePool.FetchDefaultState();
	_context->OMSetDepthStencilState(state_.depthStencilState.Get(), 0);
	state_.depthStencilState->GetDesc(&state_.depthStencilDesc);

	// Blend State
	state_.blendState = _blendStatePool.FetchDefaultState();
	_context->OMSetBlendState(state_.blendState.Get(), nullptr, 0xffffffff);
	state_.blendState->GetDesc(&state_.blendStateDesc);

	_core->AddProfilerCallback(this);

	Log("DX11CoreRender Inited");
	return true;
}

void DX11CoreRender::Free()
{
	_core->RemoveProfilerCallback(this);

	ConstantBufferPool.clear();

	state_.blendState = nullptr;
	state_.rasterState = nullptr;
	state_.depthStencilState = nullptr;

	for (auto &callback : _onCleanBroadcast)
		callback();

	_context->ClearState();

	surfaces_.clear();

	_context = nullptr;

	#ifdef DEBUG_REPORT_LIVE
	{
		ComPtr<ID3D11Debug> pDebug;
		_device.As(&pDebug);
		pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);	
	}
	#endif

	_device = nullptr;
	Log("DX11CoreRender Free");
}

void DX11CoreRender::bindSurface()
{
	DX11Texture *color = static_cast<DX11Texture*>(surface->color->GetCoreTexture());
	ID3D11RenderTargetView *rtv = color->rtView();

	DX11Texture *depth = static_cast<DX11Texture*>(surface->depth->GetCoreTexture());
	ID3D11DepthStencilView *dsv = depth->dsView();

	_context->OMSetRenderTargets(1, &rtv, dsv);

	for(int i = 0; i < 8; i++)
		state_.renderTargets[i] = nullptr;
	state_.renderDepth = nullptr;
	state_.renderTargets[0] = surface->color.get();
	state_.renderDepth = surface->depth.get();
}

auto DX11CoreRender::MakeCurrent(const WindowHandle *handle) -> void
{
	RECT r;
	GetClientRect(*handle, &r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;

	auto it = surfaces_.find(*handle);
	if (it != surfaces_.end())
	{
		surface = &it->second;

		if (!surface->color || !surface->depth)
			createCurrentSurface(w, h);

		bindSurface();

		return;
	}

	WindowSurface& s = surfaces_[*handle];
	surface = &s;

	// create new

	HRESULT hr = E_FAIL;

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

	ThrowIfFailed(hr);

	ComPtr<IDXGIFactory2> dxgiFactory2;
	hr = dxgiFactory.As(&dxgiFactory2);

	if (dxgiFactory2)
	{
		WRL::ComPtr<IDXGISwapChain1> swapChain1;

		DXGI_SWAP_CHAIN_DESC1 sd{};
		sd.Width = w;
		sd.Height = h;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count =MSAASamples_;
		sd.SampleDesc.Quality = MSAASamples_ <= 1 ? 0 : MSAAquality(DXGI_FORMAT_R8G8B8A8_UNORM, MSAASamples_);
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		
		hr = dxgiFactory2->CreateSwapChainForHwnd(_device.Get(), *handle, &sd, nullptr, nullptr, &swapChain1);
		if (SUCCEEDED(hr))
			hr = swapChain1.As(&surface->swapChain);
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd{};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = w;
		sd.BufferDesc.Height = h;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = *handle;
		sd.SampleDesc.Count = MSAASamples_;
		sd.SampleDesc.Quality = MSAASamples_ <= 1 ? 0 : MSAAquality(DXGI_FORMAT_R8G8B8A8_UNORM, MSAASamples_);
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(_device.Get(), &sd, &surface->swapChain);
	}

	// We block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(*handle, DXGI_MWA_NO_ALT_ENTER);

	if (!surface->color || !surface->depth)
		createCurrentSurface(w, h);

	bindSurface();
}

auto DX11CoreRender::SwapBuffers() -> void
{
	surface->swapChain->Present(VSyncOn_, 0);
}

auto DX11CoreRender::GetSurfaceColorTexture() -> Texture*
{
	return surface->color.get();
}

auto DX11CoreRender::GetSurfaceDepthTexture() -> Texture*
{
	return surface->depth.get();
}

auto DX11CoreRender::PushStates() -> void
{
	statesStack_.push(state_);
}

auto DX11CoreRender::PopStates() -> void
{
	State& state = statesStack_.top();

	static RasterHash rasterEq;
	static BlendHash blendEq;
	static DepthStencilHash depthStenciEq;

	if (!rasterEq.operator()(state.rasterStateDesc, state_.rasterStateDesc))
		_context->RSSetState(state.rasterState.Get());

	if (!blendEq.operator()(state.blendStateDesc, state_.blendStateDesc))
		_context->OMSetBlendState(state.blendState.Get(), nullptr, 0xffffffff);

	if (!depthStenciEq.operator()(state.depthStencilDesc, state_.depthStencilDesc))
		_context->OMSetDepthStencilState(state.depthStencilState.Get(), 0);	

	SetMesh(state.mesh);

	SetShader(state.shader);

	// Tetxures TODO
	//{
	//	// Find ranges for lowest common difference: [0 1 2 ... form ... to ... MAX_TEXTURE_SLOTS-1]
	//
	//	int to = MAX_TEXTURE_SLOTS - 1;
	//	while (to > -1 &&
	//		state.texShaderBindings[to] == _state.texShaderBindings[to])
	//	{
	//		to--;
	//	}
	//
	//	int from = 0;
	//	while (from <= to &&
	//		state.texShaderBindings[from] == _state.texShaderBindings[from])
	//	{
	//		from++;
	//	}
	//
	//	if (from <= to)
	//	{
	//		ID3D11ShaderResourceView *srv[MAX_TEXTURE_SLOTS] = {};
	//		ID3D11SamplerState *ss[MAX_TEXTURE_SLOTS] = {};
	//
	//		for (int i = from; i <= to; i++)
	//		{
	//			if (state.texShaderBindings[i])
	//			{
	//				DX11Texture *dxTex = getDX11Texture(state.texShaderBindings[i]);
	//				srv[i] = dxTex->srView();
	//				ss[i] = dxTex->sampler();
	//			}
	//		}
	//
	//		int num = to - from + 1;
	//
	//		_context->PSSetShaderResources(from, num, srv);
	//		_context->PSSetSamplers(from, num, ss);
	//	}
	//}

	// TODO
	// render target
	// clear color

	state_ = state;
	statesStack_.pop();
}

auto DX11CoreRender::CreateMesh(const MeshDataDesc * dataDesc, const MeshIndexDesc * indexDesc, VERTEX_TOPOLOGY mode) -> ICoreMesh *
{
	#define MULT_BY_8(X) assert(X % 8 == 0 && "DX11CoreRender::CreateMesh(): some argument is not multiplied by 8 bytes");

	MULT_BY_8(dataDesc->colorOffset)
	MULT_BY_8(dataDesc->colorStride)
	MULT_BY_8(dataDesc->positionStride)
	MULT_BY_8(dataDesc->positionOffset)
	MULT_BY_8(dataDesc->normalOffset)
	MULT_BY_8(dataDesc->normalStride)

	const int indexes = indexDesc->format != MESH_INDEX_FORMAT::NONE;
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

	ThrowIfFailed(D3DCompile(src.c_str(), src.size(), "", NULL, NULL, "mainVS", get_shader_profile(SHADER_TYPE::SHADER_VERTEX), SHADER_COMPILE_FLAGS, 0, &shaderBuffer, &errorBuffer));
	
	//
	// create input layout
	ThrowIfFailed((_device->CreateInputLayout(reinterpret_cast<const D3D11_INPUT_ELEMENT_DESC*>(&layout[0]), (UINT)layout.size(), shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &il)));

	//
	// vertex buffer
	ID3D11Buffer* vb{};

	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = bytes;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = dataDesc->pData;

	ThrowIfFailed(_device->CreateBuffer(&bd, &initData, &vb));

	//
	// index buffer
	ID3D11Buffer* ib{};

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

		ThrowIfFailed(_device->CreateBuffer(&bufferDesc, &ibData, &ib));
	}

	return new DX11Mesh(vb, ib, il, dataDesc->numberOfVertex, indexDesc->number, indexDesc->format, mode, attribs, bytesWidth);
}

DXGI_FORMAT engToDX11Format(TEXTURE_FORMAT format)
{
	switch (format)
	{
		case TEXTURE_FORMAT::R8:		return DXGI_FORMAT_R8_UNORM;
		case TEXTURE_FORMAT::RG8:		return DXGI_FORMAT_R8G8_UNORM;
		case TEXTURE_FORMAT::RGBA8:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TEXTURE_FORMAT::BGRA8:		return DXGI_FORMAT_B8G8R8A8_UNORM;
		case TEXTURE_FORMAT::R16F:		return DXGI_FORMAT_R16_FLOAT;
		case TEXTURE_FORMAT::RG16F:		return DXGI_FORMAT_R16G16_FLOAT;
		case TEXTURE_FORMAT::RGBA16F:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TEXTURE_FORMAT::R32F:		return DXGI_FORMAT_R32_FLOAT;
		case TEXTURE_FORMAT::RG32F:		return DXGI_FORMAT_R32G32_FLOAT;
		case TEXTURE_FORMAT::RGBA32F:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TEXTURE_FORMAT::R32UI:		return DXGI_FORMAT_R32_UINT;
		case TEXTURE_FORMAT::DXT1:		return DXGI_FORMAT_BC1_UNORM;
		case TEXTURE_FORMAT::DXT3:		return DXGI_FORMAT_BC2_UNORM;
		case TEXTURE_FORMAT::DXT5:		return DXGI_FORMAT_BC3_UNORM;
		case TEXTURE_FORMAT::D24S8:		return DXGI_FORMAT_R24G8_TYPELESS;
	}

	LogWarning("EngToDX11Format(): unknown format\n");
	return DXGI_FORMAT_UNKNOWN;
}

TEXTURE_FORMAT D3DToEng(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_R8_UNORM:				return TEXTURE_FORMAT::R8;	
		case DXGI_FORMAT_R8G8_UNORM:			return TEXTURE_FORMAT::RG8;		
		case DXGI_FORMAT_R8G8B8A8_UNORM:		return TEXTURE_FORMAT::RGBA8;		
		case DXGI_FORMAT_B8G8R8A8_UNORM:		return TEXTURE_FORMAT::BGRA8;		
		case DXGI_FORMAT_R16_FLOAT:				return TEXTURE_FORMAT::R16F;		
		case DXGI_FORMAT_R16G16_FLOAT:			return TEXTURE_FORMAT::RG16F;		
		case DXGI_FORMAT_R16G16B16A16_FLOAT:	return TEXTURE_FORMAT::RGBA16F;	
		case DXGI_FORMAT_R32_FLOAT:				return TEXTURE_FORMAT::R32F;	
		case DXGI_FORMAT_R32G32_FLOAT:			return TEXTURE_FORMAT::RG32F;		
		case DXGI_FORMAT_R32G32B32A32_FLOAT:	return TEXTURE_FORMAT::RGBA32F;	
		case DXGI_FORMAT_R32_UINT:				return TEXTURE_FORMAT::R32UI;		
		case DXGI_FORMAT_BC1_UNORM:				return TEXTURE_FORMAT::DXT1;		
		case DXGI_FORMAT_BC2_UNORM:				return TEXTURE_FORMAT::DXT3;		
		case DXGI_FORMAT_BC3_UNORM:				return TEXTURE_FORMAT::DXT5;		
		case DXGI_FORMAT_R24G8_TYPELESS:		return TEXTURE_FORMAT::D24S8;		
	}

	LogWarning("DXGIFormatToEng(): unknown format\n");
	return TEXTURE_FORMAT::UNKNOWN;
}

DXGI_FORMAT engToD3DSRV(TEXTURE_FORMAT format)
{
	switch (format)
	{
		case TEXTURE_FORMAT::R8:		return DXGI_FORMAT_R8_UNORM;
		case TEXTURE_FORMAT::RG8:		return DXGI_FORMAT_R8G8_UNORM;
		case TEXTURE_FORMAT::RGBA8:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TEXTURE_FORMAT::BGRA8:		return DXGI_FORMAT_B8G8R8A8_UNORM;
		case TEXTURE_FORMAT::R16F:		return DXGI_FORMAT_R16_FLOAT;
		case TEXTURE_FORMAT::RG16F:		return DXGI_FORMAT_R16G16_FLOAT;
		case TEXTURE_FORMAT::RGBA16F:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TEXTURE_FORMAT::R32F:		return DXGI_FORMAT_R32_FLOAT;
		case TEXTURE_FORMAT::RG32F:		return DXGI_FORMAT_R32G32_FLOAT;
		case TEXTURE_FORMAT::RGBA32F:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TEXTURE_FORMAT::R32UI:		return DXGI_FORMAT_R32_UINT;
		case TEXTURE_FORMAT::DXT1:		return DXGI_FORMAT_BC1_UNORM;
		case TEXTURE_FORMAT::DXT3:		return DXGI_FORMAT_BC2_UNORM;
		case TEXTURE_FORMAT::DXT5:		return DXGI_FORMAT_BC3_UNORM;
		case TEXTURE_FORMAT::D24S8:		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	}

	LogWarning("EngToD3DSRV(): unknown format\n");
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT engToD3DDSVFormat(TEXTURE_FORMAT format)
{
	switch (format)
	{
		case TEXTURE_FORMAT::D24S8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	}
	LogWarning("EngToD3DDSVFormat(): unknown format\n");
	return DXGI_FORMAT_UNKNOWN;
}

UINT bindFlags(TEXTURE_CREATE_FLAGS flags, TEXTURE_FORMAT format)
{
	UINT bindFlags_ = D3D11_BIND_SHADER_RESOURCE;
	if (int(flags & TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET))
	{
		if (isColorFormat(format))
			bindFlags_ |= D3D11_BIND_RENDER_TARGET;
		else
			bindFlags_ |= D3D11_BIND_DEPTH_STENCIL;
	}

	if (bool(flags & TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS) && !isCompressedFormat(format)) 
		bindFlags_ |= D3D11_BIND_RENDER_TARGET; // need for GenerateMips

	if (bool(flags & TEXTURE_CREATE_FLAGS::USAGE_UNORDRED_ACCESS)) 
		bindFlags_ |= D3D11_BIND_UNORDERED_ACCESS;

	return bindFlags_;
}

UINT msaaSamples(TEXTURE_CREATE_FLAGS flags)
{
	TEXTURE_CREATE_FLAGS f = flags & TEXTURE_CREATE_FLAGS::MSAA;
	switch(f)
	{
	case TEXTURE_CREATE_FLAGS::MSAA_2x: return 2;
	case TEXTURE_CREATE_FLAGS::MSAA_4x: return 4;
	case TEXTURE_CREATE_FLAGS::MSAA_8x: return 8;
	}

	return 1;
}

UINT getMisc(TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags)
{
	UINT ret = type == TEXTURE_TYPE::TYPE_CUBE ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

	if (bool(flags & TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS) && !isCompressedFormat(format))
		ret |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

	return ret;
}

void getFilter(D3D11_FILTER& ret, UINT& MaxAnisotropy, TEXTURE_CREATE_FLAGS flags)
{
}

int mipmapsNumber(int width, int height) // rounding down rule
{
	return 1 + (int)floor(log2((float)std::max(width, height)));
}

auto DX11CoreRender::CreateTexture(const uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) -> ICoreTexture*
{
	UINT arraySize = type == TEXTURE_TYPE::TYPE_CUBE ? 6 : 1;

	bool generateMipmaps = bool(flags & TEXTURE_CREATE_FLAGS::GENERATE_MIPMAPS) && !mipmapsPresented;

	if (isCompressedFormat(format) && generateMipmaps)
	{
		generateMipmaps = false;
		LogWarning("DX11CoreRender::CreateTexture(): Unable load mipmaps duo format is compressed");
	}

	UINT mipLevelsData = mipmapsPresented ? mipmapsNumber(width, height) : 1;
	UINT mipLevelsResource = (generateMipmaps || mipmapsPresented) ? mipmapsNumber(width, height) : 1;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = mipLevelsResource;
	desc.ArraySize = arraySize;
	desc.Format = engToDX11Format(format);
	desc.SampleDesc.Count = msaaSamples(flags);
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = bindFlags(flags, format);
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = getMisc(type, format, flags);

	ID3D11Texture2D *tex = nullptr;
	if (FAILED(_device->CreateTexture2D(&desc, NULL, &tex)))
	{
		LogCritical("DX11CoreRender::CreateTexture(): can't create texture");
		return nullptr;
	}

	if (pData)
	{
		if (arraySize > 1)
		{
			const uint8_t* pSrc = pData;
			for (UINT arraySlice = 0; arraySlice < arraySize; ++arraySlice)
			{
				uint w = width;
				uint h = height;
				for (UINT mip = 0; mip < mipLevelsData; mip++)
				{
					size_t rowBytes;
					size_t numBytes;
					GetSurfaceInfo(w, h, desc.Format, &numBytes, &rowBytes, nullptr);

					UINT res = D3D11CalcSubresource(mip, arraySlice, mipLevelsResource);

					_context->UpdateSubresource(tex, res, nullptr, pSrc, static_cast<UINT>(rowBytes), static_cast<UINT>(numBytes));

					pSrc += numBytes;
					w /= 2;
					h /= 2;
				}
			}
		}
		else
		{
			uint w = width;
			uint h = height;
			const uint8_t* pSrc = pData;

			for (UINT mip = 0; mip < mipLevelsData; mip++)
			{
				size_t rowBytes;
				size_t numBytes;
				GetSurfaceInfo(w, h, desc.Format, &numBytes, &rowBytes, nullptr);

				UINT res = D3D11CalcSubresource(mip, 0, mipLevelsResource);

				_context->UpdateSubresource(tex, res, nullptr, pSrc, static_cast<UINT>(rowBytes), static_cast<UINT>(numBytes));

				pSrc += numBytes;
				w /= 2;
				h /= 2;
			}
		}
	}

	// Sampler
	ID3D11SamplerState* sampler = nullptr;
	D3D11_SAMPLER_DESC sampDesc{};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	switch (TEXTURE_CREATE_FLAGS::FILTER & flags)
	{
		case TEXTURE_CREATE_FLAGS::FILTER_POINT: sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
		case TEXTURE_CREATE_FLAGS::FILTER_BILINEAR: sampDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT; break;
		case TEXTURE_CREATE_FLAGS::FILTER_TRILINEAR: sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
		case TEXTURE_CREATE_FLAGS::FILTER_ANISOTROPY_2X: sampDesc.Filter = D3D11_FILTER_ANISOTROPIC; sampDesc.MaxAnisotropy = 1; break;
		case TEXTURE_CREATE_FLAGS::FILTER_ANISOTROPY_4X: sampDesc.Filter = D3D11_FILTER_ANISOTROPIC; sampDesc.MaxAnisotropy = 2; break;
		case TEXTURE_CREATE_FLAGS::FILTER_ANISOTROPY_8X: sampDesc.Filter = D3D11_FILTER_ANISOTROPIC; sampDesc.MaxAnisotropy = 3; break;
		case TEXTURE_CREATE_FLAGS::FILTER_ANISOTROPY_16X: sampDesc.Filter = D3D11_FILTER_ANISOTROPIC; sampDesc.MaxAnisotropy = 4; break;
		default: sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
	}

	ThrowIfFailed(_device->CreateSamplerState(&sampDesc, &sampler));

	// SRV
	ID3D11ShaderResourceView *srv = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = engToD3DSRV(format);
	shaderResourceViewDesc.ViewDimension = type == TEXTURE_TYPE::TYPE_CUBE ?
		D3D11_SRV_DIMENSION_TEXTURECUBE :
		(desc.SampleDesc.Count > 1 ? D3D_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D);
	shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
	shaderResourceViewDesc.TextureCube.MipLevels = desc.MipLevels;
	
	if (FAILED(_device->CreateShaderResourceView(tex, &shaderResourceViewDesc, &srv)))
	{
		tex->Release();
		LogCritical("DX11CoreRender::CreateTexture(): can't create shader resource view\n");
		return nullptr;
	}

	if (generateMipmaps)
		_context->GenerateMips(srv);

	// RTV or DSV
	ID3D11RenderTargetView *rtv{};
	ID3D11DepthStencilView *dsv{};
	if (int(flags & TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET))
	{
		if (isColorFormat(format))
		{
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = desc.Format;
			renderTargetViewDesc.ViewDimension = desc.SampleDesc.Count > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;

			if (FAILED(_device->CreateRenderTargetView(tex, &renderTargetViewDesc, &rtv)))
			{
				tex->Release();
				srv->Release();
				LogCritical("DX11CoreRender::CreateTexture(): can't create render target view\n");
				return nullptr;
			}
		} else
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = 0;
			dsvDesc.Format = engToD3DDSVFormat(format);
			dsvDesc.ViewDimension = desc.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;

			if (FAILED(_device->CreateDepthStencilView(tex, &dsvDesc, &dsv)))
			{
				tex->Release();
				srv->Release();
				LogCritical("DX11CoreRender::CreateTexture(): can't create depth stencil view\n");
				return nullptr;
			}
		}
	}

	ID3D11UnorderedAccessView* uav{};
	if (int(flags & TEXTURE_CREATE_FLAGS::USAGE_UNORDRED_ACCESS))
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Format = desc.Format;
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		if (FAILED(_device->CreateUnorderedAccessView(tex, &desc, &uav)))
		{
			tex->Release();
			srv->Release();
			if (rtv)
				rtv->Release();
			LogCritical("DX11CoreRender::CreateTexture(): can't create UAV\n");
			return nullptr;
		}
	}

	DX11Texture *dxTex = new DX11Texture(tex, sampler, srv, rtv, dsv, uav, format);
	return dxTex;
}

ComPtr<ID3DBlob> DX11CoreRender::createShader(ID3D11DeviceChild *&poiterOut, SHADER_TYPE type, const char* src, ERROR_COMPILE_SHADER &err)
{
	ID3D11DeviceChild *ret = nullptr;
	ComPtr<ID3DBlob> error_buffer;
	ComPtr<ID3DBlob> shaderBuffer;

	auto hr = D3DCompile(src, strlen(src), "", NULL, NULL, get_main_function(type), get_shader_profile(type), SHADER_COMPILE_FLAGS, 0, shaderBuffer.GetAddressOf(), error_buffer.GetAddressOf());

	if (FAILED(hr))
	{
		const char *type_str = nullptr;
		switch (type)
		{
		case SHADER_TYPE::SHADER_VERTEX: type_str = "vertex"; err = ERROR_COMPILE_SHADER::VERTEX; break;
		case SHADER_TYPE::SHADER_GEOMETRY: type_str = "geometry"; err = ERROR_COMPILE_SHADER::GEOM; break;
		case SHADER_TYPE::SHADER_FRAGMENT: type_str = "fragment"; err = ERROR_COMPILE_SHADER::FRAGMENT; break;
		case SHADER_TYPE::SHADER_COMPUTE: type_str = "compute"; err = ERROR_COMPILE_SHADER::COMP; break;
		default:
			err = ERROR_COMPILE_SHADER::NONE;
		}

		if (error_buffer)
		{
			LogCritical("DX11CoreRender::create_shader_by_src() failed to compile %s shader. Error:", type_str);
			LogCritical("%s", (char*)error_buffer->GetBufferPointer());
		}
	}
	else
	{
		unsigned char *data = (unsigned char *)shaderBuffer->GetBufferPointer();
		int size = (int)shaderBuffer->GetBufferSize();
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
		case SHADER_TYPE::SHADER_COMPUTE:
			res = _device->CreateComputeShader(data, size, NULL, (ID3D11ComputeShader**)&ret);
			break;
		}

		poiterOut = ret;
	}

	return shaderBuffer;
}

auto DX11CoreRender::CreateShader(const char *vertText, const char *fragText, const char *geomText, ERROR_COMPILE_SHADER &err) -> ICoreShader*
{	
	ID3D11VertexShader *vs = nullptr;
	auto vb = createShader((ID3D11DeviceChild*&)vs, SHADER_TYPE::SHADER_VERTEX, vertText, err);
	if (!vs)
	{
		return nullptr;
	}

	ID3D11PixelShader *fs = nullptr;
	auto fb = createShader((ID3D11DeviceChild*&)fs, SHADER_TYPE::SHADER_FRAGMENT, fragText, err);
	if (!fs)
	{
		vs->Release();
		return nullptr;
	}

	ID3D11GeometryShader *gs = nullptr;
	ComPtr<ID3DBlob> gb;
	if (geomText)
	{
		gb = createShader((ID3D11DeviceChild*&)gs, SHADER_TYPE::SHADER_GEOMETRY, geomText, err);
		if (!gs && geomText)
		{
			vs->Release();
			fs->Release();
			return nullptr;
		}
	}

	ShaderInitData vi = {vs, (unsigned char *)vb->GetBufferPointer(), vb->GetBufferSize()};
	ShaderInitData fi = {fs, (unsigned char *)fb->GetBufferPointer(), fb->GetBufferSize()};
	ShaderInitData gi = {gs, (gs ? (unsigned char *)(gb->GetBufferPointer()) : nullptr), (gs ? gb->GetBufferSize() : 0)};

	return new DX11Shader(vi, fi, gi);
}

auto DX11CoreRender::CreateComputeShader(const char* compText, ERROR_COMPILE_SHADER& err) -> ICoreShader*
{
	ID3D11ComputeShader* cs = nullptr;
	auto vb = createShader((ID3D11DeviceChild * &)cs, SHADER_TYPE::SHADER_COMPUTE, compText, err);

	if (!cs)
		return nullptr;

	ShaderInitData vi = { cs, (unsigned char*)vb->GetBufferPointer(), vb->GetBufferSize() };

	return new DX11Shader(vi);
}

auto DX11CoreRender::CreateStructuredBuffer(uint size, uint elementSize) -> ICoreStructuredBuffer*
{
	assert(size % 16 == 0);

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = size;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = elementSize;

	ID3D11Buffer *pBuffer;
	ThrowIfFailed(_device->CreateBuffer(&desc, nullptr, &pBuffer));

	// Create SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;

	ID3D11ShaderResourceView *pSRVOut;
	ThrowIfFailed(_device->CreateShaderResourceView(pBuffer, &srvDesc, &pSRVOut));

	return new DX11StructuredBuffer(pBuffer, pSRVOut);
}

auto DX11CoreRender::BindTextures(int units, Texture **textures, BIND_TETURE_FLAGS flags) -> void
{
	ID3D11ShaderResourceView* srvs[16]{};
	ID3D11SamplerState* samplers[16]{};

	if (textures)
	{
		bool needUpdate = false;

		for(int i = 0; i < units; i++)
		{
			Texture *tt = *(textures + i);
			if (tt)
			{
				DX11Texture* t = static_cast<DX11Texture*>(tt->GetCoreTexture());
				srvs[i] = t->srView();
				samplers[i] = t->sampler();

				if (state_.srvs[i] != srvs[i])
					needUpdate = true;
			}
			else
			{
				if (state_.srvs[i] != nullptr)
					needUpdate = true;
			}
		}

		if (needUpdate)
		{
			memcpy(state_.srvs, srvs, sizeof(state_.srvs));

			if (bool(flags & BIND_TETURE_FLAGS::PIXEL))
			{
				_context->PSSetShaderResources(0, units, srvs);
				_context->PSSetSamplers(0, units, samplers);
			}
			if (bool(flags & BIND_TETURE_FLAGS::COMPUTE))
			{
				_context->CSSetShaderResources(0, units, srvs);
				_context->CSSetSamplers(0, units, samplers);
			}
		}
	} else
	{
		bool needUpdate = false;

		for(int i = 0; i < units; i++)
		{
			if (state_.srvs[i])
				needUpdate = true;
		}

		if (needUpdate)
		{
			memset(state_.srvs, 0, sizeof(state_.srvs));
			if (bool(flags & BIND_TETURE_FLAGS::PIXEL))
			{
				_context->PSSetShaderResources(0, units, srvs);
				_context->PSSetSamplers(0, units, samplers);
			}
			if (bool(flags & BIND_TETURE_FLAGS::COMPUTE))
			{
				_context->CSSetShaderResources(0, units, srvs);
				_context->CSSetSamplers(0, units, samplers);
			}
		}
	}
}

auto DX11CoreRender::BindUnorderedAccessTextures(int units, Texture** textures) -> void
{
	ID3D11UnorderedAccessView* uavs[16]{};

	if (textures)
	{
		bool needUpdate = false;

		for (int i = 0; i < units; i++)
		{
			Texture* tt = *(textures + i);
			if (tt)
			{
				DX11Texture* t = static_cast<DX11Texture*>(tt->GetCoreTexture());
				uavs[i] = t->uavView();

				if (state_.uavs[i] != uavs[i])
					needUpdate = true;
			}
			else
			{
				if (state_.uavs[i] != nullptr)
					needUpdate = true;
			}
		}

		if (needUpdate)
		{
			memcpy(state_.uavs, uavs, sizeof(state_.uavs));
			_context->CSSetUnorderedAccessViews(0, units, uavs, nullptr);
		}
	}
	else
	{
		bool needUpdate = false;

		for (int i = 0; i < units; i++)
		{
			if (state_.uavs[i])
				needUpdate = true;
		}

		if (needUpdate)
		{
			memset(state_.uavs, 0, sizeof(state_.uavs));
			_context->CSSetUnorderedAccessViews(0, units, uavs, nullptr);
		}
	}

}

auto DX11CoreRender::BindStructuredBuffer(int unit, StructuredBuffer *buffer) -> void
{
	if (buffer)
	{
		ICoreStructuredBuffer *coreBuffer = buffer->GetCoreBuffer();
		DX11StructuredBuffer *dxBuffer = static_cast<DX11StructuredBuffer*>(coreBuffer);
		ID3D11ShaderResourceView *srv = dxBuffer->SRV();

		if (srv != state_.srvs[unit])
		{
			_context->VSSetShaderResources(unit, 1, &srv);
			_context->PSSetShaderResources(unit, 1, &srv);
		}
	} else
	{
		ID3D11ShaderResourceView *srv = nullptr;

		if (state_.srvs[unit])
		{
			state_.srvs[unit] = nullptr;
			_context->VSSetShaderResources(unit, 1, &srv);
			_context->PSSetShaderResources(unit, 1, &srv);
		}
	}
}

auto DX11CoreRender::SetRenderTextures(int units, Texture **textures, Texture *depthTex) -> void
{
	ID3D11RenderTargetView *renderTargetViews[8]{};
	ID3D11DepthStencilView *depthStencilView = nullptr;
	for(int i = 0; i < 8; i++)
		state_.renderTargets[i] = nullptr;
	state_.renderDepth = nullptr;

	if (textures)
	{
		for(int i = 0; i < units; i++)
		{
			Texture *tt = *(textures + i);
			DX11Texture *t = static_cast<DX11Texture*>(tt->GetCoreTexture());
			renderTargetViews[i] = t->rtView();
			state_.renderTargets[i] = tt;
		}
	}

	if (depthTex)
	{
		DX11Texture *dxTex = static_cast<DX11Texture*>(depthTex->GetCoreTexture());
		depthStencilView = dxTex->dsView();
		state_.renderDepth = depthTex;
	}

	_context->OMSetRenderTargets(units, renderTargetViews, depthStencilView);
}

auto DX11CoreRender::SetDepthTest(int enabled) -> void
{
	if (state_.depthStencilDesc.DepthEnable != enabled)
	{
		state_.depthStencilDesc.DepthEnable = enabled;
		state_.depthStencilState = _depthStencilStatePool.FetchState(state_.depthStencilDesc);
		_context->OMSetDepthStencilState(state_.depthStencilState.Get(), 0);
	}
}

auto DX11CoreRender::SetDepthFunc(DEPTH_FUNC func) -> void
{
	D3D11_COMPARISON_FUNC dx = static_cast<D3D11_COMPARISON_FUNC>(func);
	if (state_.depthStencilDesc.DepthFunc != dx)
	{
		state_.depthStencilDesc.DepthFunc = dx;
		state_.depthStencilState = _depthStencilStatePool.FetchState(state_.depthStencilDesc);
		_context->OMSetDepthStencilState(state_.depthStencilState.Get(), 0);
	}
}

auto DX11CoreRender::SetMSAA(int enabled) -> void
{
	if (state_.rasterStateDesc.AntialiasedLineEnable != enabled || state_.rasterStateDesc.MultisampleEnable != enabled)
	{
		state_.rasterStateDesc.AntialiasedLineEnable = enabled;
		state_.rasterStateDesc.MultisampleEnable = enabled;
		state_.rasterState = _rasterizerStatePool.FetchState(state_.rasterStateDesc);
		_context->RSSetState(state_.rasterState.Get());
	}
}

auto DX11CoreRender::SetBlendState(BLEND_FACTOR src, BLEND_FACTOR dest) -> void
{
	D3D11_BLEND_DESC blend_desc{};
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = 0;
	blend_desc.RenderTarget[0].BlendEnable = src != BLEND_FACTOR::NONE && dest != BLEND_FACTOR::NONE;
	blend_desc.RenderTarget[0].SrcBlend = static_cast<D3D11_BLEND>(src);
	blend_desc.RenderTarget[0].DestBlend = static_cast<D3D11_BLEND>(dest);
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	const float zero[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	if (memcmp(&state_.blendStateDesc, &blend_desc, sizeof(D3D11_BLEND_DESC)))
	{
		state_.blendStateDesc = blend_desc;
		state_.blendState = _blendStatePool.FetchState(state_.blendStateDesc);
		_context->OMSetBlendState(state_.blendState.Get(), zero, ~0u);
	}
}

auto DX11CoreRender::SetCullingMode(CULLING_MODE value) -> void
{
	D3D11_CULL_MODE newMode = static_cast<D3D11_CULL_MODE>((int)value);

	if (state_.rasterStateDesc.CullMode == newMode)
		return;

	state_.rasterStateDesc.CullMode = newMode;
	state_.rasterState = _rasterizerStatePool.FetchState(state_.rasterStateDesc);
	_context->RSSetState(state_.rasterState.Get());
}

auto DX11CoreRender::SetFillingMode(FILLING_MODE value) -> void
{
	D3D11_FILL_MODE newMode = static_cast<D3D11_FILL_MODE>((int)value);

	if (state_.rasterStateDesc.FillMode == newMode)
		return;

	state_.rasterStateDesc.FillMode = newMode;
	state_.rasterState = _rasterizerStatePool.FetchState(state_.rasterStateDesc);
	_context->RSSetState(state_.rasterState.Get());
}

auto DX11CoreRender::SetMesh(Mesh * mesh) -> void
{
	if (state_.mesh == mesh)
		return;

	state_.mesh = mesh;

	if (mesh)
	{
		const DX11Mesh *dxMesh = getDX11Mesh(mesh);

		_context->IASetInputLayout(dxMesh->inputLayout());

		ID3D11Buffer *vb = dxMesh->vertexBuffer();
		UINT offset = 0;
		UINT stride = dxMesh->stride();

		_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

		if (dxMesh->indexBuffer())
			_context->IASetIndexBuffer(dxMesh->indexBuffer(), (dxMesh->indexFormat() == MESH_INDEX_FORMAT::INT16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT), 0);
	
		_context->IASetPrimitiveTopology(dxMesh->topology() == VERTEX_TOPOLOGY::TRIANGLES? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST : D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	} else
	{
		_context->IASetInputLayout(nullptr);

		ID3D11Buffer *vb = nullptr;
		UINT offset = 0;
		UINT stride = 0;
		_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

		_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);	
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

auto DX11CoreRender::SetShader(Shader *shader) -> void
{
	if (state_.shader == shader)
		return;

	state_.shader = shader;

	if (shader)
	{
		DX11Shader *dxShader = getDX11Shader(shader);
		dxShader->bind();
	} else
	{
		_context->VSSetShader(nullptr, nullptr, 0);
		_context->PSSetShader(nullptr, nullptr, 0);
		_context->GSSetShader(nullptr, nullptr, 0);
		_context->CSSetShader(nullptr, nullptr, 0);
	}
}

auto DX11CoreRender::Draw(Mesh *mesh, uint instances) -> void
{
	if (!state_.shader)
	{
		LogCritical("DX11CoreRender::Draw(): shader not set");
		return;
	}

	if (state_.mesh != mesh)
		SetMesh(mesh);

	DX11Mesh *dxMesh = getDX11Mesh(mesh);

	if (instances > 1)
	{
		if (dxMesh->indexBuffer())
			_context->DrawIndexedInstanced(dxMesh->indexNumber(), instances, 0, 0, 0);
		else
			_context->DrawInstanced(dxMesh->vertexNumber(), instances, 0, 0);
	}
	else
	{
		if (dxMesh->indexBuffer())
			_context->DrawIndexed(dxMesh->indexNumber(), 0, 0);
		else
			_context->Draw(dxMesh->vertexNumber(), 0);
	}
	stat_.drawCalls++;
	stat_.instances += instances;
	stat_.triangles += instances * dxMesh->vertexNumber() / 3;
}

auto DX11CoreRender::Dispatch(uint x, uint y, uint z) -> void
{
	_context->Dispatch(x, y, z);
}

auto DX11CoreRender::Clear() -> void
{
	// color
	for(int i = 0; i < 8; i++)
	{
		if (state_.renderTargets[i] == nullptr)
			continue;
		DX11Texture *dxRT = static_cast<DX11Texture*>(state_.renderTargets[i]->GetCoreTexture());
		_context->ClearRenderTargetView(dxRT->rtView(), state_.clearColor);
		stat_.clearCalls++;
	}

	// depth
	if (state_.renderDepth != nullptr)
	{
		DX11Texture *dxRT = static_cast<DX11Texture*>(state_.renderDepth->GetCoreTexture());
		_context->ClearDepthStencilView(dxRT->dsView(), D3D11_CLEAR_DEPTH, state_.depthClearColor, state_.stencilClearColor);
		stat_.clearCalls++;
	}
}

auto DX11CoreRender::GetViewport(uint * w, uint * h) -> void
{
	*w = state_.width;
	*h = state_.heigth;
}

auto DX11CoreRender::SetViewport(int newWidth, int newHeight) -> void
{
	if (newWidth < 1 || newHeight < 1)
		return;

	if (surface->w != newWidth || surface->h != newHeight)
	{
		surface->color = nullptr;
		surface->depth = nullptr;

		ThrowIfFailed(surface->swapChain->ResizeBuffers(1, newWidth, newHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

		createCurrentSurface(newWidth, newHeight);

		bindSurface();
	}

	if (state_.width != newWidth || state_.heigth != newHeight)
	{
		state_.width = newWidth;
		state_.heigth = newHeight;

		D3D11_VIEWPORT dxViewport{};
		dxViewport.Height = (float)newHeight;
		dxViewport.Width = (float)newWidth;
		dxViewport.MinDepth = MIN_DEPTH;
		dxViewport.MaxDepth = MAX_DEPTH;

		_context->RSSetViewports(1, &dxViewport);
	}
}

void DX11CoreRender::createCurrentSurface(int w, int h)
{
	ID3D11RenderTargetView *rtv;
	ID3D11Resource *tex;
	
	ThrowIfFailed(surface->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&tex));
	ThrowIfFailed((_device->CreateRenderTargetView(tex, nullptr, &rtv)));

	// Depth resource
	ID3D11Texture2D *depthTex;
	D3D11_TEXTURE2D_DESC descDepth{};
	descDepth.Width = w;
	descDepth.Height = h;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
	descDepth.SampleDesc.Count = MSAASamples_;
	descDepth.SampleDesc.Quality = MSAASamples_ <= 1 ? 0 : MSAAquality(DXGI_FORMAT_R32_TYPELESS, MSAASamples_);
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	ThrowIfFailed(_device->CreateTexture2D(&descDepth, nullptr, &depthTex));

	// Depth DSV
	ID3D11DepthStencilView *dsv;
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = MSAASamples_ > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	ThrowIfFailed(_device->CreateDepthStencilView(depthTex, &descDSV, &dsv));

	// Depth SRV
	ID3D11ShaderResourceView *srv;
	D3D11_SHADER_RESOURCE_VIEW_DESC depthDesc{};
	depthDesc.Format  = DXGI_FORMAT_R32_FLOAT;
	depthDesc.ViewDimension  = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthDesc.Texture2D.MostDetailedMip = 0;
	depthDesc.Texture2D.MipLevels  = -1;
	depthDesc.TextureCube.MipLevels = 1;	
	ThrowIfFailed(_device->CreateShaderResourceView(depthTex, &depthDesc, &srv));

	// Depth sampler
	ID3D11SamplerState* sampler = nullptr;
	D3D11_SAMPLER_DESC sampDesc{};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ThrowIfFailed(_device->CreateSamplerState(&sampDesc, &sampler));

	DX11Texture *dxTex = new DX11Texture(tex, nullptr, nullptr, rtv, nullptr, nullptr, TEXTURE_FORMAT::RGBA8);
	DX11Texture *dxDepthTex = new DX11Texture(depthTex, sampler, srv, nullptr, dsv, nullptr, TEXTURE_FORMAT::D24S8);

	surface->w = w;
	surface->h = h;
	surface->color = std::unique_ptr<Texture>( new Texture(unique_ptr<ICoreTexture>(dxTex)));
	surface->depth = std::unique_ptr<Texture>( new Texture(unique_ptr<ICoreTexture>(dxDepthTex)));
}


