#pragma once

#include <Windows.h>
#include <iostream>
#include <functional>
#include <mutex>
#include <memory>

#include "Offsets.h"


#define DECL_VHOOK_STATIC(RETURN, CCONV1, CCONV2, NAME, ...) \
	using NAME##_FN = RETURN(CCONV1*)(__VA_ARGS__); \
	static NAME##_FN NAME##_ACTUAL = nullptr; \
	RETURN CCONV2 LHS_##NAME(__VA_ARGS__)

#define DECL_VHOOK_CLASS(RETURN, NAME, ...) \
	class NAME##_CLASS; \
	using NAME##_FN = RETURN(NAME##_CLASS::*)(__VA_ARGS__); \
	class NAME##_CLASS \
	{ \
	public: \
		static inline NAME##_FN NAME##_ACTUAL; \
		RETURN NAME(__VA_ARGS__);\
	}; \
	RETURN NAME##_CLASS::NAME(__VA_ARGS__) \

//

#define VHOOK_EXECUTE_S(NAME, ...) \
	NAME##_ACTUAL(__VA_ARGS__)

#define VHOOK_EXECUTE(NAME, ...) \
	(this->*NAME##_ACTUAL)(__VA_ARGS__)

//

#define VHOOK_LINK_S(SINGLE_HOOK, NAME) \
	{ \
		union  \
		{ \
			NAME##_FN fn; \
			uintptr_t ptr; \
		} u; \
		u.ptr = SINGLE_HOOK->OriginalFunction(); \
		NAME##_ACTUAL = u.fn; \
	} \

#define VHOOK_LINK(SINGLE_HOOK, NAME) \
	{ \
		union  \
		{ \
			NAME##_FN fn; \
			uintptr_t ptr; \
		} u; \
		u.ptr = SINGLE_HOOK->OriginalFunction(); \
		NAME##_CLASS::NAME##_ACTUAL = u.fn; \
	} 

//

#define VHOOK_GET_CALLBACK_S(NAME)	LHS_##NAME

#define VHOOK_GET_CALLBACK(NAME)	GetAddressOfFunction(DummyFn(&NAME##_CLASS::NAME))

//

#define VHOOK_CAST_S(CAST, IFACE)	reinterpret_cast<CAST*>(IFACE)

#define VHOOK_CAST(CAST)			reinterpret_cast<CAST*>(this)

//

#define VHOOK_REG_S(NAME, PTR, OFFSET) std::make_unique<ISingleVHook>(PTR, VHOOK_GET_CALLBACK_S(NAME), OFFSET)

#define VHOOK_REG(NAME, PTR, OFFSET) std::make_unique<ISingleVHook>(PTR, VHOOK_GET_CALLBACK(NAME), OFFSET)


#define LINKED_HOOK_PROCESS(NAME, CALL, ...) \
	using namespace IGlobalEvent::SendNetMsg; \
	 \
	HookRes res = Hook::RunAllHooks(__VA_ARGS__); \
	switch (res) \
	{ \
	case HookRes::UseMyReturnValue: \
	{ \
		CALL; \
		break; \
	} \
	case HookRes::Break: \
	{ \
		return Hook::GetReturnInfo(); \
	} \
	default: \
		Hook::SetReturnInfo(CALL); \
	} \
	 \
	return Hook::GetReturnInfo()


#define VHOOK_FORCE_RETURN(NAME, INFO) \
	do { \
		IGlobalEvent::##NAME##::Hook::SetReturnInfo(INFO); \
		return HookRes::Break; \
	} while (NULL)

#define VHOOK_QUERY_RETURN(NAME) IGlobalEvent::##NAME##::Hook::GetReturnInfo()


class GuardVirtualProtect
{
	size_t size;
	void* const ptr;
	DWORD protection;
	bool is_on = true;
public:
	GuardVirtualProtect(void* ptr, size_t size) : size(size), ptr(ptr)
	{
		if (!VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &protection))
		{
			is_on = false;
			throw std::runtime_error("VirtualProtect Failed");
		}
	}
	~GuardVirtualProtect()
	{
		if (is_on)
			VirtualProtect(ptr, size, protection, &protection);
	}
};

