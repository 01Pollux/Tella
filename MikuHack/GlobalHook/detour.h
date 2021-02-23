#pragma once

#include "../GlobalHook/defines.h"
#include "../Detour/detours.h"

#include "../Profiler/mprofiler.h"


template<typename Return, typename... Args>
class IGlobalDHook : public M0Hook_Storage<Return>
{
private:
	using PreCallback = std::function<HookRes(Args&&...)>;
	using PostCallback = std::function<HookRes(Args&&...)>;

	struct PreCallbackInfo
	{
		PreCallback callback;
		HookCall order;
	};
	struct PostCallbackInfo
	{
		PostCallback callback;
		HookCall order;
	};

	using PreCBList = std::list<PreCallbackInfo>;
	using PostCBList = std::list<PostCallbackInfo>;

	std::unique_ptr<IDetour>		detour;
	void*							this_ptr;
	std::unique_ptr<PreCBList>		pre_hooks;
	std::unique_ptr<PostCBList>		post_hooks;

public:
	HookRes ProcessPreCallback(Args&&... args)
	{
		if (!pre_hooks)
			return HookRes::Continue;

		HookRes high{ };
		for (PreCBList& hooks = *pre_hooks;
			const PreCallbackInfo & info : hooks)
		{
			HookRes cur = info.callback(std::forward<Args>(args)...);

			if (cur > high)
			{
				high |= cur;
				if (cur & HookRes::BreakImmediate)
					break;
			}
		}
		return high;
	}

	bool ProcessPostCallback(Args&&... args)
	{
		if (!post_hooks)
			return false;

		HookRes high{ };
		for (PostCBList& hooks = *post_hooks;
			const PostCallbackInfo & info : hooks)
		{
			HookRes cur = info.callback(std::forward<Args>(args)...);

			if (cur > high)
			{
				high |= cur;
				if (cur & HookRes::BreakImmediate)
					break;
			}
		}

		return high & HookRes::ChangeReturnValue;
	}

public:
	~IGlobalDHook() = default;
	IGlobalDHook()	= default;

	explicit IGlobalDHook(void* addr, void* callback) noexcept
	{
		detour = std::make_unique<IDetour>(addr, callback);
	}

	void Alloc(void* addr, void* callback)
	{
		detour = std::make_unique<IDetour>(addr, callback);
	}

	bool operator==(const IGlobalDHook<Return, Args...>& other) noexcept
	{
		return detour == other.detour;
	}

	template<typename IFace = void>
	IFace* ThisPtr()
	{
		return reinterpret_cast<IFace*>(this_ptr);
	}

	void UpdateThisPtr(void* this_ptr)
	{
		this->this_ptr = this_ptr;
	}

public:
	void AddPreHook(HookCall order, const PreCallback& callback) noexcept
	{
		if (!pre_hooks) pre_hooks = std::make_unique<PreCBList>();
		pre_hooks->push_back({ callback, order });
		pre_hooks->sort([](const PreCallbackInfo& a, const PreCallbackInfo& b) { return a.order < b.order; });
	}

	void AddPostHook(HookCall order, const PostCallback& callback) noexcept
	{
		if (!post_hooks) post_hooks = std::make_unique<PostCBList>();
		post_hooks->push_back({ callback, order });
		post_hooks->sort([](const PostCallbackInfo& a, const PostCallbackInfo& b) { return a.order < b.order; });
	}

	Return ForceCall(Args&... args) const noexcept
	{
		class IEmptyClass { };
		union {
			Return(IEmptyClass::* fn)(Args...);
			void* ptr;
		} u{ .ptr = detour->OriginalFunction() };
		return (reinterpret_cast<IEmptyClass*>(this_ptr)->*u.fn)(args...);
	}

public:
	static IGlobalDHook<Return, Args...>* MakeHook(const char* name, void* this_ptr, void* callback, int offset)
	{
		using hook_type = IGlobalDHook<Return, Args...>;

		if (auto iter = GetStorage()->find(name); iter != GetStorage()->end())
			return reinterpret_cast<hook_type*>(iter->second);

		hook_type* hook(new hook_type(this_ptr, callback, offset));

		GetStorage()->insert(std::make_pair(name, reinterpret_cast<void*>(hook)));
		return hook;
	}

