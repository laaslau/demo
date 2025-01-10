#pragma once

#include <string>
#include <list>
#include <functional>
#include <optional>

using StringList = std::list<std::string>;

namespace My::Common
{
	class IStreamData
	{
	public:
		virtual const std::string& getStreamName() const = 0;
		virtual void setStreamName(const std::string& name) = 0;

		virtual const StringList& getStreamNames() const = 0;


		virtual const std::optional<int> getFaceDir() const = 0;
		virtual void setFaceDir(std::optional<int> f) = 0;

		virtual const bool getSmiling() const = 0;
		virtual void setSmiling(bool s) = 0;

	};

	class IStreamControl
	{
	public:
		virtual bool run(bool start) = 0;
		virtual bool isRunning() const = 0;
	};

	class IVideoDevice
	{
	public:
		virtual bool start(const std::string& name) = 0;
		virtual void stop() = 0;
	};

	using frameCallback = std::function<void(uint64_t, const uint8_t*, uint32_t, uint32_t)>;
	class IVideoSource
	{
	public:
		virtual ~IVideoSource() {}
		virtual bool frame(frameCallback) = 0;
		virtual float frameRate() = 0;
	};


}
