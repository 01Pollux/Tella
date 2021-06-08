#pragma once

#include "defines.hpp"

#include "Profiler/mprofiler.hpp"
#include "Helper/Debug.hpp"


template<typename Return, typename... Args>
class IGlobalVHook : public _M0Hook_Storage<Return>
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

	IUniqueVHook					VHook;

	std::unique_ptr<PreCBList>		PreHooks;
	std::unique_ptr<PostCBList>		PostHooks;

public:
	HookRes ProcessPreCallback(Args&&... args)
	{
		if (!PreHooks)
			return HookRes::Continue;

		HookRes high{ };
		for (PreCBList& hooks = *PreHooks;
			const PreCallbackInfo& info : hooks)
		{
			const HookRes cur = info.callback(std::forward<Args>(args)...);

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
			const PostCallbackInfo& info : hooks)
		{
			const HookRes cur = info.callback(std::forward<Args>(args)...);

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
	IGlobalVHook()	= default;

	explicit IGlobalVHook(M0Pointer this_ptr, M0Pointer callback, int offset) noexcept
	{
		VHook = std::make_unique<ISingleVHook>(offset, this_ptr, callback);
	}

	void Alloc(M0Pointer this_ptr, M0Pointer callback, int offset)
	{
		VHook = std::make_unique<ISingleVHook>(offset, this_ptr, callback);
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

	Return ForceCall(M0Pointer this_ptr, Args&... args) const noexcept
	{
		class IEmptyClass { };

		union {
			Return(IEmptyClass::* fn)(Args...);
			M0Pointer ptr;
		} u{ .ptr = VHook->OriginalFunction() };

		return (reinterpret_cast<IEmptyClass*>(this_ptr)->*u.fn)(args...);
	}

	bool operator==(const IGlobalVHook<Return, Args...>& other) const noexcept
	{
		return VHook == other.VHook;
	}

	bool IsActive() const noexcept
	{
		return VHook != nullptr;
	}

	template<typename IFace = void>
	IFace* ThisPtr() const noexcept
	{
		return reinterpret_cast<IFace*>(VHook->ThisPtr());
	}

public:
	IGlobalVHook(IGlobalVHook&) = delete;
	IGlobalVHook& operator=(const IGlobalVHook&) = delete;
	IGlobalVHook(IGlobalVHook&&) = delete;
	IGlobalVHook& operator=(IGlobalVHook&&) = delete;
};

#define DECL_VHOOK_HANDLER(NAME, PROFILER_GROUP, RETURN, ...) \
using GH##NAME##_Type = IGlobalVHook<RETURN, __VA_ARGS__>; \
namespace \
{ \
	template<typename Return, typename... Args> \
	class GH##NAME##_Handler \
	{ \
	public: \
		static inline GH##NAME##_Type* Handler = nullptr; \
		\
		Return Func(Args... args) \
		{ \
			PROFILE_UFUNCSIG(PROFILER_GROUP); \
			if constexpr (std::is_void_v<Return>) \
			{ \
				if (HasBitSet(Handler->ProcessPreCallback(std::forward<Args>(args)...), HookRes::DontCall)) \
					return; \
				\
				Handler->ForceCall(reinterpret_cast<M0Pointer>(this), args...); \
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
				Return ret = Handler->ForceCall(reinterpret_cast<M0Pointer>(this), args...); \
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
using GH##NAME##UH = ::GH##NAME##_Handler<RETURN, __VA_ARGS__>

#define LINK_GLOBAL_HOOK(NAME, THISPTR, OFFSET) THISPTR, GetAddressOfFunction(DummyFn(&GH##NAME##UH::Func)), OFFSET


#define EXPOSE_VHOOK(NAME, CUSTOM_NAME, THIS_PTR, OFFSET) \
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
				GH##NAME->Alloc(THIS_PTR, GetAddressOfFunction(&GH##NAME##UH::Func), OFFSET); \
			} \
		}; \
		void shutdown() \
		{ \
			M0HookManager::RemoveHook(CUSTOM_NAME); \
		} \
		auto operator->() noexcept { return GH##NAME; } \
	}
