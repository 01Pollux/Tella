#include "Timer.h"
#include <unordered_map>

#include <dbg.h>
#include <stack>
#include <queue>

using namespace std;

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
	float time = 0.1f;
	short flag;
	FrameRawData raw;

	void ExecuteData()
	{
		raw.ExecuteData();
	}
};

using PTimerData = unique_ptr<TimerData>;

static unordered_map<Timer*, PTimerData> m_pFutures;
static queue<FrameRawData> frame_callbacks;

const Timer Timer::CreateFuture(float time, short flags, TimerCallbackFn callback, void* cdata)
{
	if (time <= 0.1f)
		time = 0.1f;

	PTimerData data = make_unique<TimerData>();

	data->time = time;
	data->flag = flags;
	data->raw = { callback , cdata };
	
	Timer* timer = new Timer;

	m_pFutures.insert(make_pair(timer, move(data)));

	return *timer;
}


void Timer::RequestFrame(TimerCallbackFn callback, void* data)
{
	frame_callbacks.push({ callback, data });
}


void Timer::ExecuteFrame()
{
	static Timer timer_execute_next;
	if (!timer_execute_next.has_elapsed(100))
		return;

	timer_execute_next.update();

	TimerData* data;
	Timer* timer;

	for (auto iter = m_pFutures.begin(); iter != m_pFutures.end(); iter++)
	{
		timer = iter->first;
		data = iter->second.get();

		if (!timer->has_elapsed(data->time * 1000))
			continue;

		data->ExecuteData();
		if (data->flag & TIMER_AUTO_REPEAT)
			continue;

		delete timer;
		iter = m_pFutures.erase(iter);
	}

	while (!frame_callbacks.empty())
	{
		frame_callbacks.front().ExecuteData();
		frame_callbacks.pop();
	}
}


void Timer::DeleteFuture(Timer* timer, bool execute)
{
	for (auto iter = m_pFutures.begin(); iter != m_pFutures.end(); iter++)
	{
		if (*iter->first == *timer)
		{
			TimerData* pData = iter->second.get();
			if (execute)
				pData->ExecuteData();

			delete iter->first;
			m_pFutures.erase(iter);

			break;
		}
	}
}


void Timer::RollBack(Timer* timer)
{
	for(auto& iter: m_pFutures)
	{
		if (*iter.first == *timer)
		{
			iter.first->update();
			break;
		}
	}
}


[[inline]] void Timer::RunOnLevelShutdown()
{
	TimerData* pData;
	Timer* timer;

	for (auto& entry : m_pFutures)
	{
		pData = entry.second.get();
		if (pData->flag & TIMER_EXECUTE_ON_MAP_END)
			pData->ExecuteData();

		timer = entry.first;
		delete timer;
	}

	m_pFutures.clear();
}