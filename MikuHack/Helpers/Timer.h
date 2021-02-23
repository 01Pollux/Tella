#pragma once

#include <chrono>
#include <iostream>
#include <functional>
#include <random>

using namespace std::chrono_literals;

enum class TimerFlags: char8_t
{
	Empty,
	AutoRepeat,
	ExecuteOnMapEnd
};

using TimerCallbackFn = std::function<void(void*)>;
using TimerID = std::chrono::system_clock::rep;

class Timer
{
	using sys_clock = std::chrono::system_clock;

	std::chrono::time_point<sys_clock> last;
	std::default_random_engine random_seed;
	TimerID id;

public:
	Timer() noexcept
	{ 
		update();  
		id = last.time_since_epoch().count();
		random_seed.seed(id);
	};

	bool has_elapsed(std::chrono::milliseconds ms) const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(sys_clock::now() - last) >= ms;
	}

	bool trigger_if_elapsed(std::chrono::milliseconds ms)
	{
		if (this->has_elapsed(ms))
		{
			this->update();
			return true;
		}
		return false;
	}

	void update()
	{
		last = sys_clock::now();
	}

	constexpr TimerID seed() const noexcept
	{
		return id;
	}

	bool operator==(const Timer& other) const noexcept
	{
		return other.seed() == seed();
	}

	bool operator==(const TimerID& other) const noexcept
	{
		return other == seed();
	}

public:

	static const TimerID CreateFuture(std::chrono::milliseconds time, TimerFlags flags, const TimerCallbackFn& callback, void* data);
	static void RequestFrame(TimerCallbackFn callback, void* data);
	static void DeleteFuture(const TimerID& timer, bool execute = false);
	static void RewindBack(const TimerID& timer);
	static void ExecuteFrame();
	static void RunOnLevelShutdown();
};
