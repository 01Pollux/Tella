#pragma once

#include <chrono>
#include <iostream>
#include <functional>
#include <random>

#define TIMER_NO_FLAGS				0b00
#define TIMER_AUTO_REPEAT			0b01
#define TIMER_EXECUTE_ON_MAP_END	0b10

using TimerCallbackFn = std::function<void(void*)>;

class Timer
{
	using sys_clock = std::chrono::system_clock;
	std::chrono::time_point<sys_clock> last;

	std::default_random_engine uRandomSeed;

public:
	Timer() 
	{ 
		update();  
		uRandomSeed.seed(last.time_since_epoch().count());
	};

	bool has_elapsed(unsigned ms) const { return const_cast<Timer*>(this)->has_elapsed(ms); }
	bool has_elapsed(unsigned ms)
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(sys_clock::now() - last).count() >= ms;
	}

	bool trigger_if_elapsed(unsigned ms) const { return const_cast<Timer*>(this)->trigger_if_elapsed(ms); }
	bool trigger_if_elapsed(unsigned ms)
	{
		if (this->has_elapsed(ms))
		{
			this->update();
			return true;
		}
		return false;
	}

	void update() const { const_cast<Timer*>(this)->update(); }
	void update()
	{
		last = sys_clock::now();
	}

	bool operator==(const Timer& other)
	{
		return other.uRandomSeed == this->uRandomSeed;
	}

public:

	static const Timer CreateFuture(float time, short flags, TimerCallbackFn callback, void* data);
	static void RequestFrame(TimerCallbackFn callback, void* data);
	static void DeleteFuture(Timer* timer, bool execute = false);
	static void RollBack(Timer* timer);
	static void ExecuteFrame();
	static void RunOnLevelShutdown();
};
