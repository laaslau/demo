#pragma once
#include "common.h"
#include "GuiContainer.h"
#include "d3d11imageRender.h"

#include <memory>

namespace My::Gui
{

	using namespace My::Common;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	class VideoWindow : public IRenderable
	{
		std::string m_name;

		IVideoSource* const m_videoIn;
		ImageRender m_imageRender;

		bool m_visible{};

		void initialPosition();

		IStreamData* m_data{};

	public:

		VideoWindow() = delete;
		VideoWindow(const char* nme, IVideoSource* videoIn);

		bool render() override;

	};

}

