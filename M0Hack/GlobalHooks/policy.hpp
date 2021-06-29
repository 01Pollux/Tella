#pragma once

#include <memory>
#include <map>
#include <functional>

#include "enum_class.hpp"

namespace tella
{
	namespace hook
	{
		class hook_policy
		{
		public:
			virtual ~hook_policy()	= default;
			hook_policy()			= default;
			hook_policy(const hook_policy&) = delete;	hook_policy& operator=(const hook_policy&)	= delete;
			hook_policy(hook_policy&&)		= delete;	hook_policy& operator=(hook_policy&&)		= delete;
		};

		class storage
		{
		public:
			using key_type = const char*;
			using storage_type = std::map<key_type, std::unique_ptr<hook_policy>>;

			_NODISCARD static hook_policy* find(key_type name)
			{
				auto iter = Hooks.find(name);
				return iter == Hooks.end() ? nullptr : iter->second.get();
			}

			_NODISCARD static bool exists(key_type name)
			{
				return find(name) != nullptr;
			}

			static bool insert_hook(key_type name, hook_policy* data)
			{
				return Hooks.emplace(name, std::unique_ptr<hook_policy>(data)).second;
			}

			static void remove_hook(key_type name)
			{
				Hooks.erase(name);
			}

			static void remove_all()
			{
				Hooks.clear();
			}

			static auto& printf()
			{
				return Hooks;
			}

		private:
			static inline storage_type Hooks;
		};
	}

	enum class hook_order : char8_t
	{
		reserved_first,
		very_early,
		early,
		any,
		late,
		very_late,
		reserved_last
	};

	enum class hook_results_ : int
	{
		ignored,
		changed_params,
		changed_return,
		break_loop,
		dont_call,
		skip_post,
		//rerun_loops
		_Highest_Enum,
	};	using hook_results = bitmask::mask<hook_results_>;
}