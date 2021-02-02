#include "sdk.h"
#include "SigTools.h"
#include <time.h>

#include <cstdint>
#include <psapi.h>
#include <regex>
#include <sstream>
#include <thread>

#include "../Source/Debug.h"

using namespace std;

static void PatternToBytes(const char* pattern, vector<int>& bytes)
{
	regex to_replace("(\\?|\\*)");
	regex reg("([A-Fa-f0-9]{2})");

	string actual = regex_replace(pattern, to_replace, "2A");
	auto bytes_begin = sregex_iterator(actual.begin(), actual.end(), reg);
	auto bytes_end = sregex_iterator();

	int res;
	stringstream stream;
	stream << std::hex;

	while (bytes_begin != bytes_end)
	{
		stream << bytes_begin->str();
		stream >> res;
		stream.clear();

		bytes.push_back(res);
		bytes_begin++;
	}
}

void* SigTools::ResolveSymbol(void* lib, const char* sym)
{
	return GetProcAddress(reinterpret_cast<HMODULE>(lib), sym);
}

uintptr_t SigTools::FindPatternEx(uintptr_t lib, const char* sym)
{
	static mutex mu;
	lock_guard<mutex> m(mu);

	vector<int> bytes;
	thread resolve_pattern(PatternToBytes, sym, ref(bytes));

	MEMORY_BASIC_INFORMATION MBI{ };
	if (!VirtualQuery(reinterpret_cast<LPCVOID>(lib), &MBI, sizeof(MEMORY_BASIC_INFORMATION)))
	{
		MIKUDebug::LogCritical("VirtualQuery Failed");
		resolve_pattern.detach();
		return NULL;
	}

	uint8_t* cur_address = reinterpret_cast<uint8_t*>(MBI.AllocationBase);

	auto IDH = reinterpret_cast<PIMAGE_DOS_HEADER>(cur_address);
	auto IPE = reinterpret_cast<PIMAGE_NT_HEADERS>(cur_address + IDH->e_lfanew);

	uint8_t* end_address = cur_address + IPE->OptionalHeader.SizeOfImage - 1;

	resolve_pattern.join();

	const size_t size = bytes.size();

	bool found;
	while (cur_address < end_address)
	{
		found = true;
		for (size_t i = 0; i < size; i++)
		{
			if (bytes[i] != 0x2A && bytes[i] != cur_address[i])
			{
				found = false;
				break;
			}
		}

		if (found)
			return reinterpret_cast<uintptr_t>(cur_address);
		
		cur_address++;
	}

	MIKUDebug::LogCritical(Format("Failed to find patterns: \"", sym, "\""));
	return NULL;
}

uintptr_t SigTools::GetFuncStart(uintptr_t ptr)
{
	for (; *reinterpret_cast<uint8_t*>(ptr) != 0x55 
		|| *reinterpret_cast<uint16_t*>(ptr + 1) != 0xEC8B 
		|| *reinterpret_cast<uint8_t*>(ptr + 4) != 0xEC; 
		ptr--) { };
	return ptr;
}

size_t SigTools::FindPatternStr(uintptr_t ptr, const char* str, vector<uintptr_t>& pResults, size_t size)
{
	const char* sym = str;
	if (!str || !*sym)
		return 0;

	size_t len = strlen(str);
	uintptr_t pCur = ptr;
	uintptr_t pEnd = pCur + size;

	static bool skip = false;
	vector<uintptr_t> pAddress;

	for (pCur; pCur < pEnd; pCur++)
	{
		if (!pCur)
			break;
		if (*reinterpret_cast<uint8_t*>(pCur) == sym[0])
		{
			skip = false;
			for (uint8_t i = 0; i < len; i++)
			{
				if (reinterpret_cast<uint8_t*>(pCur)[i] != sym[i])
				{
					skip = true;
					break;
				}
			}
			if (!skip)
				pResults.push_back(pCur);
		}
	}
	for (auto addr : pResults)
		pResults.push_back(GetFuncStart(addr));

	return pResults.size();
}