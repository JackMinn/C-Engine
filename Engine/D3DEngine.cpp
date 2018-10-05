#include "stdafx.h"
#include "d3dengine.h"
#include <stdio.h>
#include "EngineUtil.h"

D3DEngine::D3DEngine()
{
	m_Adapter = nullptr;
	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_SwapChain = nullptr;

	m_BackBufferRenderTargetView = nullptr;
	m_DepthStencilBuffer = nullptr;
	m_DepthStencilView = nullptr;

	m_DepthStencilState = nullptr;
	m_RasterState = nullptr;

	for (unsigned int i = 0; i < DEFERRED_BUFFER_COUNT; i++) 
	{
		m_DeferredBuffers[i] = nullptr;
		m_DeferredRenderTargetViews[i] = nullptr;
		m_DeferredShaderResourceViews[i] = nullptr;
	}
}

D3DEngine::D3DEngine(const D3DEngine& other)
{

}

D3DEngine::~D3DEngine()
{

}

bool D3DEngine::Initialize(const uint32_t& screenWidth, const uint32_t& screenHeight, const bool& vsync, HWND const& hwnd, const bool& fullscreen, D3D11_VIEWPORT* const& viewport)
{
	uint32_t numerator, denominator;

	if (!InitializeMonitorProperties(vsync, hwnd, numerator, denominator)) 
	{
		return false;
	}

	if (!InitializeDeviceAndSwapChain(screenWidth, screenHeight, numerator, denominator, hwnd))
	{
		return false;
	}

	if (!InitializeBackBufferAndDepthBuffer(screenWidth, screenHeight))
	{
		return false;
	}

	if (!InitializeRasterizer())
	{
		return false;
	}

	// Set all device context properties.
	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState, 1);
	m_DeviceContext->OMSetRenderTargets(1, &m_BackBufferRenderTargetView, m_DepthStencilView);
	m_DeviceContext->RSSetState(m_RasterState);
	m_DeviceContext->RSSetViewports(1, viewport);

	return true;
}

void D3DEngine::Shutdown()
{
	for (unsigned int i = 0; i < DEFERRED_BUFFER_COUNT; i++)
	{
		SafeRelease(m_DeferredBuffers[i]);
		SafeRelease(m_DeferredRenderTargetViews[i]);
		SafeRelease(m_DeferredShaderResourceViews[i]);
	}

	SafeRelease(m_RasterState);
	SafeRelease(m_DepthStencilState);

	SafeRelease(m_DepthStencilView);
	SafeRelease(m_DepthStencilBuffer);
	SafeRelease(m_BackBufferRenderTargetView);

	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_SwapChain)
	{
		m_SwapChain->SetFullscreenState(false, NULL);
	}
	SafeRelease(m_SwapChain);
	SafeRelease(m_DeviceContext);
	SafeRelease(m_Device);
	SafeRelease(m_Adapter);
}