	static IGlobalDHook<Return, Args...>* MakeHook(const char* name)
	{
		using hook_type = IGlobalDHook<Return, Args...>;

		if (auto iter = GetStorage()->find(name); iter != GetStorage()->end())
			return reinterpret_cast<hook_type*>(iter->second);

		hook_type* hook(new hook_type);

		GetStorage()->insert(std::make_pair(name, reinterpret_cast<void*>(hook)));
		return hook;
	}

	static IGlobalDHook<Return, Args...>* QueryHook(const char* name)
	{
		auto iter = GetStorage()->find(name);
		return iter == GetStorage()->end() ? nullptr : reinterpret_cast<IGlobalDHook<Return, Args...>*>(iter->second);
	}

	static void DeleteHook(IGlobalDHook<Return, Args...>*& hook, const char* name)
	{
		for (auto iter = GetStorage()->begin(); iter != GetStorage()->end(); iter++)
		{
			if (iter->first == name && *reinterpret_cast<IGlobalDHook<Return, Args...>*>(iter->second) == *hook)
			{
				hook->detour = nullptr;
				delete hook;

				GetStorage()->erase(iter);
				break;
			}
		}
	}

	static void DeleteHook(const char* name)
	{
		auto iter = GetStorage()->find(name);
		if (iter != GetStorage()->end())
		{
			IGlobalDHook<Return, Args...>* hook = reinterpret_cast<IGlobalDHook<Return, Args...>*>(iter->second);
			hook->detour = nullptr;
			delete hook;

			GetStorage()->erase(iter);
		}
	}

public:
	IGlobalDHook(IGlobalDHook&) = delete;
	IGlobalDHook& operator=(const IGlobalDHook&) = delete;
	IGlobalDHook(IGlobalDHook&&) = delete;
	IGlobalDHook& operator=(IGlobalDHook&&) = delete;
};



#define DECL_DHOOK_HANDLER(NAME, PROFILER_GROUP, RETURN, ...) \
namespace \
{ \
	template<typename Return, typename... Args> \
	class GH##NAME##_Handler \
	{ \
	public: \
		using MyHookType = IGlobalDHook<Return, Args...>; \
		static inline MyHookType* Handler = nullptr; \
		\
		Return Func(Args... args) \
		{ \
			M0Profiler profile_handler(__FUNCTION__, PROFILER_GROUP); \
			Handler->UpdateThisPtr(reinterpret_cast<void*>(this)); \
			if constexpr (std::is_void_v<Return>) \
			{ \
				if (Handler->ProcessPreCallback(std::forward<Args>(args)...) & HookRes::DontCall) \
					return; \
				\
				Handler->ForceCall(args...); \
				\
				Handler->ProcessPostCallback(std::forward<Args>(args)...); \
			} \
			else \
			{ \
				HookRes hr = Handler->ProcessPreCallback(std::forward<Args>(args)...); \
				Return tmp = Handler->GetReturnInfo(); \
				if (hr & HookRes::DontCall) \
					return tmp; \
				\
				const bool overwritten = (hr & HookRes::ChangeReturnValue) != 0; \
				\
				Return ret = Handler->ForceCall(args...); \
				\
				const Return& actual = overwritten ? tmp : ret; \
				Handler->SetReturnInfo(actual); \
				\
				if (Handler->ProcessPostCallback(std::forward<Args>(args)...) && overwritten) \
					Handler->SetReturnInfo(actual); \
				\
				return Handler->GetReturnInfo(); \
			} \
		} \
	}; \
} \
using GH##NAME##UH = ::GH##NAME##_Handler<RETURN, __VA_ARGS__>; \
using GH##NAME##_Type = IGlobalDHook<RETURN, __VA_ARGS__>


class INetMessage;
namespace IGlobalDHookPolicy
{
#define DECL_HOOK_POLICY(NAME, CUSTOM_NAME, ...) \
	namespace NAME \
	{ \
		using Hook = IGlobalDHook<__VA_ARGS__>; \
		constexpr const char* Name = CUSTOM_NAME; \
	} 

	DECL_HOOK_POLICY(SendNetMsg, "SendNetMsg", bool, INetMessage&, bool, bool);

#undef DECL_HOOK_POLICY
}