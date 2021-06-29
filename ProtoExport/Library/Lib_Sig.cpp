#include "Lib_Sig.hpp"
#include "../Helper/Json.hpp"

#include <sstream>
#include <iostream>
#include <regex>
#include <fstream>

using namespace std;


namespace M0SigSearch
{
	std::ostream& operator<<(std::ostream& str, vector<int>& vec)
	{
		for (auto& i : vec)
			str << i << ' ';
		return str;
	}

	M0Pointer ResolveSymbol(M0Pointer lib, const char* symbol)
	{
		return GetProcAddress(reinterpret_cast<HMODULE>(lib.get()), symbol);
	}

	static void PatternToBytes(const char* pattern, vector<int>& bytes)
	{
		regex to_replace("(\\?|\\*)");
		regex reg("([A-Fa-f0-9]{2})");

		string actual = regex_replace(pattern, to_replace, "2A");
		auto bytes_begin = sregex_iterator(actual.begin(), actual.end(), reg);
		auto bytes_end = sregex_iterator();

		const size_t size = distance(bytes_begin, bytes_end);
		bytes.reserve(size);

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

	M0Pointer FindPattern(M0Pointer lib, const char* pattern)
	{
		MEMORY_BASIC_INFORMATION MBI{ };
		if (!VirtualQuery(lib, &MBI, sizeof(MEMORY_BASIC_INFORMATION)))
			return nullptr;

		M0Pointer cur_address = MBI.AllocationBase;
		const PIMAGE_DOS_HEADER IDH = reinterpret_cast<PIMAGE_DOS_HEADER>(cur_address.get());
		const PIMAGE_NT_HEADERS NTH = reinterpret_cast<PIMAGE_NT_HEADERS>(cur_address + IDH->e_lfanew);

		const M0Pointer end_address = cur_address + NTH->OptionalHeader.SizeOfImage - 1;
		std::vector<int> bytes;

		PatternToBytes(pattern, bytes);

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
				return cur_address;

			cur_address++;
		}

		return nullptr;
	}

	size_t FindPatternByString(M0Pointer ptr, const char* str, std::vector<M0Pointer>& results, size_t search_size)
	{
		const char* sym = str;
		if (!str || !*sym)
			return NULL;

		const size_t len = strlen(str);
		const M0Pointer end = ptr + search_size;

		for (ptr; ptr < end; ptr++)
		{
			if (!ptr)
				break;
			if (ptr[0] == sym[0])
			{
				bool skip = false;
				for (uint8_t i = 0; i < len; i++)
				{
					if (ptr[i] != sym[i])
					{
						skip = true;
						break;
					}
				}
				if (!skip)
					results.push_back(ptr);
			}
		}

		return results.size();
	}

	const M0Pointer FindSig(M0SigInfo info)
	{
		Json::Value sigs;
		{
			std::ifstream config(".\\Miku\\Signatures.json", std::ifstream::binary);
			config >> sigs;
		}

		const char* name = info.Library->GetName();
		Json::Value& data = sigs[name][info.Symbol];

		const char* pattern{ };
		if (data.isMember("value"))
			pattern = data["value"].asCString();
		else return nullptr;

		int extra{ };
		int read{ };

		if (data.isMember("offset"))
			extra = data["offset"].asInt();

		if (data.isMember("read"))
			read = data["read"].asInt();
		
		M0Pointer addr = M0SigSearch::FindPattern(info.Library->GetAddress(), pattern);
		if (addr)
		{
			addr = addr + extra;

			for (int i = 0; i < read; i++)
				addr = addr.at<M0Pointer>(0);
		}

		return addr;
	}
}