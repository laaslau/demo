#pragma once
#include "common.h"
#include "imgui.h"
#include <string>
#include <array>


namespace My::Gui
{

	ImVec4 imageCoord();
	float widthTillEdge();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum class TxtStatus
	{
		unknown,
		invalid,
		valid
	};

	static constexpr ImVec4 red{ 1.0f, 0.0f, 0.0f, 1.0f };
	static constexpr ImVec4 green{ 0.0f, 1.0f, 0.0f, 1.0f };
	static constexpr ImVec4 blue{ 0.0f, 0.0f, 1.0f, 1.0f };
	static constexpr ImVec4 gray{ 0.4f, 0.4f, 0.4f, 1.0f };
	static constexpr ImVec4 invisible{ 0.0f, 0.0f, 0.0f, 0.0f };
	static constexpr ImVec4 yellow{ 1.0f, 1.0f, 0.0f, 1.0f };

	void Bullet(TxtStatus status);
	std::array<float, 4> adjustedRect(float availableWidth, float availableHeight, int width, int height);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class RenderableText
	{

	protected:
		static constexpr std::size_t MAXBUFFSIZE = 512;

		std::string m_id{ "##" };
		std::array<char, MAXBUFFSIZE + 1> m_buffer{};
		TxtStatus m_status{ TxtStatus::unknown };
		bool m_showStatusBullet{ true };

	public:

		RenderableText& operator = (const RenderableText&) = default;
		bool operator == (const RenderableText& another) const { return strcmp(m_buffer.data(), another.m_buffer.data()) == 0; }
		
		RenderableText(const std::string& id, bool showStatusBullet);
		virtual bool render();

		void setText(const std::string& text);
				
		bool empty() const { return m_buffer[0] == 0x00; }
		
		bool validText() const { return (m_status == TxtStatus::valid && !empty()); }
		std::string getText() const { return std::string{ m_buffer.data() }; }
		TxtStatus getStatus() const { return m_status; }
		void setStatus(TxtStatus status) { m_status = status; }
	};

	class RenderableTextCombo : public RenderableText
	{
		const StringList* m_combo{};

	public:
		RenderableTextCombo(const std::string& id, const StringList* combo);
		virtual bool render() override;

		void setComboDataPtr(const StringList* combo) { m_combo = combo; }

	};
}



