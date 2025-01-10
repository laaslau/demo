#include "d3d11imageRender.h"
#include "GuiHelper.h"
#include "imgui.h"

namespace My::Gui
{

	std::tuple<ID3D11ShaderResourceView*, uint32_t, uint32_t> ImageRender::getShaderResourceView()
	{

		uint32_t height{};
		uint32_t width{};

		auto renderFun = [this, &width, &height](uint64_t frameId, const uint8_t* image, uint32_t w, uint32_t h)
		{

			width = w;
			height = h;


			if (m_imageID == frameId && m_shaderResourceView)
			{
				return;
			}

			m_imageID = frameId;
			if (m_shaderResourceView)
			{
				m_shaderResourceView->Release();
				m_shaderResourceView = {};
			}


			D3D11_TEXTURE2D_DESC desc{ 0, 0, 1, 1, DXGI_FORMAT_B8G8R8A8_UNORM, {1, 0},  D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };

			desc.Width = width;
			desc.Height = height;
			
			D3D11_SUBRESOURCE_DATA subResource{ image,  desc.Width * 4, 0};

			
			ID3D11Texture2D* pTexture{};
			m_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

			// Create texture view
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = desc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;

			if (pTexture)
			{
				m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_shaderResourceView);
				pTexture->Release();
			}
		};

		if (m_image)
		{
			m_image->frame(renderFun);
		}
		
		return { m_shaderResourceView, width, height };
	}

	ImageRender::~ImageRender()
	{
		if (m_shaderResourceView)
		{
			m_shaderResourceView->Release();
			m_shaderResourceView = {};
		}
	}

	ID3D11ShaderResourceView* ImageRender::render(float left, float top, float imageWidth, float imageHeight)
	{
		if (imageWidth < 0 || imageHeight < 0)
		{
			return nullptr;
		}
		const auto [pointer, width, height] = getShaderResourceView();
		if (pointer)
		{
			const auto corrCoord = adjustedRect(imageWidth, imageHeight, width, height);
			ImVec2 position{ left + corrCoord[0], top + corrCoord[1] };
			ImVec2 size{ corrCoord[2], corrCoord[3] };
			ImGui::SetCursorPos(position);
			ImGui::Image(reinterpret_cast<void*>(pointer), size);

			m_lastRender = { 
				static_cast<float>(width), static_cast<float>(height),
				position.x, position.y,
				size.x, size.y
			};

		}

		ImGui::SetCursorPosY(top + imageHeight);
		return pointer;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RenderThread::onDataApplied(std::unique_lock<std::mutex>& lock)
	{

		ID3D11ShaderResourceView* shaderResourceView{};

		My::Toolbox::unlocker ulck(lock);
		{
			m_desc.Width = m_width;
			m_desc.Height = m_height;


			D3D11_SUBRESOURCE_DATA subResource{ m_image, static_cast<UINT>(m_width * 4), 0 };

			ID3D11Texture2D* pTexture{};
			m_pd3dDevice->CreateTexture2D(&m_desc, &subResource, &pTexture);

			if (!pTexture)
			{
				return false;
			}

			// Create texture view
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = m_desc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = m_desc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			
			m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &shaderResourceView);
			pTexture->Release();

		}

		if (m_views.size() > 2)
		{ 
			auto* srv = m_views.back();
			srv->Release();
			m_views.pop_back();
		}

		m_views.push_back(shaderResourceView);
		return true;

	}

	bool RenderThread::frame(frameCallback callb)
	{
		std::lock_guard lock(m_mtx);
		while (m_views.size() > 1)
		{
			auto* srv = m_views.front();
			srv->Release();
			m_views.pop_front();
		}

		if (m_views.empty())
		{
			return false;
		}

		auto* result = m_views.front();

		ID3D11Resource* resource{};
		ID3D11Texture2D* texture{};

		result->GetResource(&resource);
		resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);


		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		callb(0, reinterpret_cast<uint8_t*>(result), static_cast<uint32_t>(desc.Width), static_cast<uint32_t>(desc.Height));

		texture->Release();
		resource->Release();

		return true;
	}

}