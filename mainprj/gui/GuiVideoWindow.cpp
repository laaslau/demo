#include "GuiVideoWindow.h"
#include "GuiHelper.h"
#include "IconsFontAwesome5.h"
#include "fontdata.h"
#include "d3d11imageRender.h"
#include "imspinner.h"
#include <ranges>
#include <iostream>
namespace My::Gui
{


	VideoWindow::VideoWindow(const char* nme, IVideoSource* videoSource) :
		m_name{ nme },
		m_videoIn{ videoSource },
		m_imageRender{ GuiContainer::m_d3d.g_pd3dDevice, m_videoIn }
	{
	}


	bool VideoWindow::render()
	{
		if (!m_visible)
		{
			initialPosition();
			m_visible = true;
		}

		ImGui::Begin(m_name.c_str());

		const auto& ic = imageCoord();
		m_imageRender.render(ic.x, ic.y, ic.z, ic.w);

		ImGui::Separator();
		auto frameRate = ImGui::GetIO().Framerate;
		float frrt{};
		if (m_videoIn)
		{
			frrt = m_videoIn->frameRate();
		}

		if (frrt)
		{
			ImGui::Text("Render: %.3f ms/frame (%.1f FPS). Video %.1f", 1000.0f / frameRate, frameRate, frrt);
		}
		else
		{
			ImGui::Text("Render: %.3f ms/frame (%.1f FPS).", 1000.0f / frameRate, frameRate);
			ImGui::SameLine();
			ImSpinner::SpinnerFadeBars("##SpinnerFadeBars", 4, 8, { 1.f, 1.f, 1.f, 1.f }, 6.f, 10, true);
		}


		ImGui::End();
		return m_visible;
	}

	void VideoWindow::initialPosition()
	{
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), WIN_APPEARANCE);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 2.f, main_viewport->Size.y / 2.f), WIN_APPEARANCE);
	}


}
