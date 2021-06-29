
#include "event_listener.hpp"
#include "Profiler/mprofiler.hpp"
#include <vector>

namespace tella
{
	namespace event_listener
	{
		struct event_info
		{
			callback_type	Callback;
			const char*		CustomName;
		};

		class event_wrapper
		{
		public:
			event_wrapper(const char* name, const callback_type& listener, const char* custom_name) : EventName(name)
			{
				Listeners.reserve(8);
				Listeners.emplace_back(listener, custom_name);
			}

			auto find(const callback_type& event_callback)
			{
				return std::find_if(Listeners.begin(), Listeners.end(), [event_callback](const event_info& other) {
					return event_callback.target<callback_type>() == other.Callback.target<callback_type>(); 
					}
				);
			}

			const char* name() const noexcept { return EventName; }

			const char* EventName;
			std::vector<event_info> Listeners;
		};

		static std::vector<event_wrapper> RegisteredEvents;

		callback_data find(const char* event_name)
		{
			auto iter = std::find_if(RegisteredEvents.begin(), RegisteredEvents.end(), [event_name](const event_wrapper& o) { return o.name() == event_name; });
			return iter;
		}

		void insert(const char* event_name, const callback_type& callback, const char* custom_name)
		{
			if (auto info = find(event_name))
				info->Listeners.emplace_back(callback, custom_name);
			else
				RegisteredEvents.emplace_back(event_name, callback, custom_name);
		}

		void remove(const char* event_name, const callback_type& callback)
		{
			if (auto info = find(event_name))
			{
				auto& listeners = info->Listeners;
				auto iter = info->find(callback);
				if (iter != listeners.end())
				{
					listeners.erase(iter);

					if (listeners.empty())
						RegisteredEvents.erase(info.get());
				}
			}
		}

		void destroy(const callback_data& event_info)
		{
			RegisteredEvents.erase(event_info.get());
		}

		void destroy(const char* event_name)
		{
			if (auto pos = find(event_name))
				destroy(pos);
		}

		callback_data::operator bool() const noexcept
		{
			return _Iterator != RegisteredEvents.end();
		}

		void callback_data::operator()(data* info)
		{
			for (const auto& listener : _Iterator->Listeners)
			{
				M0Profiler event_profile(listener.CustomName, M0PROFILER_GROUP::DISPATCH_EVENT_CALLBACK);
				listener.Callback(info);
			}
		}
	}
};