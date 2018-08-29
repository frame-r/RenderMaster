#include "DX11CoreRender.h"
#include "Core.h"
#include "DX11Shader.h"
#include "DX11Mesh.h"
#include <d3dcompiler.h>

using WRL::ComPtr;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)


const char *DX11CoreRender::get_shader_profile(int type)
{
	switch (type)
	{
	case TYPE_VERTEX: return "vs_5_0";
	case TYPE_GEOMETRY: return "gs_5_0";
	case TYPE_FRAGMENT: return "ps_5_0";
	}

	return NULL;
}

const char *DX11CoreRender::get_main_function(int type)
{
	switch (type)
	{
	case TYPE_VERTEX: return "mainVS";
	case TYPE_GEOMETRY: return "mainGS";
	case TYPE_FRAGMENT: return "mainFS";
	}

	return NULL;
}

void DX11CoreRender::_destroy_buffers()
{
	if (renderTargetTex) { renderTargetTex->Release(); renderTargetTex = nullptr; }
	if (renderTargetView) { renderTargetView->Release(); renderTargetView = nullptr; }
	if (depthStencilTex) { depthStencilTex->Release(); depthStencilTex = nullptr; }
	if (depthStencilView) { depthStencilView->Release(); depthStencilView = nullptr; }
}

bool DX11CoreRender::_create_buffers(uint w, uint h)
{
	// Create a render target view
	auto hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&renderTargetTex);
	if (FAILED(hr))
		return hr;

	hr = device->CreateRenderTargetView(renderTargetTex, nullptr, &renderTargetView);
	if (FAILED(hr))
		return hr;

	// Create depth stencil texture
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
	hr = device->CreateTexture2D(&descDepth, nullptr, &depthStencilTex);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(depthStencilTex, &descDSV, &depthStencilView);
	if (FAILED(hr))
		return hr;

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

ID3D11DeviceChild* DX11CoreRender::_create_shader(int type, const char* src)
{
	ID3D11DeviceChild *ret{nullptr};
	ID3DBlob *error_buffer{nullptr};
	ID3DBlob *shader_buffer{nullptr};

	#ifndef NDEBUG
		constexpr UINT flags = (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG);
	#else
		constexpr UINT flags = (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3);
	#endif
	
		auto hr = D3DCompile(src, strlen(src), "", NULL, NULL, get_main_function(type), get_shader_profile(type), flags, 0, &shader_buffer, &error_buffer);

		if (FAILED(hr))
		{
			if (error_buffer)
			{
				LOG_FATAL_FORMATTED("DX11CoreRender::_create_shader() failed to compile shader %s\n", (char*)error_buffer->GetBufferPointer());
				error_buffer->Release();
			}

			if (shader_buffer)
				shader_buffer->Release();
		}else
		{
			unsigned char *data = (unsigned char *)shader_buffer->GetBufferPointer();
			int size = (int)shader_buffer->GetBufferSize();
			HRESULT res = S_FALSE;
			switch (type)
			{
			case TYPE_VERTEX:
				res = device->CreateVertexShader(data, size, NULL, (ID3D11VertexShader**)&ret);
				break;
			case TYPE_GEOMETRY:
				//if (entries)
				//	ret = device->CreateGeometryShaderWithStreamOutput(data, size, entries, num_entries, &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM, NULL, &shaders[type].geometry_shader);
				//else
				res = device->CreateGeometryShader(data, size, NULL, (ID3D11GeometryShader**)&ret);
				//break;
			case TYPE_FRAGMENT:
				res = device->CreatePixelShader(data, size, NULL, (ID3D11PixelShader**)&ret);
				break;
			}

			if (shader_buffer)
				shader_buffer->Release();

			if (ret)
				return ret;
		}
			
	return nullptr;
}

DX11CoreRender::DX11CoreRender()
{
	LOG("DX11CoreRender initalized");
}

DX11CoreRender::~DX11CoreRender()
{
}

API DX11CoreRender::MakeCurrent(const WinHandle* handle)
{
	return E_NOTIMPL;
}

