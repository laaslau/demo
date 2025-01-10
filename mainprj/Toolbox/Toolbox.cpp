#include "Toolbox.h"
#include <string>
#include <filesystem>
#include <Windows.h>
#include <format>
#include <ranges>
#include <algorithm>
#include <format>
#include <iostream>

namespace My::Toolbox
{
	std::filesystem::path exePathA()
	{
		char path[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);
		return { path };
	}

	std::filesystem::path exePath()
	{
		wchar_t buffer[MAX_PATH];
		GetModuleFileNameW(NULL, buffer, MAX_PATH);
		return { buffer };
	}

	std::filesystem::path exeFolder()
	{
		const auto& ep = exePath();
		if (ep.has_parent_path())
		{
			return ep.parent_path();
		}
		return ep;
	}

	std::string settingsStorage()
	{
		auto path = exeFolder();
		path /= "settings_";
		return path.string();
	}

	void writeStringPar(const char* sect, const char* key, const std::string& val)
	{
		WritePrivateProfileStringA(sect, key, val.c_str(), settingsStorage().c_str());
	}

	std::string readStringPar(const char* sect, const char* key, const std::string& valDef)
	{
		char valueStr[256] = {};
		GetPrivateProfileStringA(sect, key, valDef.c_str(), valueStr, sizeof(valueStr), settingsStorage().c_str());
		return { valueStr };
	}


	std::string retrieveSetting(const std::string& section, const std::string& key, const std::string& def)
	{
		static constexpr int SBUFSIZ = 256;
		const auto& fn = settingsStorage();
		char b[SBUFSIZ]{};
		GetPrivateProfileStringA(section.c_str(), key.c_str(), def.c_str(), b, SBUFSIZ - 1, fn.c_str());
		return { b };
	}

	namespace
	{
		std::string rect2str(const Rctngl& r)
		{
			return std::format("{}|{}|{}|{}", r.x, r.y, r.w, r.h);
		}
	}

	void saveMainWinCoord(const Rctngl& r)
	{
		const auto& settings = settingsStorage();

		static constexpr char S[] = "MainWin";
		static constexpr char K[] = "Rect";

		WritePrivateProfileStringA(S, K, rect2str(r).c_str(), settings.c_str());

	}

	const Rctngl retrieveMainWinCoord(const Rctngl& r)
	{
		Rctngl result = r;
		static constexpr char S[] = "MainWin";
		static constexpr char K[] = "Rect";

		const auto& data = retrieveSetting(S, K, rect2str(r));

		int inx = 0;
		std::size_t pos{};
		while (inx < 4)
		{
			auto posNew = data.find('|', pos);
			if (posNew == std::string::npos)
			{
				break;
			}
			try
			{
				int n = std::stoi(data.substr(pos, posNew));
				switch (inx)
				{
				case 0:
					result.x = n;
					break;
				case 1:
					result.y = n;
					break;
				case 2:
					result.w = n;
					break;
				case 3:
					result.h = n;
					break;
				}
			}
			catch (...) {}
			pos = posNew;
			inx++;

		}

		return result;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	using namespace std::chrono_literals;

	HitRate::HitRate()
	{
		restart();
	}

	void HitRate::restart()
	{
		m_last = m_second = m_start = std::chrono::steady_clock::now();
		m_counter = 0;
		m_secondCounter = 0;
		m_perScond = 0;
	}


	HitRate::operator float()
	{
		auto time = std::chrono::steady_clock::now();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(time - m_start);
		std::chrono::milliseconds onesecond(1s);
		float result = static_cast<float>(m_counter) / (static_cast<float>(millis.count()) / static_cast<float>(onesecond.count()));
		if (millis > 3000ms)
		{
			restart();
		}
		return result;
	}

	HitRate::operator uint32_t() const
	{
		return m_perScond;
	}

	void HitRate::hit()
	{

		m_last = std::chrono::steady_clock::now();
		if ((m_last - m_second) > 1000ms)
		{
			m_second = m_last;
			m_perScond = m_secondCounter;
			m_secondCounter = 0;
		}
		m_secondCounter++;
		m_counter++;
		if (m_counter == 0)
		{
			m_start = m_last;
		}

	}

	uint64_t HitRate::check()
	{
		auto tim = std::chrono::steady_clock::now();
		if ((tim - m_last) > 1000ms)
		{
			restart();
		}

		return m_counter;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	void OneFunThrd::threadFn()
	{
		while(true)
		{
			std::unique_lock lock(m_mtx);
			m_condition.wait(lock, [&]() {return m_data.test() || m_exit.test(); });
			if (m_exit.test())
			{
				break;
			}

			m_data.clear();
			if (!onDataApplied(lock))
			{
				break;
			}
		}
	}

	void OneFunThrd::start()
	{
		m_thread = std::thread(&OneFunThrd::threadFn, this);
	}

	void OneFunThrd::stop()
	{
		if (m_thread.joinable())
		{
			m_exit.test_and_set();
			m_condition.notify_one();
			m_thread.join();
		}
	}

	void OneFunThrd::hit()
	{
		m_data.test_and_set();
		m_condition.notify_one();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void Proflr::startRecord()
	{
		m_tp = std::chrono::steady_clock::now();
	}

	void Proflr::stopRecord(std::function<void(const std::string& s)> callback)
	{
		auto tim = std::chrono::steady_clock::now();
		m_average += std::chrono::duration_cast<std::chrono::microseconds>(tim - m_tp);
		m_averageCount++;


		std::chrono::milliseconds passed = std::chrono::duration_cast<std::chrono::milliseconds>(tim - m_rep);
		if (passed > m_profTime)
		{
			auto t = m_average / m_averageCount;
			if (callback)
			{
				callback(std::format("[{}] {}us", m_nme, t.count()));
			}
			m_rep = tim;
		}
	}
}