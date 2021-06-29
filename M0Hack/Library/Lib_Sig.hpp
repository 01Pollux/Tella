#pragma once

#include "Lib.hpp"

#include <utility>
#include <vector>
#include <functional>


namespace M0SigSearch
{
	struct SigInfo
	{
		M0Library*	Library;
		const char* Symbol;
	};

	_NODISCARD void*	ResolveSymbol(void* lib, const char* symbol);
	_NODISCARD void*	FindPattern(void* lib, const char* pattern);
	_NODISCARD size_t	FindPatternByString(void* ptr, const std::string_view str, std::vector<void*>& results, size_t search_size);

	_NODISCARD void*	FindSig(SigInfo info);
}