void D3DEngine::BeginFrame(const DirectX::SimpleMath::Color& clearColor)
{
	// Clear the back buffer and depth buffer.
	m_DeviceContext->ClearRenderTargetView(m_BackBufferRenderTargetView, (float*)&clearColor);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3DEngine::EndFrame()
{
	// Present the back buffer to the screen since rendering is complete.
	if (m_VsyncEnabled)
	{
		// Lock to screen refresh rate.
		m_SwapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_SwapChain->Present(0, 0);
	}
}

void D3DEngine::GetVideoCardInfo(char* cardName, uint32_t& memory)
{
	strcpy_s(cardName, 128, m_VideoCardDescription);
	memory = m_VideoCardMemory;
	return;
}

bool D3DEngine::InitializeMonitorProperties(const bool& vsync, HWND const& hwnd, uint32_t& numeratorOut, uint32_t& denominatorOut)
{
	HRESULT result;
	IDXGIFactory1* factory;
	IDXGIOutput* adapterOutput;
	unsigned int numModes;
	unsigned int numerator = 0, denominator = 0;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;

	m_VsyncEnabled = vsync;

	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	//Later on instead of adapter 0, we may want to find the adapter with the best specs to initialize
	result = factory->EnumAdapters(0, &m_Adapter);
	if (FAILED(result))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	result = m_Adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Create and fill a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	HMONITOR currentMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO activeMonitorInfo;
	activeMonitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(currentMonitor, (LPMONITORINFO)&activeMonitorInfo);
	long activeMonitorWidth = activeMonitorInfo.rcMonitor.right - activeMonitorInfo.rcMonitor.left;
	long activeMonitorHeight = activeMonitorInfo.rcMonitor.bottom - activeMonitorInfo.rcMonitor.top;
	DebugLog("sisi", "Active monitor height is: ", activeMonitorHeight, ", Active monitor width is: ", activeMonitorWidth);

	// Now go through all the display modes and find the one that matches the active monitor width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (uint32_t i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)activeMonitorWidth)
		{
			if (displayModeList[i].Height == (unsigned int)activeMonitorHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	if (numerator == 0 && denominator == 0)
	{
		DebugLog("s", "Did not find a display mode matching the window resolutions");
	}

	result = m_Adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	m_VideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
	sprintf_s(m_VideoCardDescription, "%ws", adapterDesc.Description);
	DebugLog("s", m_VideoCardDescription);

	delete[] displayModeList;
	displayModeList = nullptr;

	SafeRelease(adapterOutput);
	SafeRelease(factory);

	numeratorOut = numerator;
	denominatorOut = denominator;

	return true;
}

bool D3DEngine::InitializeDeviceAndSwapChain(const uint32_t& screenWidth, const uint32_t& screenHeight, const uint32_t& numerator, const uint32_t& denominator, const HWND& hwnd)
{
	HRESULT result;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	DebugLog("si", "Frame buffer width is", screenWidth);
	DebugLog("si", "Frame buffer height is", screenHeight);
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	if (m_VsyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = true;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	//[https://blogs.msdn.microsoft.com/directx/2018/04/09/dxgi-flip-model/]
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	featureLevel = D3D_FEATURE_LEVEL_11_0;
	assert(m_Adapter != nullptr);

	DXGI_ADAPTER_DESC adapterDesc;
	result = m_Adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	DebugLog("si", "Video card memory is: ", (uint64_t)(adapterDesc.DedicatedVideoMemory / 1024 / 1024));

	//either use the default adapter, or use the enumerated adapter (where later we can search for the most powerful GPU)
#if 0
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_SwapChain, &m_Device, NULL, &m_DeviceContext);
#else
	result = D3D11CreateDeviceAndSwapChain(m_Adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_SwapChain, &m_Device, NULL, &m_DeviceContext);
#endif

	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3DEngine::InitializeBackBufferAndDepthBuffer(const uint32_t& screenWidth, const uint32_t& screenHeight)
{
	HRESULT result;
	ID3D11Texture2D* backBuffer;
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	// Get the pointer to the back buffer.
	result = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = m_Device->CreateRenderTargetView(backBuffer, NULL, &m_BackBufferRenderTargetView);
	if (FAILED(result))
	{
		return false;
	}
	SafeRelease(backBuffer);

	//Initialize and fill the depth/stencil buffer description, then create the buffer from the description.
	ZeroMemory(&depthStencilBufferDesc, sizeof(depthStencilBufferDesc));
	depthStencilBufferDesc.Width = screenWidth;
	depthStencilBufferDesc.Height = screenHeight;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0;
	depthStencilBufferDesc.MiscFlags = 0;
	result = m_Device->CreateTexture2D(&depthStencilBufferDesc, NULL, &m_DepthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize and fill the depth/stencil view description, then create the view from the description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	result = m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &depthStencilViewDesc, &m_DepthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize and fill the description of the depth/stencil state.
	ZeroMemory(&depthStencilStateDesc, sizeof(depthStencilStateDesc));

	depthStencilStateDesc.DepthEnable = true;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilStateDesc.StencilEnable = true;
	depthStencilStateDesc.StencilReadMask = 0xFF;
	depthStencilStateDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_Device->CreateDepthStencilState(&depthStencilStateDesc, &m_DepthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3DEngine::InitializeRasterizer()
{
	HRESULT result;
	D3D11_RASTERIZER_DESC rasterDesc;

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	//rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = m_Device->CreateRasterizerState(&rasterDesc, &m_RasterState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}