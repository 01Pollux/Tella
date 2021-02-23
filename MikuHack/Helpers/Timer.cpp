#include "Timer.h"
#include <unordered_map>

#include <dbg.h>
#include <stack>
#include <queue>


using TimerFlagType = std::underlying_type_t<TimerFlags>;

using namespace std;

template<>
struct std::hash<Timer>
{
	size_t operator()(const Timer& timer) const
	{
		return timer.seed();
	}
};

struct FrameRawData
{
	function<void(void*)> callback;
	void* data{ };

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

using PTimerData = unique_ptr<TimerData>;

static unordered_map<Timer, PTimerData> m_pFutures;
static queue<FrameRawData> frame_callbacks;

const TimerID Timer::CreateFuture(std::chrono::milliseconds time, TimerFlags flags, const TimerCallbackFn& callback, void* cdata)
{
	if (time < 100ms)
		time = 100ms;

	PTimerData data = make_unique<TimerData>();

	data->time = time;
	data->flag = flags;
	data->raw = { callback , cdata };
	
	Timer timer;
	TimerID seed = timer.seed();

	m_pFutures.insert(make_pair(move(timer), move(data)));

	return seed;
}


void Timer::RequestFrame(TimerCallbackFn callback, void* data)
{
	frame_callbacks.push({ callback, data });
}


void Timer::ExecuteFrame()
{
	static Timer timer_execute_next;
	if (!timer_execute_next.has_elapsed(100ms))
		return;

	timer_execute_next.update();

	TimerData* data;

	for (auto iter = m_pFutures.begin(); iter != m_pFutures.end(); iter++)
	{
		const Timer& timer = iter->first;
		data = iter->second.get();

		if (!timer.has_elapsed(data->time))
			continue;

		data->ExecuteData();
		if (static_cast<TimerFlagType>(data->flag) & static_cast<TimerFlagType>(TimerFlags::AutoRepeat))
			continue;

		iter = m_pFutures.erase(iter);
	}

	while (!frame_callbacks.empty())
	{
		frame_callbacks.front().ExecuteData();
		frame_callbacks.pop();
	}
}


void Timer::DeleteFuture(const TimerID& timer, bool execute)
{
	constexpr  size_t sz = sizeof(TimerID);
	for (auto iter = m_pFutures.begin(); iter != m_pFutures.end(); iter++)
	{
		if (iter->first == timer)
		{
			TimerData* pData = iter->second.get();
			if (execute)
				pData->ExecuteData();

			m_pFutures.erase(iter);
			break;
		}
	}
}


void Timer::RewindBack(const TimerID& seed)
{
	for(auto& [timer, data]: m_pFutures)
	{
		if (timer == seed)
		{
			const_cast<Timer&>(timer).update();
			break;
		}
	}
}


[[inline]] void Timer::RunOnLevelShutdown()
{
	for (auto& [timer, pData] : m_pFutures)
	{
		if (static_cast<TimerFlagType>(pData->flag) & static_cast<TimerFlagType>(TimerFlags::ExecuteOnMapEnd))
			pData->ExecuteData();
	}

	m_pFutures.clear();
}