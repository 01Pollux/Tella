#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>


//#define M01_MANUAL_MAPPED_DLL
#define M01_USING_VECTORED_HANDLER
//#define M01_USING_STDOUT



namespace ThisDLL
{
	void Unload();
}

template<class _Ty, size_t _Size>
[[nodiscard]] constexpr size_t SizeOfArray(const _Ty(&)[_Size])
{
	return _Size;
}

template<class _Ty, size_t _Size>
[[nodiscard]] constexpr size_t SizeOfArray(const std::array<_Ty, _Size>&)
{
	return _Size;
}



#ifdef M01_USING_STDOUT
#define PrintToConsole(STR) std::cout << STR << '\n';
#else
#define PrintToConsole(STR)
#endif