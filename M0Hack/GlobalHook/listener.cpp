
#include "listener.hpp"
#include "Profiler/mprofiler.hpp"
#include <vector>

namespace M0EventManager
{
	struct IEventInfo
	{
		EventCallback Callback;
		M0EventCustomName CustomName;
	};

	class IEventWrapper
	{
	public:
		IEventWrapper(M0EventName name, const EventCallback& listener, M0EventCustomName custom_name) : EventName(name)
		{
			Listeners.reserve(8);
			Listeners.emplace_back(listener, custom_name);
		}

		auto find(M0EventCustomName name)
		{
			return std::find_if(Listeners.begin(), Listeners.end(), [name](const IEventInfo& other) { return name == other.CustomName; });
		}

		M0EventName name() const noexcept { return EventName; }

		M0EventName EventName;
		std::vector<IEventInfo> Listeners;
	};

	static std::vector<IEventWrapper> RegisteredEvents;

	void AddListener(M0EventName event_name, const EventCallback& callback, M0EventCustomName custom_name)
	{
		if (auto info = Find(event_name))
			info->Listeners.emplace_back(callback, custom_name);
		else
			RegisteredEvents.emplace_back(event_name, callback, custom_name);
	}

	void RemoveListener(M0EventName event_name, const EventCallback& callback, M0EventCustomName custom_name)
	{
		if (auto info = Find(event_name))
		{
			auto& listeners = info->Listeners;
			auto iter = info->find(custom_name);
			if (iter != listeners.end())
			{
				listeners.erase(iter);

				if (listeners.empty())
					RegisteredEvents.erase(info.get());
			}
		}
	}

	EventData Find(M0EventName event_name)
	{
		auto iter = std::find_if(RegisteredEvents.begin(), RegisteredEvents.end(), [event_name](const IEventWrapper& o) { return o.name() == event_name; });
		return iter;
	}

	void Destroy(const EventData& event_info)
	{
		RegisteredEvents.erase(event_info.get());
	}

	void Destroy(M0EventName event_name)
	{
		if (auto pos = Find(event_name))
			Destroy(pos);
	}

	EventData::operator bool() const noexcept
	{
		return Iterator != RegisteredEvents.end();
	}

	void EventData::operator()(M0EventData* info)
	{
		for (const auto& listener : Iterator->Listeners)
		{
			M0Profiler event_profile(listener.CustomName, M0PROFILER_GROUP::DISPATCH_EVENT_CALLBACK);
			listener.Callback(info);
		}
	}


};