#define BEGIN_VIRTUAL_PROTECT(PTR, SIZE) \
		{ \
			GuardVirtualProtect _PROTECT(reinterpret_cast<void*>(PTR), SIZE)

#define RESTORE_VIRTUAL_PROTECT() \
		}



enum class HookRes: char8_t
{
	Continue,
	NoAction,
	UseMyReturnValue,	// Call Original Function, but use my return value
	Break,				// Dont call original function, and use my return value
};

struct ISearchInfo
{
	using IFace = void*;

	IFace this_ptr;
	int offset;

	bool operator==(const ISearchInfo& o)
	{
		return this_ptr == o.this_ptr && offset == o.offset;
	}
};

class ISingleVHook
{
	void Init(void* ptr, void* callback);

	int offset{ };
	uintptr_t** base;

	uintptr_t original;
	uintptr_t actual;

public:

	void Register(void* ptr, void* callback) { Init(ptr, callback); }
	void Shutdown();
	void Reconfigure(int offset) { this->offset = offset; }

	template<typename ThisPtr>
	requires std::is_pointer_v<ThisPtr> || std::is_integral_v<ThisPtr> && (sizeof(ThisPtr) == sizeof(uintptr_t)) && std::is_unsigned_v<ThisPtr>
		ISingleVHook(ThisPtr ptr, void* callback, int offset) : offset(offset) { Init(reinterpret_cast<void*>(ptr), callback); }
	explicit ISingleVHook(int offset) : offset(offset) { };
	ISingleVHook() { };
	~ISingleVHook()
	{
		Shutdown();
	}

	void* ThisPtr()
	{
		return base;
	}

	constexpr int Offset()
	{
		return offset;
	}

	constexpr uintptr_t OriginalFunction()
	{
		return original;
	}

	operator bool()
	{
		return ThisPtr() && this->offset != 0;
	}
};

using IUniqueVHook = std::unique_ptr<ISingleVHook>;

template<typename Temp>
const void* _AddressOfSTDFunctionCallback(const std::function<Temp>& fn)
{
	return ((const void**)(&fn))[1];
}

