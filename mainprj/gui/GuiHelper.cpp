#include "GuiHelper.h"
#include <format>
#include <ranges>

namespace My::Gui
{


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ImVec4 imageCoord()
	{
		auto currpos = ImGui::GetCursorPos();
		float imageWidth = ImGui::GetWindowWidth() - currpos.x - ImGui::GetStyle().ItemSpacing.x;
		float imageHeight = ImGui::GetWindowHeight() - currpos.y - (ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 2);
		return { currpos.x, currpos.y, imageWidth, imageHeight };
	}

	float widthTillEdge()
	{
		return ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x;
	}

	void Bullet(TxtStatus status)
	{
		if (status == TxtStatus::valid)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, green);
		}
		else if (status == TxtStatus::invalid)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, red);
		}
		else if (status == TxtStatus::unknown)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, gray);
		}

		ImGui::Bullet();
		ImGui::PopStyleColor();
	}

	std::array<float, 4> adjustedRect(float availableWidth, float availableHeight, int width, int height)
	{
		float newX = 0;
		float newY = 0;
		float newWidth = availableWidth;
		float newHeight = availableHeight;

		if (availableWidth == 0.0f || availableHeight == 0.0f || width == 0 || height == 0)
		{
			return { newX, newY, newWidth, newHeight };
		}

		float ratioA = availableWidth / availableHeight;
		float ratioB = (static_cast<float>(width) / static_cast<float>(height));

		if (ratioA < ratioB)
		{
			newHeight = availableWidth / ratioB;
			newY = (availableHeight - newHeight) / 2.0f;
		}
		else if (ratioA > ratioB)
		{
			newWidth = availableHeight * ratioB;
			newX = (availableWidth - newWidth) / 2.0f;
		}
		return { newX, newY, newWidth, newHeight };
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	RenderableText::RenderableText(const std::string& id, bool showStatusBullet) : m_id{ id }, m_showStatusBullet{ showStatusBullet }
	{}



	bool RenderableText::render()
	{

		ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data)
		{
			RenderableText* edit = reinterpret_cast<RenderableText*>(data->UserData);
			edit->setStatus(TxtStatus::unknown);
			return 0;
		};

		bool result = ImGui::InputText(m_id.c_str(), m_buffer.data(), MAXBUFFSIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit, callback, this);
		if (!result)
		{
			result = ImGui::IsItemDeactivatedAfterEdit();
		}

		if (m_showStatusBullet)
		{
			ImGui::SameLine(0.0f, 1.0f);
			Bullet(m_status);
		}

		return result;
	}


	void RenderableText::setText(const std::string& text)
	{
		auto count = std::min(text.size(), MAXBUFFSIZE);
		std::copy_n(begin(text), count, m_buffer.data());
		m_buffer.data()[count] = 0x00;
		m_status = TxtStatus::valid;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	RenderableTextCombo::RenderableTextCombo(const std::string& id, const StringList* combo) : 
		RenderableText(id, false), 
		m_combo{combo}
	{

	}

	bool RenderableTextCombo::render()
	{
		
		ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data)
		{
			RenderableText* edit = reinterpret_cast<RenderableText*>(data->UserData);
			edit->setStatus(TxtStatus::unknown);
			return 0;
		};

		ImGui::PushID(m_id.c_str());

		auto limit = ImGui::CalcItemWidth();
		ImGui::SetNextItemWidth(limit - ImGui::GetFrameHeightWithSpacing());


		bool result = ImGui::InputText(m_id.c_str(), m_buffer.data(), MAXBUFFSIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit, callback, this);

		ImVec2 pos = ImGui::GetItemRectMin();
		ImVec2 size = ImGui::GetItemRectSize();

		if (!result)
		{
			result = ImGui::IsItemDeactivatedAfterEdit();
		}
		ImGui::OpenPopupOnItemClick("##combobox"); //Enable right-click
		if (m_combo && !m_combo->empty())
		{
			ImGui::SameLine(0, 0);


			if (ImGui::ArrowButton("##comboarrow", ImGuiDir_Down))
			{
				ImGui::OpenPopup("##combobox");
			}

			pos.y += size.y;
			size.x += ImGui::GetItemRectSize().x;
			size.y += size.y * m_combo->size();

			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(size);

			if (ImGui::BeginPopup("##combobox", ImGuiWindowFlags_::ImGuiWindowFlags_NoMove)) 
			{
				for (const auto& txt : *m_combo)
				{
					if (ImGui::Selectable(txt.c_str()))
					{
						setText(txt);
						result = true;
					}
				}

				ImGui::EndPopup();
			}
		}




		ImGui::PopID();


		return result;

	}

}


