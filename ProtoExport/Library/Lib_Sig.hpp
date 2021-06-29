#pragma once

#include "Lib.hpp"

#include <utility>
#include <vector>
#include <functional>

struct M0SigInfo
{
	M0Library* Library;
	const char* Symbol;
};

namespace M0SigSearch
{
	M0Pointer ResolveSymbol(M0Pointer lib, const char* symbol);
	M0Pointer FindPattern(M0Pointer lib, const char* pattern);
	size_t FindPatternByString(M0Pointer ptr, const char* str, std::vector<M0Pointer>& results, size_t search_size);

	const M0Pointer FindSig(M0SigInfo info);
}