#pragma once

#include "core.hpp"
#include "Profiler/mprofiler.hpp"


namespace tella
{
	namespace hook
	{
		template<bool _MFP, typename _RTy, typename... _Args>
		struct reference
		{
			using type = hook_manager<_MFP, _RTy, _Args...>;

			reference() = default;

			reference(storage::key_type key)
			{
				find(key);
			}

			bool find(storage::key_type key) { _Hook = std::bit_cast<type*>(storage::find(key)); return operator bool(); }

			type* operator->()				noexcept { return _Hook; }
			constexpr operator bool() const noexcept { return _Hook != nullptr; }
			operator type*()				noexcept { return _Hook; }

		private: 
			type* _Hook{ };
		};
	}
}


#define TH_HANDLER(NAME)					tella::hook::NAME##_handler::_Handler

#define TH_HANDLER_TYPE(NAME)				tella::hook::NAME##_handler::handler_type

#define TH_HANDLER_FUNC(NAME)				std::bit_cast<void*>(&tella::hook::NAME##_handler::Func)

#define TH_REFERENCE(NAME)					tella::hook::NAME##_reference

#define TH_REFERENCE_NAME(NAME)				tella::hook::NAME##_ref_name

#define TH_ALLOC_HANDLER(NAME, PTR)			TH_HANDLER(NAME) = new TH_HANDLER_TYPE(NAME)(PTR, TH_HANDLER_FUNC(NAME));

#define TH_ALLOC_HANDLER_ONCE(NAME, KEY, PTR) \
if (!tella::hook::storage::exists(KEY)) \
{ \
	TH_ALLOC_HANDLER(NAME, PTR); \
	tella::hook::storage::insert_hook(KEY, TH_HANDLER(NAME)); \
}

#define TH_ALLOC_HANDLER_ONCE_V(NAME, V)	TH_ALLOC_HANDLER_ONCE(NAME, V, *V)



#define TH_DECL_REFERENCE_MFP(NAME, CUSTOM_NAME, RETURN, ...) \
namespace tella \
{ \
	namespace hook \
	{ \
		static constexpr const char* NAME##_ref_name = CUSTOM_NAME; \
		using NAME##_reference = reference<true, RETURN, void*, __VA_ARGS__>; \
	} \
}

#define TH_PARAMS_MFP(NAME) TH_REFERENCE(NAME)::type* Handler, void* _THISPTR

#define TH_THISPTR(CLASS) std::bit_cast<CLASS*>(_THISPTR)

#define TH_DECL_HANDLER_MFP(NAME, PROFILER_GROUP, RETURN, ...) \
namespace tella \
{ \
	namespace hook \
	{ \
		template<typename _RTy, typename... _Args> \
		class NAME##_handler_mfp_t \
		{ \
		public: \
			using return_type = _RTy; \
			using handler_type = hook_manager_mfp<_RTy, _Args...>; \
 \
			return_type Func(_Args... args) \
			{ \
				M0Profiler hook_profile(PROFILER_GROUP); \
				if constexpr (std::is_void_v<return_type>) \
				{ \
					const hook_results hr = _Handler->_ProcessPreHooks(this, std::forward<_Args>(args)...); \
\
					if (!hr.test(hook_results_::dont_call)) \
						_Handler->ForceCall(this, std::forward<_Args>(args)...); \
 \
					if (!hr.test(hook_results_::skip_post)) \
						_Handler->_ProcessPostHooks(this, std::forward<_Args>(args)...); \
				} \
				else \
				{ \
					const hook_results hr = _Handler->_ProcessPreHooks(this, std::forward<_Args>(args)...); \
					const bool overwritten = hr.test(hook_results_::changed_return); \
\
					return_type ret; \
\
					if (!hr.test(hook_results_::dont_call)) \
						ret = _Handler->ForceCall(this, std::forward<_Args>(args)...); \
\
					if (overwritten) \
						ret = _Handler->get_return_info(); \
					else _Handler->set_return_info(ret); \
\
					if (!hr.test(hook_results_::skip_post)) \
						if (_Handler->_ProcessPostHooks(this, std::forward<_Args>(args)...) && !overwritten) \
							ret = _Handler->get_return_info(); \
\
					return ret; \
				} \
			} \
\
			static inline handler_type* _Handler = nullptr; \
		}; \
\
	using NAME##_handler = NAME##_handler_mfp_t<RETURN, __VA_ARGS__>; \
	} \
}


#define TH_DECL_REFERENCE_VFP(NAME, RETURN, ...) TH_DECL_REFERENCE_MFP(NAME, nullptr, RETURN, __VA_ARGS__)


#define TH_DECL_REFERENCE_SFP(NAME, CUSTOM_NAME, RETURN, ...) \
namespace tella \
{ \
	namespace hook \
	{ \
		using NAME##_reference = reference<false, RETURN, __VA_ARGS__>; \
	} \
}

#define TH_PARAMS_SFP(NAME) TH_REFERENCE(NAME)::type* Handler

#define TH_DECL_HANDLER_SFP(NAME, RETURN, ...) \
namespace tella \
{ \
	namespace hook \
	{ \
		template<typename _RTy, typename... _Args> \
		class NAME##_handler_sfp_t \
		{ \
		public: \
			using return_type = _RTy; \
			using handler_type = hook_manager_sfp<_RTy, _Args...>; \
\
			static return_type Func(_Args... args) \
			{ \
				if constexpr (std::is_void_v<return_type>) \
				{ \
					const hook_results hr = _Handler->_ProcessPreHooks(std::forward<_Args>(args)...); \
\
					if (!hr.test(hook_results_::dont_call)) \
						_Handler->ForceCall(std::forward<_Args>(args)...); \
\
					if (!hr.test(hook_results_::skip_post)) \
						_Handler->_ProcessPostHooks(std::forward<_Args>(args)...); \
				} \
				else \
				{ \
					const hook_results hr = _Handler->_ProcessPreHooks(std::forward<_Args>(args)...); \
					const bool overwritten = hr.test(hook_results_::changed_return); \
					\
					return_type ret; \
					\
					if (!hr.test(hook_results_::dont_call)) \
						ret = _Handler->ForceCall(std::forward<_Args>(args)...); \
\
					if (overwritten) \
						ret = _Handler->get_return_info(); \
					else _Handler->set_return_info(ret); \
\
					if (!hr.test(hook_results_::skip_post)) \
						if (_Handler->_ProcessPostHooks(std::forward<_Args>(args)...) && !overwritten) \
							ret = _Handler->get_return_info(); \
\
					return ret; \
				} \
			} \
\
			static inline handler_type* _Handler = nullptr; \
		}; \
\
		using NAME##_handler = NAME##_handler_sfp_t<RETURN, __VA_ARGS__>; \
	} \
}