/*
template<typename RetType, typename ...Args>
class IFakeVFunc
{
	class __EmptyClass__ {};

	using CallbackPrototype = std::function<RetType(void*, Args...)>;

	int			offset{ };
	void**		thisptr{ };
	uintptr_t	pofn;
	CallbackPrototype fn;

private:

	void Init()
	{
		using namespace std;

		uintptr_t* vfnp = reinterpret_cast<uintptr_t*>(*thisptr) + offset;

		constexpr size_t size_of_dword = sizeof(uintptr_t);

		BEGIN_VIRTUAL_PROTECT(vfnp, size_of_dword);

		pofn = *vfnp;
		*vfnp = reinterpret_cast<const uintptr_t>(_AddressOfSTDFunctionCallback(fn));

		RESTORE_VIRTUAL_PROTECT();
	}

	void Shutdown()
	{
		if (!thisptr)
			return;

		uintptr_t* vtable = reinterpret_cast<uintptr_t*>(*thisptr);

		constexpr size_t size_of_dword = sizeof(uintptr_t);

		BEGIN_VIRTUAL_PROTECT(vtable + offset, size_of_dword);
		vtable[offset] = pofn;
		RESTORE_VIRTUAL_PROTECT();

		thisptr = nullptr;
		fn = nullptr;
	}

public:
	IFakeVFunc(int offset, void* thisptr = nullptr, const CallbackPrototype& callback = nullptr) :
		offset(offset), thisptr(reinterpret_cast<void**>(thisptr)), fn(std::move(callback))
	{
		if (thisptr && fn)
			Init();
	}

	~IFakeVFunc()
	{
		Shutdown();
	}

	RetType CallActual(const Args&... args) const
	{
		union {
			RetType(__EmptyClass__::* fn)(Args...);
			uintptr_t ptr;
		} u;
		u.ptr = pofn;
		return (reinterpret_cast<__EmptyClass__*>(thisptr)->*u.fn)(args...);
	}

	constexpr int Offset() noexcept
	{
		return offset;
	}

	void* ThisPtr() const noexcept
	{
		return reinterpret_cast<void*>(thisptr);
	}
};


#include <cdll_int.h>


namespace IGlobalHook
{
	namespace
	{
		class __GenericClass {};
		typedef void (__GenericClass::* __VoidFunc)();
		template<typename MFNPT = PVOID>
		static inline MFNPT __GetAddress__(__VoidFunc fn)
		{
			return *reinterpret_cast<MFNPT*>(&fn);
		}

		class Hook_Internal
		{
		public:
			using PHOOK = void*;

			ISearchInfo Info;
			size_t		UseCount = 0;
			PHOOK		Hook;

			struct
			{
				std::list<Hook_Internal> Data;
				std::mutex Mutex;

			} static inline HookStorage;

			static inline auto Find(const ISearchInfo& info)
			{
				auto iter = HookStorage.Data.begin();
				for (; iter != HookStorage.Data.end(); iter++)
					if (iter->Info == info)
						break;

				return iter;
			}
		};
	}

#define __IMPL_BASIC_HOOK() \
	private: \
	 \
	using MyFuncType = IFakeVFunc<RetType, Args...>; \
	std::unique_ptr<MyFuncType> Proto; \
	 \
	std::list<MyPreCallback>	PreHooks; \
	std::list<MyPostCallback>	PostHooks; \
	 \
	public:  \
	void AddPreHook(MyPreCallback callback) \
	{ \
		PreHooks.push_back(callback); \
	} \
	void AddPostHook(MyPostCallback callback) \
	{ \
		PostHooks.push_back(callback); \
	} \
	void RemovePreHook(MyPreCallback callback) \
	{ \
		const void* my_callback = _AddressOfSTDFunctionCallback(callback); \
		PreHooks.remove_if(\
			[&my_callback](const MyPreCallback& other) { \
				return _AddressOfSTDFunctionCallback(other) == my_callback; \
			} \
		); \
	} \
	void RemovePostHook(MyPostCallback callback) \
	{ \
		const void* my_callback = _AddressOfSTDFunctionCallback(callback); \
		PostHooks.remove_if( \
			[&my_callback](const MyPostCallback& other) { \
				return _AddressOfSTDFunctionCallback(other) == my_callback; \
			} \
		); \
	} \
	bool QueryEmptyHook() \
	{ \
		return PostHooks.empty() && PreHooks.empty(); \
	} \
	template<typename IFace> \
	IFace* GetThisPtr() \
	{ \
		return reinterpret_cast<IFace*>(Proto->ThisPtr()); \
	} \
	bool Destroy() \
	{ \
		std::lock_guard<std::mutex> lock(Hook_Internal::HookStorage.Mutex); \
		auto iter = Hook_Internal::Find({ GetThisPtr<ISearchInfo::IFace>(), Proto->Offset() }); \
		if (iter != Hook_Internal::HookStorage.Data.end()) \
		{ \
			--iter->UseCount; \
			if (!iter->UseCount) \
			{ \
				auto hook = reinterpret_cast<ILinkedHook<RetType, Args...>*>(iter->Hook); \
				delete hook; \
				Hook_Internal::HookStorage.Data.erase(iter); \
				return true; \
			} \
		} \
		return false; \
	} \
	void QueryHandler() \
	{ \
		printf("Offset: %i\n", Proto->Offset()); \
	} \
	template<class... PlaceHolders>  \
	ILinkedHook<RetType, Args...>(void* thisptr, int offset, PlaceHolders&&... placeholders) \
	{\
		Proto = std::make_unique<MyFuncType>(offset, thisptr, std::bind(&MyType::GlobalProcess, this, thisptr, placeholders...)); \
	} \
	template<class... PlaceHolders> \
	static MyType* NewHook(const ISearchInfo& info, PlaceHolders&&... placeholders) \
	{ \
		auto iter = Hook_Internal::Find(info); \
		if (iter == Hook_Internal::HookStorage.Data.end()) \
		{ \
			MyType* hook = new MyType(info.this_ptr, info.offset, placeholders...); \
			Hook_Internal::HookStorage.Data.push_back({ info, 1, reinterpret_cast<Hook_Internal::PHOOK>(hook) }); \
			printf("Offset: %i\n", info.offset); \
			return hook; \
		} \
		else { \
			printf("Offset: %i\tCount: %i\n", info.offset, iter->UseCount); \
			++iter->UseCount; \
			return reinterpret_cast<MyType*>(iter->Hook); \
		} \
	} \
	static bool QueryHook(const ISearchInfo& info, MyType** Hook = nullptr) \
	{ \
		auto iter = Hook_Internal::Find(info); \
		if (iter != Hook_Internal::HookStorage.Data.end()) \
		{ \
			if (Hook) \
			{ \
				++iter->UseCount; \
				*Hook = reinterpret_cast<MyType*>(iter->Hook); \
			}\
			return true; \
		} \
		return false; \
	} \

	template<typename RetType, typename ...Args>
	class ILinkedHook
	{
	public:
		using MyPreCallback = std::function<HookRes(Args...)>;
		using MyPostCallback = std::function<void(Args...)>;

	private:
		using MyType = ILinkedHook<RetType, Args...>;

		HookRes ProcessPreList(Args&... args)
		{
			HookRes high{ }, cur;
			for (const auto& hook : PreHooks)
			{
				cur = hook(args...);
				if (high < cur)
				{
					high = cur;

					if (cur == HookRes::Break)
						break;
				}
			}
			return high;
		}

		void ProcessPostList(const Args&... args)
		{
			for (const auto& hook : PostHooks)
				hook(args...);
		}

		RetType CallOriginal(const Args&... args)
		{
			return Proto->CallActual(args...);
		}

		RetType GlobalProcess(void*, Args... args)
		{
			HookRes res = this->ProcessPreList(args...);
			switch (res)
			{
			case HookRes::UseMyReturnValue:
			{
				this->CallOriginal(args...);
				break;
			}
			case HookRes::Break:
			{
				return this->GetGlobalReturn();
			}
			default:
				this->SetGlobalReturn(this->CallOriginal(args...));
			}

			this->ProcessPostList(args...);

			return this->GetGlobalReturn();
		}

		RetType						LastRetRes;

	public:

		__IMPL_BASIC_HOOK()

		void SetGlobalReturn(RetType ret)
		{
			LastRetRes = ret;
		}

		RetType GetGlobalReturn()
		{
			return LastRetRes;
		}

	};

	template<typename ...Args>
	class ILinkedHook<void, Args...>
	{
	public:

		using MyPreCallback = std::function<HookRes(Args...)>;
		using MyPostCallback = std::function<void(Args...)>;

	private:

		using RetType = void;
		using MyType = ILinkedHook<RetType, Args...>;

		HookRes ProcessPreList(Args&... args)
		{
			HookRes high{ }, cur;
			for (const auto& hook : PreHooks)
			{
				cur = hook(args...);
				if (high < cur)
				{
					high = cur;
					if (cur == HookRes::Break)
						break;
			}
			}
			return high;
		}

		void ProcessPostList(Args&... args)
		{
			for (const auto& hook : PostHooks)
				hook(args...);
		}

		void CallOriginal(const Args&... args)
		{
			Proto->CallActual(args...);
		}

		void GlobalProcess(void*, Args... args)
		{
	//		PROF_FUNCTION(LDEBUG);
			std::cout << Format("thisptr:", this, " / ", Proto->ThisPtr(), "Offset: ", Proto->Offset(), " Calling") << std::endl;
			HookRes res = this->ProcessPreList(args...);
			if (res == HookRes::Break)
				return;
			this->CallOriginal(args...);
			this->ProcessPostList(args...);
		}

	public:

		__IMPL_BASIC_HOOK()

	};
}



#define DECL_HOOK_NAME(RET, NAME, ...) \
	using LH##NAME##_Type = IGlobalHook::ILinkedHook<RET, __VA_ARGS__>; \
	using LH##NAME##_PreHookFn = LH##NAME##_Type::MyPreCallback; \
	using LH##NAME##_PostHookFn = LH##NAME##_Type::MyPostCallback; \
	static LH##NAME##_Type* LH##NAME{ nullptr }

*/


