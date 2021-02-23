#pragma once

#include <memory>
#include <vector>

namespace SigTools
{
	void*		ResolveSymbol(void* lib, const char* sym);
	uintptr_t	FindPatternEx(uintptr_t lib, const char* sym);
	size_t		FindPatternStr(uintptr_t ptr, const char* str, std::vector<uintptr_t>& results, size_t search_size = 0x1000);
	uintptr_t	GetFuncStart(uintptr_t ptr);
};