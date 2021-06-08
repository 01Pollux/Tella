#pragma once

#include "GlobalHook/defines.hpp"
#include "Detour/detours.hpp"

#include "Profiler/mprofiler.hpp"


template<typename Return, typename... Args>
class IGlobalDHook : public _M0Hook_Storage<Return>
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

	std::unique_ptr<IDetour>		Detour;
	M0Pointer						ThisPointer;
	std::unique_ptr<PreCBList>		PreHooks;
	std::unique_ptr<PostCBList>		PostHooks;

public:
	HookRes ProcessPreCallback(Args&&... args)
	{
		if (!PreHooks)
			return HookRes::Continue;

		HookRes high{ };
		for (PreCBList& hooks = *PreHooks;
			const PreCallbackInfo & info : hooks)
		{
			HookRes cur = info.callback(std::forward<Args>(args)...);

			if (cur > high)
			{
				high |= cur;
				if (HasBitSet(cur, HookRes::BreakImmediate))
					break;
			}
		}
		return high;
	}

	bool ProcessPostCallback(Args&&... args)
	{
		if (!PostHooks)
			return false;

		HookRes high{ };
		for (PostCBList& hooks = *PostHooks;
			const PostCallbackInfo & info : hooks)
		{
			HookRes cur = info.callback(std::forward<Args>(args)...);

			if (cur > high)
			{
				high |= cur;
				if (HasBitSet(cur, HookRes::BreakImmediate))
					break;
			}
		}

		return HasBitSet(high, HookRes::ChangeReturnValue);
	}

public:
	IGlobalDHook()	= default;

	explicit IGlobalDHook(M0Pointer addr, M0Pointer callback) noexcept
	{
		Alloc(addr, callback);
	}

	void Alloc(M0Pointer addr, M0Pointer callback) noexcept
	{
		Detour = std::make_unique<IDetour>(addr, callback);
	}

	bool operator==(const IGlobalDHook<Return, Args...>& other) const noexcept
	{
		return Detour == other.Detour;
	}

	bool IsActive() const noexcept
	{
		return Detour != nullptr;
	}

	template<typename IFace = void>
	IFace* ThisPtr() const noexcept
	{
		return reinterpret_cast<IFace*>(ThisPointer);
	}

	void _UpdateThisPtr(M0Pointer this_ptr) noexcept
	{
		this->ThisPointer = this_ptr;
	}

public:
	void AddPreHook(HookCall order, const PreCallback& callback) noexcept
	{
		if (!PreHooks) PreHooks = std::make_unique<PreCBList>();

		PreHooks->push_back({ callback, order });
		PreHooks->sort([](const PreCallbackInfo& a, const PreCallbackInfo& b) { return a.order < b.order; });
	}

	void AddPostHook(HookCall order, const PostCallback& callback) noexcept
	{
		if (!PostHooks) PostHooks = std::make_unique<PostCBList>();

		PostHooks->push_back({ callback, order });
		PostHooks->sort([](const PostCallbackInfo& a, const PostCallbackInfo& b) { return a.order < b.order; });
	}

	Return ForceCall(Args&... args) const noexcept
	{
		class IEmptyClass { };

		union {
			Return(IEmptyClass::* fn)(Args...);
			M0Pointer ptr;
		} u{ .ptr = Detour->OriginalFunction() };

		return (reinterpret_cast<IEmptyClass*>(ThisPointer)->*u.fn)(args...);
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
			PROFILE_UFUNCSIG(PROFILER_GROUP); \
			Handler->_UpdateThisPtr(reinterpret_cast<M0Pointer>(this)); \
			\
			if constexpr (std::is_void_v<Return>) \
			{ \
				if (HasBitSet(Handler->ProcessPreCallback(std::forward<Args>(args)...), HookRes::DontCall)) \
					return; \
				\
				Handler->ForceCall(args...); \
				\
				Handler->ProcessPostCallback(std::forward<Args>(args)...); \
			} \
			else \
			{ \
				const HookRes hr = Handler->ProcessPreCallback(std::forward<Args>(args)...); \
				Return tmp = Handler->GetReturnInfo(); \
				if (HasBitSet(hr, HookRes::DontCall)) \
					return tmp; \
				\
				const bool overwritten = HasBitSet(hr, HookRes::ChangeReturnValue); \
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


#define EXPOSE_DHOOK(NAME, CUSTOM_NAME, ADDRESS) \
	static_assert(M0HookManager::Policy::NAME::Name == CUSTOM_NAME, "Hook Name mismatch"); \
	class NAME##_Hook \
	{ \
		GH##NAME##_Type* GH##NAME{ }; \
		 \
	public: \
		void init() \
		{ \
			GH##NAME = M0HookManager::AddHook<GH##NAME##_Type>(CUSTOM_NAME); \
			if (!GH##NAME->IsActive()) \
			{ \
				GH##NAME##UH::Handler = GH##NAME; \
				GH##NAME->Alloc(reinterpret_cast<void*>(ADDRESS), GetAddressOfFunction(&GH##NAME##UH::Func)); \
			} \
		}; \
		void shutdown() \
		{ \
			M0HookManager::RemoveHook(CUSTOM_NAME); \
		} \
		auto operator->() noexcept { return GH##NAME; } \
	}