enum class HookType
{
	Load,
	Unload,

	SendNetMsg,
	CreateMove,

	LevelInit,
	LevelShutdown,

	FrameStageNotify,
	PaintTraverse,
};



class MyDummyClass {};
typedef void (MyDummyClass::* DummyFn)();
static inline void* GetAddressOfFunction(DummyFn fn)
{
	return *reinterpret_cast<PVOID*>(&fn);
}

template<typename RetType, typename = void>
class ILinkedHook_Storage;

template<typename RetType>
class ILinkedHook_Storage<RetType, std::enable_if_t<!std::is_same_v<RetType, void>>>
{
public:
	static inline RetType LastRet{ };

	static RetType GetReturnInfo() noexcept
	{
		return LastRet;
	}

	static void SetReturnInfo(const RetType& ret) noexcept
	{
		LastRet = ret;
	}
};

template<typename RetType>
class ILinkedHook_Storage<RetType, std::enable_if_t<std::is_same_v<RetType, void>>>
{

};

template<HookType HType, typename RetType, typename... Args>
class ILinkedHook : public ILinkedHook_Storage<RetType>
{
	using FuncPrototype = std::function<HookRes(Args...)>;
	using FuncList = std::list<std::function<HookRes(Args...)>>;
	static inline std::unique_ptr<FuncList> Callbacks;
	static inline std::once_flag alloc_once;

public:
	static void Register(const FuncPrototype& func)
	{
		std::call_once(alloc_once, []() {
			Callbacks = std::make_unique<FuncList>();
		});

		Callbacks->push_back(func);
	}

