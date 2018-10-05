#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

constexpr unsigned int DEFERRED_BUFFER_COUNT = 4;

#include <d3d11.h>
#include "SimpleMath.h"

class D3DEngine
{
public:
	D3DEngine();
	D3DEngine(const D3DEngine&);
	~D3DEngine();

	bool Initialize(const uint32_t&, const uint32_t&, const bool&, HWND const&, const bool&, D3D11_VIEWPORT* const&);
	void Shutdown();

	void BeginFrame(const DirectX::SimpleMath::Color&);
	void EndFrame();

	inline ID3D11Device* GetDevice() const{ return m_Device; }
	inline ID3D11DeviceContext* GetDeviceContext() const { return m_DeviceContext; }

	void GetVideoCardInfo(char*, uint32_t&);

	void SetDepthTest(bool depthEnable, D3D11_COMPARISON_FUNC comparisonFunction);
	void SetStencilTest();
	void SetCullModeAndZBias();


private:
	bool m_VsyncEnabled;
	uint32_t m_VideoCardMemory;
	char m_VideoCardDescription[128];

	IDXGIAdapter* m_Adapter = nullptr;
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;
	IDXGISwapChain* m_SwapChain;

	ID3D11RenderTargetView* m_BackBufferRenderTargetView;
	ID3D11Texture2D* m_DepthStencilBuffer;
	ID3D11DepthStencilView* m_DepthStencilView;

	ID3D11DepthStencilState* m_DepthStencilState;
	ID3D11RasterizerState* m_RasterState;

	ID3D11Texture2D* m_DeferredBuffers[DEFERRED_BUFFER_COUNT];
	ID3D11RenderTargetView* m_DeferredRenderTargetViews[DEFERRED_BUFFER_COUNT];
	ID3D11ShaderResourceView* m_DeferredShaderResourceViews[DEFERRED_BUFFER_COUNT];

	bool InitializeMonitorProperties(const bool&, HWND const&, uint32_t&, uint32_t&);
	bool InitializeDeviceAndSwapChain(const uint32_t&, const uint32_t&, const uint32_t&, const uint32_t&, const HWND&);
	bool InitializeBackBufferAndDepthBuffer(const uint32_t&, const uint32_t&);
	bool InitializeRasterizer();
};