#pragma once

#include <Windows.h>
#include <iostream>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "Offsets.h"


enum class HookCall : char8_t
{
	ReservedFirst,
	VeryEarly,
	Early,
	Any,
	Late,
	VeryLate,
	ReservedLast
};

enum class HookType
{
	SendNetMsg,
};

enum class HookRes : int
{
	Continue			= 0b00001,
	Changed				= 0b00010,
	ChangeReturnValue	= 0b00100,
	BreakImmediate		= 0b01000,
	DontCall			= 0b10000,
};

inline bool operator&(const HookRes& a, const HookRes& b)
{
	using type = std::underlying_type_t<HookRes>;
	return (static_cast<type>(a) & static_cast<type>(b)) != 0;
}
inline HookRes operator|(HookRes a, HookRes b)
{
	using type = std::underlying_type_t<HookRes>;
	return static_cast<HookRes>(static_cast<type>(a) | static_cast<type>(b));
}
inline HookRes& operator|=(HookRes& a, HookRes b)
{
	a = a | b;
	return a;
}


class ISingleVHook
{
	void Init(void* ptr, void* callback);

	int offset{ };
	uintptr_t** base{ };

	uintptr_t original{ };
	uintptr_t actual{ };

public:

	void Register(void* ptr, void* callback) { Init(ptr, callback); }
	void Shutdown();
	void Reconfigure(int offset) { this->offset = offset; }

	template<typename ThisPtr>
		requires std::is_pointer_v<ThisPtr> || std::is_integral_v<ThisPtr> && (sizeof(ThisPtr) == sizeof(uintptr_t)) && std::is_unsigned_v<ThisPtr>
	ISingleVHook(ThisPtr ptr, void* callback, int offset) : offset(offset) { Init(reinterpret_cast<void*>(ptr), callback); }

	explicit ISingleVHook(int offset) : offset(offset) { };
	ISingleVHook() = default;

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

	bool operator==(const ISingleVHook& other) noexcept
	{
		return offset == other.offset && other.base == base;
	}

public:
	ISingleVHook(ISingleVHook&)						= delete;
	ISingleVHook& operator=(const ISingleVHook&)	= delete;
	ISingleVHook(ISingleVHook&&)					= default;
	ISingleVHook& operator=(ISingleVHook&&)			= default;
};

using IUniqueVHook = std::unique_ptr<ISingleVHook>;

template<typename Temp>
static const void* _AddressOfSTDFunctionCallback(const std::function<Temp>& fn)
{
	return ((const void**)(&fn))[1];
}



class MyDummyClass {};
typedef void (MyDummyClass::* DummyFn)();
static inline void* GetAddressOfFunction(DummyFn fn)
{
	return *reinterpret_cast<PVOID*>(&fn);
}





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


#define LINKED_HOOK_PROCESS(HOOK_RET, FNCALL, HOOK_NAME, ...) \
	switch (HOOK_RET) \
	{ \
	case HookRes::UseMyReturnValue: \
	{ \
		FNCALL(HOOK_NAME, __VA_ARGS__); \
		break; \
	} \
	case HookRes::Break: \
	{ \
		return Hook::GetReturnInfo(); \
	} \
	default: \
		Hook::SetReturnInfo(VHOOK_EXECUTE(HOOK_NAME, __VA_ARGS__)); \
	} \
	 \
	return Hook::GetReturnInfo()


#define VHOOK_FORCE_RETURN(NAME, INFO) \
	do { \
		IGlobalEvent::##NAME##::Hook::SetReturnInfo(INFO); \
		return HookRes::Break; \
	} while (NULL)

#define VHOOK_QUERY_RETURN(NAME) IGlobalEvent::##NAME##::Hook::GetReturnInfo()
