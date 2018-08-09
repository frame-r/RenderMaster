#include "DX11CoreRender.h"
#include "Core.h"

using WRL::ComPtr;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

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
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

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
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), &dxgiFactory2);
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
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&swapChain));
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

	// Create a render target view
	ComPtr<ID3D11Texture2D> pBackBuffer;
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &renderTarget);
	if (FAILED(hr))
		return hr;

	context->OMSetRenderTargets(1, &renderTarget, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	context->RSSetViewports(1, &vp);


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

API DX11CoreRender::CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode)
{
	return E_NOTIMPL;
}

API DX11CoreRender::CreateShader(OUT ICoreShader **pShader, const ShaderText *shaderDesc)
{
	return E_NOTIMPL;
}

API DX11CoreRender::SetShader(const ICoreShader* pShader)
{
	return E_NOTIMPL;
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
	return E_NOTIMPL;
}

API DX11CoreRender::Draw(ICoreMesh* mesh)
{
	return E_NOTIMPL;
}

API DX11CoreRender::SetDepthState(int enabled)
{
	return E_NOTIMPL;
}

API DX11CoreRender::SetViewport(uint w, uint h)
{
	return E_NOTIMPL;
}

API DX11CoreRender::GetViewport(OUT uint* wOut, OUT uint* hOut)
{
	return E_NOTIMPL;
}

API DX11CoreRender::Clear()
{
	return E_NOTIMPL;
}

API DX11CoreRender::SwapBuffers()
{
	swapChain->Present(0, 0);
	return S_OK;
}

API DX11CoreRender::Free()
{
	if (context) context->ClearState();
	if (renderTarget) renderTarget->Release();
	if (swapChain) swapChain->Release();
	if (context) context->Release();

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
