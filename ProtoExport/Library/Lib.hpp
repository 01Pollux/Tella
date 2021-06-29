#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <map>

class M0Pointer
{
public:
	using type = uintptr_t;

	M0Pointer() = default;
	constexpr M0Pointer(type ptr)		noexcept : Ptr(ptr) { };
	constexpr M0Pointer(void* ptr)		noexcept : Ptr(type(ptr)) { };
	constexpr M0Pointer(std::nullptr_t)	noexcept { };
	constexpr M0Pointer(HMODULE ptr)	noexcept : Ptr(type(ptr)) { };

	M0Pointer(const M0Pointer&) = default;				M0Pointer(M0Pointer&&) = default;
	M0Pointer& operator=(const M0Pointer&) = default;	M0Pointer& operator=(M0Pointer&&) = default;
	
	constexpr operator type() const noexcept { return Ptr; }
	
	operator void*()			  noexcept { return get(); }
	operator const void*()	const noexcept { return get(); }

	_NODISCARD void* get()			   noexcept { return reinterpret_cast<void*>(Ptr); }
	_NODISCARD const void* get() const noexcept { return reinterpret_cast<const void*>(Ptr); }

	M0Pointer& operator+=(const M0Pointer& r) noexcept
	{ Ptr = static_cast<type>(static_cast<ptrdiff_t>(Ptr) + static_cast<ptrdiff_t>(r.Ptr)); return *this; }
	M0Pointer& operator-=(const M0Pointer& r) noexcept
	{ Ptr = static_cast<type>(static_cast<ptrdiff_t>(Ptr) - static_cast<ptrdiff_t>(r.Ptr)); return *this; }

	M0Pointer& operator++() noexcept { Ptr++; return *this; }
	M0Pointer& operator--() noexcept { Ptr--; return *this; }
	M0Pointer operator++(int) noexcept { M0Pointer old(*this); ++Ptr; return old; }
	M0Pointer operator--(int) noexcept { M0Pointer old(*this); --Ptr; return old; }

	auto operator<=>(const M0Pointer&) const = default;

	template<typename _Ty = uint8_t>
	const _Ty& at(size_t pos) const noexcept { return reinterpret_cast<const _Ty*>(get())[pos]; }
	template<typename _Ty = uint8_t>
	_Ty& at(size_t pos)				noexcept { return reinterpret_cast<_Ty*>(get())[pos]; }

	const uint8_t& operator[](size_t pos) const noexcept { return at(pos); }
	uint8_t& operator[](size_t pos)				noexcept { return at(pos); }

private: type Ptr{ };
};


class M0Library
{
public:
	M0Library(const char* name) : Name(name)
	{
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, name, reinterpret_cast<HMODULE*>(&Pointer));
	}
	
	const char* GetName() const noexcept
	{
		return Name;
	}

	const M0Pointer GetAddress() const noexcept
	{
		return Pointer;
	}

	M0Pointer FindPattern(const char* name);

	// Find an exported interface by the game
	M0Pointer FindInterface(const char* interface_name);

private:
	const char* Name;
	M0Pointer Pointer;
};


class CallClass { };

template<typename ReturnType, typename... Args>
class IFuncThunk
{
	using FnPtr = ReturnType(*)(Args...);
public:
	IFuncThunk() = default;
	IFuncThunk(M0Pointer ptr) noexcept
	{
		union
		{
			FnPtr fn;
			M0Pointer ptr;
		} u{ .ptr = ptr };

		Func = u.fn;
	};

	IFuncThunk(const char* lib_name, const char* sig_name)
	{
		M0Library lib(lib_name);
		union
		{
			FnPtr fn;
			M0Pointer ptr;
		} u{ .ptr = lib.FindPattern(sig_name) };

		Func = u.fn;
	}

	constexpr ReturnType operator()(Args... args)
	{
		return Func(args...);
	}

private:
	FnPtr Func{ };
};

template<typename ReturnType, typename... Args>
class IMemberFuncThunk
{
	using FnPtr = ReturnType(CallClass::*)(Args...);
public:
	IMemberFuncThunk() = default;
	IMemberFuncThunk(M0Pointer ptr) noexcept
	{
		union
		{
			FnPtr fn;
			M0Pointer ptr;
		} u{ .ptr = ptr };

		Func = u.fn;
	};

	IMemberFuncThunk(const char* lib_name, const char* sig_name)
	{
		M0Library lib(lib_name);
		union
		{
			FnPtr fn;
			M0Pointer ptr;
		} u{ .ptr = lib.FindPattern(sig_name) };

		Func = u.fn;
	}

	template<class Class>
	constexpr ReturnType operator()(Class* thisptr, Args... args)
	{
		return ((CallClass*)thisptr->*Func)(args...);
	}

	template<class Class>
	constexpr ReturnType operator()(const Class* thisptr, Args... args)
	{
		return ((CallClass*)thisptr->*Func)(args...);
	}

private:
	FnPtr Func{ };
};


template<typename ReturnType, typename... Args>
class IMemberVFuncThunk
{
	using FnPtr = ReturnType(CallClass::*)(Args...);
public:
	IMemberVFuncThunk() = default;
	IMemberVFuncThunk(int offset) noexcept : Offset(offset) { };

	template<class Class>
	constexpr ReturnType operator()(Class* thisptr, Args... args)
	{
		M0Pointer* vtable = *(M0Pointer**)(thisptr);
		union
		{
			FnPtr fn;
			M0Pointer ptr;
		} u{ .ptr = vtable[Offset] };

		return ((CallClass*)thisptr->*u.fn)(args...);
	}

	template<class Class>
	constexpr ReturnType operator()(const Class* thisptr, Args... args)
	{
		const M0Pointer* vtable = *(const M0Pointer**)(thisptr);
		union
		{
			FnPtr fn;
			const M0Pointer ptr;
		} u{ .ptr = vtable[Offset] };

		return ((CallClass*)thisptr->*u.fn)(args...);
	}

private:
	int Offset{ };
};