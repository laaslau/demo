#pragma once
#include "Toolbox.h"
#include "common.h"
#include <list>
#include <d3d11.h>
#include <utility>
#include <tuple>
#include <mutex>

namespace My::Gui
{
	using namespace My::Common;

	struct RenderImageDat
	{
		float width{};
		float height{};
		
		float renderX{};
		float renderY{};

		float renderWidth{};
		float renderHeight{};
	};

	class ImageRender
	{
		ID3D11Device* const m_pd3dDevice{};
		IVideoSource* const m_image{};

		ID3D11ShaderResourceView* m_shaderResourceView{};
		uint64_t m_imageID{};

		std::tuple<ID3D11ShaderResourceView*, uint32_t, uint32_t> getShaderResourceView();

		RenderImageDat m_lastRender;

	public:
		ImageRender() = delete;
		ImageRender(ID3D11Device* dev, IVideoSource* image) : m_pd3dDevice{ dev }, m_image{ image }
		{}
		~ImageRender();
		ID3D11ShaderResourceView* render(float left, float top, float width, float height);
		const RenderImageDat& getLastRenderData() const { return m_lastRender; }
	};


	class RenderThread : public My::Toolbox::OneFunThrd
	{
		uint8_t* m_image{};
		int m_width{};
		int m_height{};


		ID3D11Device* const m_pd3dDevice{};
		std::list<ID3D11ShaderResourceView*> m_views{};

		D3D11_TEXTURE2D_DESC m_desc{ 0, 0, 1, 1, DXGI_FORMAT_B8G8R8A8_UNORM, {1, 0},  D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
		D3D11_SUBRESOURCE_DATA m_subResource{};

		bool onDataApplied(std::unique_lock<std::mutex>& lock) override;
		bool frame(frameCallback callback);
		
	};

}
