#pragma once
#include <array>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <filesystem>

namespace My::Toolbox
{

	struct Rctngl
	{
		int x{};
		int y{};
		int w{};
		int h{};
	};

	void saveMainWinCoord(const Rctngl& r);
	const Rctngl retrieveMainWinCoord(const Rctngl& r);

	std::filesystem::path exePathA();
	void writeStringPar(const char* sect, const char* key, const std::string& val);
	std::string readStringPar(const char* sect, const char* key, const std::string& valDef);

	template <typename T>
	void saveProcParam(const char* section, const char* key, const T& value)
	{
		std::ostringstream oss; oss << value;
		writeStringPar(section, key, oss.str());
	}

	template <typename T>
	T retriveProcParam(const char* section, const char* key, T defValue)
	{
		const auto& val = readStringPar(section, key, std::to_string(defValue));
		std::stringstream ss(val);
		T value = defValue;
		ss >> value;
		return value;
	}

	
	class HitRate
	{
		std::chrono::steady_clock::time_point m_last;
		std::chrono::steady_clock::time_point m_start;
		std::chrono::steady_clock::time_point m_second;
		uint64_t m_counter{ 0 };
		uint32_t m_secondCounter{ 0 };
		uint32_t m_perScond{ 0 };

	public:

		HitRate();
		void restart();
		operator float();
		operator uint32_t() const;
		void hit();
		uint64_t check();

	};

	template <typename LOCK>
	class unlocker
	{
		LOCK& lock;
	public:

		unlocker(LOCK& l) : lock(l)
		{
			lock.unlock();
		}

		~unlocker()
		{
			lock.lock();
		}

	};

	class OneFunThrd
	{
	protected:
		mutable std::mutex m_mtx{};

	private:
		std::thread m_thread{};
		std::atomic_flag m_data{};
		std::condition_variable m_condition{};
		std::atomic_flag m_exit{};

		void threadFn();
		virtual bool onDataApplied(std::unique_lock<std::mutex>& lock) = 0;

	protected:

		void hit();

		void start();
		void stop();


	public:
		virtual ~OneFunThrd() {}
	};

	class Proflr
	{
		std::string m_nme{};

		std::chrono::milliseconds m_profTime{};
		std::chrono::steady_clock::time_point m_tp{};

		std::chrono::steady_clock::time_point m_rep{};

		std::chrono::microseconds m_average{};
		uint64_t m_averageCount{};
	public:
		Proflr(std::chrono::milliseconds pt, const std::string& nme = "") : m_profTime{ pt }, m_nme{ nme }
		{
			m_rep = std::chrono::steady_clock::now();
		}

		void startRecord();
		void stopRecord(std::function<void(const std::string& s)> callback = nullptr);


	};


	template <uint32_t LIMIT>
	class Countdown
	{
		uint32_t m_counter{ LIMIT };

	public:

		bool hit()
		{
			auto newCount = std::exchange(m_counter, m_counter - 1);
			if (newCount == 0)
			{
				m_counter = LIMIT;
				return true;
			}
			return false;
		}
	};

}