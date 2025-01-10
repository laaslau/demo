#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "imgui.h"

namespace My::Gui
{
	class D3d11Face
	{
	public:
		ID3D11Device* g_pd3dDevice{};
		ID3D11DeviceContext* g_pd3dDeviceContext{};
		IDXGISwapChain* g_pSwapChain{};
		ID3D11RenderTargetView* g_mainRenderTargetView{};

		bool CreateDeviceD3D(HWND hWnd);
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();
	};
}

