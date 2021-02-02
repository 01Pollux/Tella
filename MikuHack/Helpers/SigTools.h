#pragma once

#include "sdk.h"

namespace SigTools
{
	void*		ResolveSymbol(void* lib, const char* sym);
	uintptr_t	FindPatternEx(uintptr_t lib, const char* sym);
	size_t		FindPatternStr(uintptr_t ptr, const char* str, std::vector<uintptr_t>& pResults, size_t iSize = 0x1000);
	uintptr_t	GetFuncStart(uintptr_t ptr);
};