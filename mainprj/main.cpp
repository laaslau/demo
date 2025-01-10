#include <iostream>
#include "Toolbox.h"
#include "GuiContainer.h"
#include "GuiVideoWindow.h"


#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <random>
#include <limits>


using namespace std::chrono_literals;
/*
* Class for feeding texture by video data, expected format is B8G8R8A8
* 
* std::vector buffer need to be feeded with frame data, here it done in threadFn.
* This content will passed to renderer in renderer thread.
* Buffer size (in correlation with width and height may be changed)
* 
* 
*/
class MyVideoSource : public My::Common::IVideoSource
{
	uint64_t sequenceNo{};
	std::vector<uint32_t> buffer;
	uint32_t width{ 1920 };
	uint32_t height{ 1080 };

	My::Toolbox::HitRate rate;

	std::thread thrd;
	std::mutex mtx;
	std::condition_variable cond;
	std::atomic_flag exit;

	void threadFn()
	{

		std::mt19937 engine(std::random_device{}());
		std::uniform_int_distribution<unsigned int> dist(static_cast<unsigned int>(0), static_cast<unsigned int>(0xffffffff));

		std::unique_lock lck(mtx);
		while (true)
		{
			cond.wait_for(lck, 333ms, [&exit = exit]() {return exit.test(); });
			if (exit.test())
			{
				break;
			}

			auto num = dist(engine);
			std::ranges::for_each(buffer, [&num](auto& pix) {pix = num; });
			sequenceNo++;
			rate.hit();

		}
	}


public:

	MyVideoSource()
	{
		buffer.resize(width * height);
		thrd = std::thread(&MyVideoSource::threadFn, this);
	}

	~MyVideoSource() 
	{
		if (thrd.joinable())
		{
			exit.test_and_set();
			cond.notify_one();
			thrd.join();
		}
	}

	bool frame(My::Common::frameCallback callback) override
	{
		std::lock_guard lck(mtx);
		callback(sequenceNo, reinterpret_cast<uint8_t*>(buffer.data()), width, height);
		return true;
	}

	virtual float frameRate() { return rate; };
};


MyVideoSource g_mvc;


static int mine()
{
	My::Gui::GuiContainer container("DEMO", "1.0.0.0");
	
	My::Gui::VideoWindow videoW("Video", &g_mvc);
	container.add("video", &videoW);

	while (container.render()) {}
	return 0;
}

int main()
{
	return mine();
}

int APIENTRY wWinMain(_In_ HINSTANCE /*hInstance*/,
	_In_opt_ HINSTANCE /*hPrevInstance*/,
	_In_ LPWSTR    /*lpCmdLine*/,
	_In_ int       /*nCmdShow*/)
{
	return mine();

}


