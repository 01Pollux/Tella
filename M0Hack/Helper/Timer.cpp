#include "Timer.hpp"
#include "GlobalVars.hpp"

#include <unordered_map>
#include <queue>
#include <memory>


inline bool ITimer::has_elapsed(float sec) const noexcept
{
	return (Interfaces::GlobalVars->CurTime - last_time) >= sec;
}

inline void ITimer::update()
{
	last_time = Interfaces::GlobalVars->CurTime;
}


template<>
struct std::hash<IUTimer>
{
	size_t operator()(const IUTimer& timer) const
	{
		return std::hash<long long>()(timer.seed());
	}
};

namespace tella
{
	namespace timer
	{
		struct FrameRawData
		{
			std::function<void(TimerRawData)> Callback;
			TimerRawData Data{ };

			void ExecuteData()
			{
				this->Callback(this->Data);
			}
		};

		struct TimerData
		{
			std::chrono::milliseconds	Time = 100ms;
			MTimerFlags					Flags;
			FrameRawData				CallbackInfo;

			void ExecuteData()
			{
				CallbackInfo.ExecuteData();
			}
		};

		static std::unordered_map<IUTimer, std::unique_ptr<TimerData>> futures;
		static std::queue<FrameRawData> frame_callbacks;

		const TimerID new_future(std::chrono::milliseconds time, MTimerFlags flags, const TimerCallbackFn& callback, TimerRawData cdata)
		{
			if (time < 100ms)
				time = 100ms;

			auto data(std::make_unique<TimerData>(time, flags, FrameRawData{ callback , cdata }));

			IUTimer timer;
			const TimerID seed = timer.seed();

			futures.insert(std::make_pair(std::move(timer), std::move(data)));

			return seed;
		}

		void request_frame(TimerCallbackFn callback, void* data)
		{
			frame_callbacks.push({ callback, data });
		}

		void delete_future(const TimerID& timer, bool execute)
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

		void rewind(const TimerID& seed)
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

		void _execute_frame()
		{
			static ITimer timer_execute_next;
			if (!timer_execute_next.trigger_if_elapsed(0.1f))
				return;

			for (auto iter = futures.begin(); iter != futures.end(); iter++)
			{
				const IUTimer& timer = iter->first;
				TimerData* data = iter->second.get();

				if (!timer.has_elapsed(data->Time))
					continue;

				data->ExecuteData();
				if (data->Flags.test(TimerFlags::AutoRepeat))
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

		void _on_level_shutdown()
		{
			for (const auto& [timer, data] : futures)
			{
				if (data->Flags.test(TimerFlags::ExecuteOnMapEnd))
					data->ExecuteData();
			}
			futures.clear();
		}
	}
}