API DX11CoreRender::GetName(OUT const char **pTxt)
{
	*pTxt = "DX11CoreRender";
	return S_OK;
}

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

		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &device, &g_featureLevel, &context);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1, D3D11_SDK_VERSION, &device, &g_featureLevel, &context);
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
		hr = device->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);
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
		hr = dxgiFactory2->CreateSwapChainForHwnd(device, hwnd, &sd, nullptr, nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1.As(&swapChain);
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

		hr = dxgiFactory->CreateSwapChain(device, &sd, &swapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);


	if (FAILED(hr))
		return hr;

	_create_buffers(width, height);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	context->RSSetViewports(1, &vp);

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

	ID3D11RasterizerState *rasterState;
	auto result = device->CreateRasterizerState(&rasterDesc, &rasterState);
	if (FAILED(result))
	{
		return S_FALSE;
	}

	context->RSSetState(rasterState);
	
	context->RSGetState(&rasterState);


	// debug
	D3D11_RASTERIZER_DESC desc;
	rasterState->GetDesc(&desc);


	//ID3D11DepthStencilState *ppDepthStencilState;
	//UINT s;
	//context->OMGetDepthStencilState(&ppDepthStencilState, &s);

	//D3D11_DEPTH_STENCIL_DESC d;
	//ppDepthStencilState->GetDesc(&d);

	return S_OK;
}

API DX11CoreRender::PushStates()
{
	return E_NOTIMPL;
}

API DX11CoreRender::PopStates()
{
	return E_NOTIMPL;
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
	offset += 12;

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
	hr = device->CreateInputLayout(reinterpret_cast<const D3D11_INPUT_ELEMENT_DESC*>(&layout[0]), layout.size(), shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &il);

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

	hr = device->CreateBuffer(&bd, &initData, &vb);

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

		hr = device->CreateBuffer(&bufferDesc, &ibData, &ib);

		if (FAILED(hr))
		{
			vb->Release();
			il->Release();
			return hr;
		}
	}

	DX11Mesh *pDXMesh = new DX11Mesh(vb, ib, il, dataDesc->numberOfVertex, indexDesc->number, indexDesc->format, mode, attribs, bytesWidth);
	*pMesh = pDXMesh;

	return S_OK;
}

API DX11CoreRender::CreateShader(OUT ICoreShader **pShader, const ShaderText *shaderDesc)
{
	ID3D11VertexShader *vs = (ID3D11VertexShader*) _create_shader(TYPE_VERTEX, shaderDesc->pVertText);
	ID3D11PixelShader *fs = (ID3D11PixelShader*) _create_shader(TYPE_FRAGMENT, shaderDesc->pFragText);
	ID3D11GeometryShader *gs{nullptr};
	if (shaderDesc->pGeomText)
		gs = (ID3D11GeometryShader*)_create_shader(TYPE_GEOMETRY, shaderDesc->pGeomText);

	*pShader = new DX11Shader(vs, gs, fs);

	return S_OK;
}

API DX11CoreRender::SetShader(const ICoreShader* pShader)
{
	const DX11Shader *dxShader = static_cast<const DX11Shader*>(pShader);

	context->VSSetShader(dxShader->vs(), nullptr, 0);
	context->GSSetShader(dxShader->gs(), nullptr, 0);
	context->PSSetShader(dxShader->fs(), nullptr, 0);

	return S_OK;
}

API DX11CoreRender::CreateUniformBuffer(OUT IUniformBuffer **pBuffer, uint size)
{
	ID3D11Buffer* ret{nullptr};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	auto hr = device->CreateBuffer(&bd, nullptr, &ret);

	*pBuffer = new DX11ConstantBuffer(ret);
	_pResMan->AddToList((IResource*)*pBuffer);

	return hr;
}

API DX11CoreRender::SetUniform(IUniformBuffer *pBuffer, const void *pData)
{
	const DX11ConstantBuffer *dxBuffer = static_cast<const DX11ConstantBuffer *>(pBuffer);
	context->UpdateSubresource(dxBuffer->nativeBuffer(), 0, nullptr, pData, 0, 0);

	return S_OK;
}

