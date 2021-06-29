#pragma once

#include <map>
#include "winmain.hpp"
#include "SDK/ProtoExport.hpp"

#define M0ENGINE_DLL		"engine.dll"
#define M0CLIENT_DLL		"client.dll"
#define M0VALVESTD_DLL		"vstdlib.dll"
#define M0D3DX9_DLL			"shaderapidx9.dll"
#define M0MATSURFACE_DLL	"vguimatsurface.dll"


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

	void* GetAddress() const noexcept
	{
		return Pointer;
	}

	void* FindPattern(const char* name);

	// Find an exported interface by the game
	void* FindInterface(const char* interface_name);

private:
	const char* Name;
	void* Pointer;
};


class CallClass { };

template<typename ReturnType, typename... Args>
class IFuncThunk
{
	using FnPtr = ReturnType(*)(Args...);
public:
	IFuncThunk() = default;
	IFuncThunk(void* ptr) noexcept { set(ptr); }
	IFuncThunk(const char* lib_name, const char* sig_name) { M0Library lib(lib_name); set(lib.FindPattern(sig_name)); }

	constexpr ReturnType operator()(Args... args)
	{
		return Func(args...);
	}

	void set(void* ptr) noexcept
	{
		union
		{
			FnPtr fn;
			void* ptr;
		} u{ .ptr = ptr };

		Func = u.fn;
	}

	void* get() const noexcept
	{
		union
		{
			FnPtr fn;
			void* ptr;
		} u{ .fn = Func };
		return u.ptr;
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
	IMemberFuncThunk(void* ptr) noexcept { set(ptr); }
	IMemberFuncThunk(const char* lib_name, const char* sig_name) { M0Library lib(lib_name); set(lib.FindPattern(sig_name)); }

	constexpr ReturnType operator()(const void* thisptr, Args... args)
	{
		return ((CallClass*)thisptr->*Func)(args...);
	}

	void set(void* ptr) noexcept
	{
		union
		{
			FnPtr fn;
			void* ptr;
		} u{ .ptr = ptr };

		Func = u.fn;
	}

	void* get() const noexcept
	{
		union
		{
			FnPtr fn;
			void* ptr;
		} u{ .fn = Func };
		return u.ptr;
	}

private:
	FnPtr Func{ };
};


template<typename _RTy, typename... _Args>
class IMemberVFuncThunk
{
public:
	IMemberVFuncThunk() = default;
	IMemberVFuncThunk(int offset) noexcept : Offset(offset) { };

	constexpr _RTy operator()(const void* thisptr, _Args... args)
	{
		void** vtable = *(void***)(thisptr);
		IMemberFuncThunk<_RTy, _Args...> fn(vtable[Offset]);
		return fn(thisptr, args...);
	}

	void set(int offset) noexcept
	{
		Offset = offset;
	}

	int get() const noexcept
	{
		return Offset;
	}

private:
	int Offset{ };
};