#include "GuiContainer.h"
#include "IconsFontAwesome5.h"
#include "fontdata.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <iostream>
#include <tchar.h>
#include <ranges>
#include <algorithm>
#include <chrono>
#include <filesystem>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


namespace My::Gui
{
	using namespace std::chrono_literals;

	GuiContainer::GuiContainer(const std::string& prodName, const std::string& prodVersion) : m_prodName{ prodName }, m_prodVersion { prodVersion }
	{
		m_firstUseEver = !std::filesystem::exists("imgui.ini");
		init();

	}

	GuiContainer::~GuiContainer()
	{
		cleanUp();
	}

	void GuiContainer::threadFun()
	{
		while (!m_exit.test())
		{
			std::unique_lock lock(m_mtx);
			m_condition.wait_for(lock, 50ms, [exit = &m_exit] { return exit->test(); });
			if (!m_exit.test())
			{
				validateRendererBuffers();
				doRender();
			}
		}
	}

	void GuiContainer::storeWinPlace(HWND h)
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(h, &wp))
		{
			m_size.x = wp.rcNormalPosition.left;
			m_size.y = wp.rcNormalPosition.top;
			m_size.w = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			m_size.h = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			m_sizeChanged = true;
		}

	}

	bool GuiContainer::isPointVisibleOnDisplay(int x, int y, RECT displayRect) const
	{
		if (x >= displayRect.left && x < displayRect.right &&
			y >= displayRect.top && y < displayRect.bottom)
		{
			return true;
		}

		return false;

	}

	bool GuiContainer::isPointVisible(int pointX, int pointY) const
	{

		int numDisplays = GetSystemMetrics(SM_CMONITORS);
		for (int i = 0; i < numDisplays; ++i) {
			MONITORINFO monitorInfo;
			monitorInfo.cbSize = sizeof(monitorInfo);

			if (GetMonitorInfo(MonitorFromPoint(POINT{ pointX, pointY }, MONITOR_DEFAULTTOPRIMARY), &monitorInfo)) {
				RECT displayRect = monitorInfo.rcMonitor;
				bool isVisible = isPointVisibleOnDisplay(pointX, pointY, displayRect);

				if (isVisible) {
					return true;
				}
			}
		}

		return false;
	}


	LRESULT GuiContainer::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		GuiContainer* pThis = reinterpret_cast<GuiContainer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (pThis) {
			// Call the non-static member function
			return pThis->WiPro(hwnd, uMsg, wParam, lParam);
		}
		else {
			// Handle the case where the instance pointer is not yet set
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	LRESULT GuiContainer::WiPro(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_ENTERSIZEMOVE:

			startThread();
			return 0;

		case WM_EXITSIZEMOVE:

			stopThread();
			return 0;

		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED)
			{
				return 0;
			}

			m_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
			m_ResizeHeight = (UINT)HIWORD(lParam);
			[[fallthrough]];

		case WM_MOVE:

			storeWinPlace(hWnd);
			return 0;

		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		case WM_DPICHANGED:
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
			{
				const RECT* suggested_rect = (RECT*)lParam;
				::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			break;
		}
		return ::DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	void GuiContainer::startThread()
	{
		if (m_thread.joinable())
		{
			return;
		}
		m_exit.clear();
		m_thread = std::thread(&GuiContainer::threadFun, this);
	}

	void GuiContainer::stopThread()
	{
		if (!m_thread.joinable())
		{
			return;
		}
		m_exit.test_and_set();
		m_condition.notify_one();
		m_thread.join();
	}

	
	bool GuiContainer::init()
	{
		static constexpr My::Toolbox::Rctngl DFLT__{ 50, 10, 1800, 1180 };

		ImGui::CreateContext();

		// Create application window

		m_wc = { sizeof(m_wc), CS_CLASSDC, GuiContainer::WindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"YRoulette", nullptr };
		::RegisterClassExW(&m_wc);

		My::Toolbox::Rctngl r = retrieveMainWinCoord(DFLT__);
		if (!isPointVisible(r.x, r.y))
		{
			r = DFLT__;
		}

		std::string verA{ m_prodVersion };
		std::string namA{ m_prodName};
		std::wstring verW; verW.append(begin(namA), end(namA)).append( L" - " ).append(begin(verA), end(verA));
		m_hwnd = ::CreateWindowW(m_wc.lpszClassName, verW.c_str(), WS_OVERLAPPEDWINDOW, r.x, r.y, r.w, r.h, nullptr, nullptr, m_wc.hInstance, nullptr);

		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		// Initialize Direct3D
		if (!m_d3d.CreateDeviceD3D(m_hwnd))
		{
			m_d3d.CleanupDeviceD3D();
			::UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
			return 1;
		}

		// Show the window
		::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
		::UpdateWindow(m_hwnd);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

		
		m_font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\arial.ttf", 16.0f);

		float iconFontSize = 25.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

		// merge in icons from Font Awesome
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = iconFontSize;
		icons_config.GlyphOffset = { 0, 5 };
		// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
		icons_config.FontDataOwnedByAtlas = false;
		m_fontIcon = io.Fonts->AddFontFromMemoryTTF(fa_solid_900_ttf, fa_solid_900_ttf_len, iconFontSize, &icons_config, icons_ranges);
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplWin32_Init(m_hwnd);
		ImGui_ImplDX11_Init(m_d3d.g_pd3dDevice, m_d3d.g_pd3dDeviceContext);


		return true;
	}


	void GuiContainer::initDockSpace(ImGuiID dockSpaceId) 
	{
		ImGui::DockBuilderAddNode(dockSpaceId);                // Create a new dock node to use

		ImGuiID dockLeft{};
		ImGuiID dockRight{};
		ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.8f, &dockLeft, &dockRight);

		// left pane
		ImGuiID dockLeftUp{};
		ImGuiID dockLeftDown{};
		ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Up, 0.1f, &dockLeftUp, &dockLeftDown);

		ImGui::DockBuilderDockWindow("Main", dockLeftUp);
		ImGui::DockBuilderDockWindow("Video", dockLeftDown);

		
		// right pane
		ImGuiID dockRightUp{};
		ImGuiID dockRightDown{};
		ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Up, 0.5f, &dockRightUp, &dockRightDown);


		ImGui::DockBuilderDockWindow("Smile", dockRightUp);
		ImGui::DockBuilderDockWindow("Result", dockRightDown);


		ImGui::DockBuilderFinish(dockSpaceId);

	}

	void GuiContainer::cleanUp()
	{
		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		m_d3d.CleanupDeviceD3D();
		::DestroyWindow(m_hwnd);
		::UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
	}

	void GuiContainer::startFrameRender()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void GuiContainer::doRender()
	{
		std::vector<std::string> remove;
		startFrameRender();
		//----------------------------------------------------
		ImGuiID id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		//----------------------------------------------------
		if (!m_once && m_firstUseEver)
		{
			initDockSpace(id);
			m_once = true;
		}

		ImGui::PushFont(m_font);
		std::ranges::for_each(m_collection,
			[&remove](const auto& renderable)
		{
			if (renderable.second->render() == false)
			{
				remove.push_back(renderable.first);
			}
		});

		ImGui::PopFont();

		finishFrameRender();

		std::ranges::for_each(m_collection,
			[&remove](const auto& renderable)
		{
			renderable.second->postRender();
		});

		std::ranges::for_each(remove,
			[this](auto renderable)
		{
			this->remove(renderable);
		});
	}

	bool GuiContainer::render()
	{
		bool render = processEvents();
		if (render)
		{
			doRender();
		}
		return render;
	}

	void GuiContainer::add(const std::string& name, IRenderable* renderable)
	{
		remove(name);
		m_collection[name] = renderable;
	}

	void GuiContainer::remove(const std::string& name)
	{
		m_collection.erase(name);
	}

	void GuiContainer::finishFrameRender()
	{
		static constexpr ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.70f, 1.00f);
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();

		const float clear_color_with_alpha[4] = { clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w };
		m_d3d.g_pd3dDeviceContext->OMSetRenderTargets(1, &m_d3d.g_mainRenderTargetView, nullptr);
		m_d3d.g_pd3dDeviceContext->ClearRenderTargetView(m_d3d.g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		m_d3d.g_pSwapChain->Present(1, 0); // Present with vsync

	}

	void GuiContainer::validateRendererBuffers()
	{
		if (m_ResizeWidth != 0 && m_ResizeHeight != 0)
		{
			m_d3d.CleanupRenderTarget();
			m_d3d.g_pSwapChain->ResizeBuffers(0, m_ResizeWidth, m_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			m_ResizeWidth = m_ResizeHeight = 0;
			m_d3d.CreateRenderTarget();
		}
	}

	bool GuiContainer::processEvents()
	{

		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				return false;
			}
		}

		validateRendererBuffers();
		if (m_sizeChanged)
		{
			m_sizeChanged = false;
			saveMainWinCoord(m_size);
		}
		return true;
	}

	

}
