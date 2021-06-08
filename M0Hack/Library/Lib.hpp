#pragma once

#include "../winmain.hpp"

#include <map>

using M0Pointer = void*;

class M0Library
{
public:
	M0Library(const char* name) : Name(name)
	{
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, name, reinterpret_cast<HMODULE*>(&Address));
	}
	
	const char* GetName() const noexcept
	{
		return Name;
	}

	M0Pointer GetAddress() const noexcept
	{
		return Address;
	}

	M0Pointer FindPattern(const char* name);

	// Find an exported interface by the game
	M0Pointer FindInterface(const char* interface_name);

private:
	const char* Name;
	M0Pointer Address;

	std::map<const char*, M0Pointer> CachedPointers;
};

class CallClass { };

namespace M0Libraries
{
	extern M0Library* Client;
	extern M0Library* Engine;
	extern M0Library* ValveSTD;
	extern M0Library* D3DX9;
	extern M0Library* MatSurface;

	void InitLibrary();
}

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