#pragma once

#include "../GlobalHook/defines.h"

#include "../Profiler/mprofiler.h"
#include "../Source/Debug.h"


template<typename Return, typename... Args>
class IGlobalVHook : public M0Hook_Storage<Return>
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

	IUniqueVHook					vhook;

	std::unique_ptr<PreCBList>		pre_hooks;
	std::unique_ptr<PostCBList>		post_hooks;

public:
	HookRes ProcessPreCallback(Args&&... args)
	{
		if (!pre_hooks)
			return HookRes::Continue;

		HookRes high{ };
		for (PreCBList& hooks = *pre_hooks;
			const PreCallbackInfo& info : hooks)
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
			const PostCallbackInfo& info : hooks)
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
	~IGlobalVHook()	= default;
	IGlobalVHook()	= default;

	explicit IGlobalVHook(void* this_ptr, void* callback, int offset) noexcept
	{
		vhook = std::make_unique<ISingleVHook>(this_ptr, callback, offset);
	}

	void Alloc(void* this_ptr, void* callback, int offset)
	{
		vhook = std::make_unique<ISingleVHook>(this_ptr, callback, offset);
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
			uintptr_t ptr;
		} u{ .ptr = vhook->OriginalFunction() };
		return (reinterpret_cast<IEmptyClass*>(vhook->ThisPtr())->*u.fn)(args...);
	}

	bool operator==(const IGlobalVHook<Return, Args...>& other) noexcept
	{
		return vhook == other.vhook;
	}

	template<typename IFace = void>
	IFace* ThisPtr()
	{
		return reinterpret_cast<IFace*>(vhook->ThisPtr());
	}

public:
	static IGlobalVHook<Return, Args...>* MakeHook(const char* name, void* this_ptr, void* callback, int offset)
	{
		using hook_type = IGlobalVHook<Return, Args...>;

		if (auto iter = GetStorage()->find(name); iter != GetStorage()->end())
			return reinterpret_cast<hook_type*>(iter->second);

		hook_type* hook(new hook_type(this_ptr, callback, offset));

		GetStorage()->insert(std::make_pair(name, reinterpret_cast<M0HOOK_INSTANCE>(hook)));
		return hook;
	}

	static IGlobalVHook<Return, Args...>* MakeHook(const char* name)
	{
		using hook_type = IGlobalVHook<Return, Args...>;

		if (auto iter = GetStorage()->find(name); iter != GetStorage()->end())
			return reinterpret_cast<hook_type*>(iter->second);

		hook_type* hook(new hook_type);

		GetStorage()->insert(std::make_pair(name, reinterpret_cast<M0HOOK_INSTANCE>(hook)));
		return hook;
	}

	static IGlobalVHook<Return, Args...>* QueryHook(const char* name)
	{
		auto iter = GetStorage()->find(name);
		return iter == GetStorage()->end() ? nullptr : reinterpret_cast<IGlobalVHook<Return, Args...>*>(iter->second);
	}

	static void DeleteHook(IGlobalVHook<Return, Args...>*& hook, const char* name)
	{
		for (auto iter = GetStorage()->begin(); iter != GetStorage()->end(); iter++)
		{
			if (iter->first == name && *reinterpret_cast<IGlobalVHook<Return, Args...>*>(iter->second) == *hook)
			{
				hook->pre_hooks = nullptr;
				hook->post_hooks = nullptr;
				hook->vhook = nullptr;

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
			IGlobalVHook<Return, Args...>* hook = reinterpret_cast<IGlobalVHook<Return, Args...>*>(iter->second);

			hook->pre_hooks = nullptr;
			hook->post_hooks = nullptr;
			hook->vhook = nullptr;

			delete hook;

			GetStorage()->erase(iter);
		}
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
			M0Profiler profile_handler(__FUNCTION__, PROFILER_GROUP); \
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
using GH##NAME##UH = ::GH##NAME##_Handler<RETURN, __VA_ARGS__>


#define LINK_GLOBAL_HOOK(NAME, THISPTR, OFFSET) THISPTR, GetAddressOfFunction(DummyFn(&GH##NAME##UH::Func)), OFFSET

#define EXPOSE_GLOBAL_HOOK(NAME, CUSTOM_NAME, IDENTITY) \
	GH##NAME = GH##NAME##_Type::MakeHook(CUSTOM_NAME); \
	GH##NAME##UH::Handler = GH##NAME; \
	GH##NAME->Alloc(IDENTITY)


namespace IGlobalVHookManager
{
	void Init();
	void Shutdown();
}



class	CUserCmd;
enum	ClientFrameStage_t;
namespace IGlobalVHookPolicy
{
#define DECL_HOOK_POLICY(NAME, CUSTOM_NAME, ...) \
	namespace NAME \
	{ \
		using Hook = IGlobalVHook<__VA_ARGS__>; \
		constexpr const char* Name = CUSTOM_NAME; \
	} 

	DECL_HOOK_POLICY(CreateMove, "CreateMove",				bool, float, CUserCmd*);
	DECL_HOOK_POLICY(PaintTraverse, "PaintTraverse",		void, unsigned int, bool, bool);
	DECL_HOOK_POLICY(FrameStageNotify, "FrameStageNotify",	void, ClientFrameStage_t);
	DECL_HOOK_POLICY(LevelInit, "LevelInit",				void, const char*);
	DECL_HOOK_POLICY(LevelShutdown, "LevelShutdown",		void);

#undef DECL_HOOK_POLICY
}