API DX11CoreRender::SetUniformBufferToShader(IUniformBuffer *pBuffer, uint slot)
{
	DX11ConstantBuffer *dxBuffer = static_cast<DX11ConstantBuffer *>(pBuffer);
	ID3D11Buffer * buf = dxBuffer->nativeBuffer();

	context->VSSetConstantBuffers(slot, 1, &buf);
	context->PSSetConstantBuffers(slot, 1, &buf);
	context->GSSetConstantBuffers(slot, 1, &buf);

	return S_OK;
}

API DX11CoreRender::SetUniform(const char* name, const void* pData, const ICoreShader* pShader, SHADER_VARIABLE_TYPE type)
{
	return E_NOTIMPL;
}

API DX11CoreRender::SetUniformArray(const char* name, const void* pData, const ICoreShader* pShader, SHADER_VARIABLE_TYPE type, uint number)
{
	return E_NOTIMPL;
}

API DX11CoreRender::SetMesh(const ICoreMesh* mesh)
{
	const DX11Mesh *dxMesh = static_cast<const DX11Mesh*>(mesh);

	context->IASetInputLayout(dxMesh->inputLayout());

	ID3D11Buffer *vb = dxMesh->vertexBuffer();
	UINT offset = 0;
	UINT stride = dxMesh->stride();
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

	if (dxMesh->indexBuffer())
		context->IASetIndexBuffer(dxMesh->indexBuffer(), (dxMesh->indexFormat() == MESH_INDEX_FORMAT::INT16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT), 0);
	
	context->IASetPrimitiveTopology(dxMesh->topology() == VERTEX_TOPOLOGY::TRIANGLES? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST : D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	return S_OK;
}

API DX11CoreRender::Draw(ICoreMesh* mesh)
{
	const DX11Mesh *dxMesh = static_cast<const DX11Mesh*>(mesh);

	if (dxMesh->indexBuffer())
		context->DrawIndexed(dxMesh->indexNumber(), 0, 0);
	else
		context->Draw(dxMesh->vertexNumber(), 0);

	return S_OK;
}

API DX11CoreRender::SetDepthState(int enabled)
{
	return E_NOTIMPL;
}

API DX11CoreRender::SetViewport(uint w, uint h)
{
	D3D11_VIEWPORT v;

	UINT viewportNumber = 1;
	context->RSGetViewports(&viewportNumber, &v);

	uint new_w = (uint)v.Width;
	uint new_h = (uint)v.Height;

	if (new_w != w || new_h != h)
	{
		v.Height = (float)h;
		v.Width = (float)w;

		_destroy_buffers();
		
		if (FAILED(swapChain->ResizeBuffers(1, new_w, new_h, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)))
			return E_ABORT;

		_create_buffers(new_w, new_h);

		context->RSSetViewports(1, &v);
	}

	return S_OK;
}

API DX11CoreRender::GetViewport(OUT uint* wOut, OUT uint* hOut)
{
	D3D11_VIEWPORT v;
	UINT viewportNumber = 1;
	context->RSGetViewports(&viewportNumber, &v);

	*wOut = (uint)v.Width;
	*hOut = (uint)v.Height;

	return S_OK;
}

API DX11CoreRender::Clear()
{
	static const FLOAT color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	context->ClearRenderTargetView(renderTargetView, color);

	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return S_OK;
}

API DX11CoreRender::SwapBuffers()
{
	swapChain->Present(1, 0);
	return S_OK;
}

API DX11CoreRender::Free()
{
	if (context) context->ClearState();

	_destroy_buffers();
	if (swapChain) swapChain.Reset();
	if (context) context->Release();

	LOG("DX11CoreRender::Free()");

	// debug
	//ID3D11Debug *pDebug;
	//auto hr = g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pDebug));
	//if (pDebug != nullptr)
	//{
	//	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	//	pDebug->Release();
	//}

	if (device) device->Release();

	return S_OK;
}

API DX11ConstantBuffer::Free()
{
	const auto free_ = [&]() -> void { if (buffer) buffer->Release(); };
	standard_free_and_delete(this, free_, _pCore);
	return S_OK;
}

API DX11ConstantBuffer::GetType(OUT RES_TYPE * type)
{
	*type = RES_TYPE::UNIFORM_BUFFER;
	return S_OK;
}
