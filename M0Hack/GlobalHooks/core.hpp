#pragma once

#include <iostream>
#include <list>
#include <set>
#include "policy.hpp"
#include "detour.hpp"

namespace tella
{
	namespace hook
	{
		template<typename _RTy>
		class _hook_manager : public hook_policy
		{
		public:
			void set_return_info(const _RTy& ret) noexcept(std::is_nothrow_copy_assignable_v<_RTy>) { ReturnData = ret; }
			_NODISCARD const _RTy& get_return_info() const noexcept { return ReturnData; }

		private: _RTy ReturnData{ };
		};

		template<>
		class _hook_manager<void> : public hook_policy { };
	}

	template<bool _MFP, typename _RTy, typename... _Args>
	class hook_manager final : public hook::_hook_manager<_RTy>
	{
	public:
		static constexpr bool is_member_function_pointer_v = _MFP;
		using return_type = _RTy;
		using callback_type = std::function<hook_results(_Args&&...)>;

		hook_manager(void* org_func, void* callback) { Detour = std::make_unique<hook::detour>(org_func, callback); }

		void AddPreHook(hook_order order, const callback_type& callback)
		{
			PreHooks.push_back({ callback, order });
			PreHooks.sort([](const callback_info_t& a, const callback_info_t& b) { return a.Order < b.Order; });
		}

		void AddPostHook(hook_order order, const callback_type& callback)
		{
			PostHooks.push_back({ callback, order });
			PostHooks.sort([](const callback_info_t& a, const callback_info_t& b) { return a.Order < b.Order; });
		}

	public:
		return_type ForceCall(std::add_rvalue_reference_t<_Args>... args)
		{
			using call_type = std::conditional_t<is_member_function_pointer_v, return_type(__thiscall*)(_Args...), return_type(*)(_Args...)>;
			union {
				call_type fn;
				void* ptr;
			} u{ .ptr = Detour->original_function() };
			return (u.fn)(args...);
		}

		/// <summary>	Internal Only </summary>
		hook_results _ProcessPreHooks(std::add_rvalue_reference_t<_Args>... args) const
		{
			hook_results highest;
			for (auto& hooks : PreHooks)
			{
				hook_results cur = hooks.Callback(std::forward<_Args>(args)...);
				if (!highest.test(cur))
				{
					highest |= cur;
					if (highest.test(hook_results_::break_loop))
						break;
				}
			}
			return highest;
		}

		/// <summary>	Internal Only </summary>
		bool _ProcessPostHooks(std::add_rvalue_reference_t<_Args>... args) const
		{
			hook_results highest;
			for (auto& hooks : PostHooks)
			{
				hook_results cur = hooks.Callback(std::forward<_Args>(args)...);
				if (!highest.test(cur))
				{
					highest |= cur;
					if (highest.test(hook_results_::break_loop))
						break;
				}
			}
			return highest.test(hook_results_::changed_return);
		}

	private:
		struct callback_info_t
		{
			callback_type	Callback;
			hook_order		Order;
		};
		using callback_list = std::list<callback_info_t>;

		callback_list				PreHooks;
		callback_list				PostHooks;
		hook::unique_detour			Detour;
	};

	template<typename _RTy, typename... _Args> using hook_manager_mfp = hook_manager<true, _RTy, void*, _Args...>;
	template<typename _RTy, typename... _Args> using hook_manager_sfp = hook_manager<false, _RTy, _Args...>;

	namespace hook
	{
		namespace arg = std::placeholders;
		using ignore_this_t = std::set<void*>;
	}
}