#include "Timer.hpp"
#include "GlobalVars.hpp"

#include <unordered_map>
#include <queue>


inline bool ITimer::has_elapsed(float sec) const noexcept
{
	return (Interfaces::GlobalVars->CurTime - last_time) >= sec;
}

inline void ITimer::update()
{
	last_time = Interfaces::GlobalVars->CurTime;
}


using namespace std;

template<>
struct std::hash<IUTimer>
{
	size_t operator()(const IUTimer& timer) const
	{
		return std::hash<long long>()(timer.seed());
	}
};

struct FrameRawData
{
	function<void(TimerRawData)> callback;
	TimerRawData data{ };

	void ExecuteData()
	{
		this->callback(this->data);
	}
};

struct TimerData
{
	std::chrono::milliseconds time = 100ms;
	TimerFlags flag;
	FrameRawData raw;

	void ExecuteData()
	{
		raw.ExecuteData();
	}
};

namespace ITimerSys
{
	static unordered_map<IUTimer, unique_ptr<TimerData>> futures;
	static queue<FrameRawData> frame_callbacks;

	const TimerID CreateFuture(std::chrono::milliseconds time, TimerFlags flags, const TimerCallbackFn& callback, TimerRawData cdata)
	{
		if (time < 100ms)
			time = 100ms;

		auto data(make_unique<TimerData>());

		data->time = time;
		data->flag = flags;
		data->raw = { callback , cdata };

		IUTimer timer;
		const TimerID seed = timer.seed();

		futures.insert(make_pair(move(timer), move(data)));

		return seed;
	}

	void RequestFrame(TimerCallbackFn callback, void* data)
	{
		frame_callbacks.push({ callback, data });
	}

	void ExecuteFrame()
	{
		static ITimer timer_execute_next;
		if (!timer_execute_next.trigger_if_elapsed(0.1f))
			return;

		for (auto iter = futures.begin(); iter != futures.end(); iter++)
		{
			const IUTimer& timer = iter->first;
			TimerData* data = iter->second.get();

			if (!timer.has_elapsed(data->time))
				continue;

			data->ExecuteData();
			if ((data->flag & TimerFlags::AutoRepeat) == TimerFlags::AutoRepeat)
			{
				timer.update();
				continue;
			}

			iter = futures.erase(iter);
		}

		while (!frame_callbacks.empty())
		{
			frame_callbacks.front().ExecuteData();
			frame_callbacks.pop();
		}
	}

	void DeleteFuture(const TimerID& timer, bool execute)
	{
		auto iter = find_if(
			futures.begin(),
			futures.end(), 
			[&timer](const auto& data)
			{
				return timer == data.first;
			}
		);

		if (iter != futures.end())
		{
			TimerData* data = iter->second.get();
			if (execute)
				data->ExecuteData();

			futures.erase(iter);
		}
	}

	void RewindBack(const TimerID& seed)
	{
		for (const auto& data : futures)
		{
			if (data.first == seed)
			{
				data.first.update();
				break;
			}
		}
	}

	void RunOnLevelShutdown()
	{
		for (const auto& [timer, data] : futures)
		{
			if ((data->flag & TimerFlags::ExecuteOnMapEnd) == TimerFlags::ExecuteOnMapEnd)
				data->ExecuteData();
		}

		futures.clear();
	}
}