#pragma once
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <map>
#include <Windows.h>
#include "Toolbox.h"
#include "d3d11helper.h"
#include "GuiHelper.h"
#include "imgui.h"


namespace My::Gui
{
	static constexpr ImGuiCond WIN_APPEARANCE = ImGuiCond_FirstUseEver; 

	class IRenderable
	{
	public:
		virtual ~IRenderable() {}
		virtual bool render() = 0;
		virtual void postRender() {}
	};


	class GuiContainer
	{
		std::map<std::string, IRenderable*> m_collection{};
		//std::vector<std::string> m_dockWins;
		bool m_once{};
		bool m_firstUseEver{};
		const std::string m_prodVersion;
		const std::string m_prodName;
		bool init();
		virtual void initDockSpace(ImGuiID dockSpace);
		void cleanUp();

		bool processEvents();
		void doRender();

		void startFrameRender();
		void finishFrameRender();

		void validateRendererBuffers();

		std::thread m_thread{};
		std::mutex m_mtx{};
		std::condition_variable m_condition{};
		std::atomic_flag m_exit = ATOMIC_FLAG_INIT;

		void threadFun();


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// win api stuff
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		HWND m_hwnd{};
		WNDCLASSEXW m_wc{};
		UINT  m_ResizeWidth{};
		UINT  m_ResizeHeight{};
		My::Toolbox::Rctngl m_size{};
		bool m_sizeChanged{};
		void storeWinPlace(HWND h);
		bool isPointVisibleOnDisplay(int x, int y, RECT displayRect) const;
		bool isPointVisible(int pointX, int pointY) const;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT WiPro(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	public:	
		static inline ImFont* m_font{};
		static inline ImFont* m_fontIcon{};

		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// d3d stuff - single instance 
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		static inline D3d11Face m_d3d{};

	public:
		GuiContainer(const std::string& prodName, const std::string& prodVersion);
		~GuiContainer();

		bool render();

		void add(const std::string& name, IRenderable* renderable);
		void remove(const std::string& name);


		void startThread();
		void stopThread();

	};


}

