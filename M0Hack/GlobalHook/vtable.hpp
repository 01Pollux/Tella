#pragma once

#include <functional>
#include <memory>

#include "Library/Lib.hpp"
#include "Helper/Offsets.hpp"
#include "Helper/EnumClass.hpp"


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

enum class HookRes : int
{
	Continue			= 0b00001,
	Changed				= 0b00010,
	ChangeReturnValue	= 0b00100,
	BreakImmediate		= 0b01000,
	DontCall			= 0b10000,
};
ECLASS_BITWISE_OPERATORS(HookRes);


class ISingleVHook
{
	
public:
	ISingleVHook() = default;

	ISingleVHook(int offset) noexcept : VTIdx(offset) { };

	template<typename ThisPtr>
		requires std::is_pointer_v<ThisPtr> || std::is_integral_v<ThisPtr> && (sizeof(ThisPtr) == sizeof(M0Pointer)) && std::is_unsigned_v<ThisPtr>
	ISingleVHook(int offset, ThisPtr this_ptr, void* callback) : VTIdx(offset)
	{
		Init(reinterpret_cast<void*>(this_ptr), callback);
	}

	void Reconfigure(int offset) noexcept
	{
		this->VTIdx = offset;
	}

	void Register(int offset, M0Pointer this_ptr, M0Pointer callback)
	{
		Reconfigure(offset);
		Init(this_ptr, callback);
	}

	void Register(M0Pointer this_ptr, M0Pointer callback)
	{
		Init(this_ptr, callback);
	}

	void Shutdown();

	~ISingleVHook()
	{
		Shutdown();
	}

	M0Pointer ThisPtr() const noexcept
	{
		return ThisPointer;
	}

	constexpr int Offset()
	{
		return VTIdx;
	}

	M0Pointer OriginalFunction() const noexcept
	{
		return OriginalFunc;
	}

	operator bool() const noexcept
	{
		return ThisPtr() && this->VTIdx != 0;
	}

	bool operator==(const ISingleVHook& other) const noexcept
	{
		return VTIdx == other.VTIdx && other.ThisPointer == ThisPointer;
	}

	ISingleVHook(ISingleVHook&) = delete;
	ISingleVHook& operator=(const ISingleVHook&) = delete;
	ISingleVHook(ISingleVHook&&) = delete;
	ISingleVHook& operator=(ISingleVHook&&) = delete;

private:
	void Init(M0Pointer ptr, M0Pointer callback);

	int			VTIdx{ };
	M0Pointer*	ThisPointer{ };

	M0Pointer	OriginalFunc{ };
	M0Pointer	CurrentFunc{ };
};

using IUniqueVHook = std::unique_ptr<ISingleVHook>;

template<typename Temp>
inline const M0Pointer AddressOfSTDFunctionCallback(const std::function<Temp>& fn)
{
	return reinterpret_cast<const M0Pointer*>(&fn)[1];
}

template<class Func>
static inline PVOID GetAddressOfFunction(Func fn) { return *reinterpret_cast<PVOID*>(&fn); }






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
			M0Pointer ptr; \
		} u; \
		u.ptr = SINGLE_HOOK->OriginalFunction(); \
		NAME##_ACTUAL = u.fn; \
	} \

#define VHOOK_LINK(SINGLE_HOOK, NAME) \
	{ \
		union  \
		{ \
			NAME##_FN fn; \
			M0Pointer ptr; \
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

#define VHOOK_REG_S(NAME, PTR, OFFSET) std::make_unique<ISingleVHook>(OFFSET, PTR, VHOOK_GET_CALLBACK_S(NAME))

#define VHOOK_REG(NAME, PTR, OFFSET) std::make_unique<ISingleVHook>(OFFSET, PTR, VHOOK_GET_CALLBACK(NAME))