	static void Unregister(const FuncPrototype& func)
	{
		const void* my_callback = _AddressOfSTDFunctionCallback(func);
		Callbacks->remove_if(
			[&my_callback](const FuncPrototype& other) {
				return _AddressOfSTDFunctionCallback(other) == my_callback;
			}
		);
	}

	static void Clear(bool clear_only = false)
	{
		if (clear_only)
			Callbacks->clear();
		else Callbacks = nullptr;
	}

	static HookRes RunAllHooks(const Args&... args)
	{
		HookRes high{ }, cur;
		for (FuncList& callbacks = *Callbacks; const auto& fn : callbacks)
		{
			cur = fn(args...);
			if (cur > high)
			{
				cur = high;
				if (high == HookRes::Break)
					break;
			}
		}
		return high;
	}
};


class INetMessage;
enum ClientFrameStage_t;

namespace IGlobalEvent
{
#define DECL_GLOBAL_EVENT(NAME, OFFSET, HOOK_TYPE, ...) \
	namespace NAME \
	{ \
		constexpr	int Offset = OFFSET; \
		constexpr	HookType Type = HOOK_TYPE; \
		using		Hook = ILinkedHook<Type, __VA_ARGS__>; \
	} \

	DECL_GLOBAL_EVENT(LoadDLL, NULL, HookType::Load, void);
	DECL_GLOBAL_EVENT(UnloadDLL, NULL, HookType::Unload, void);

	DECL_GLOBAL_EVENT(SendNetMsg, NULL, HookType::SendNetMsg, bool, INetMessage&);
	DECL_GLOBAL_EVENT(CreateMove, Offsets::ClientDLL::VTIdx_CreateMove, HookType::CreateMove, void, bool&);

	DECL_GLOBAL_EVENT(LevelInit, Offsets::ClientDLL::VTIdx_LevelInit, HookType::LevelInit, void);
	DECL_GLOBAL_EVENT(LevelShutdown, Offsets::ClientDLL::VTIdx_LevelShutdown, HookType::LevelShutdown, void);

	DECL_GLOBAL_EVENT(FrameStageNotify, Offsets::ClientDLL::VTIdx_FrameStageNotify, HookType::FrameStageNotify, void, ClientFrameStage_t);

	DECL_GLOBAL_EVENT(PaintTraverse, Offsets::ClientDLL::VTIdx_PaintTraverse, HookType::PaintTraverse, void);


#undef DECL_HOOK_PROTO
}