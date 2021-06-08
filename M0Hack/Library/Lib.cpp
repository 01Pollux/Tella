
#include "Lib.hpp"
#include "Lib_Sig.hpp"

#include "Helper/Debug.hpp"

M0Pointer M0Library::FindPattern(const char* signame)
{
	const auto iter = CachedPointers.find(signame);
	if (iter == CachedPointers.end())
	{
		M0Pointer addr = M0SigSearch::FindSig({ this, signame });

		if (!addr)
		{
			M0Logger::Err(
				"[Library: {}] Empty Signature for: \"{}\".",
				this->Name,
				signame
			);
		}

		CachedPointers.insert(std::make_pair(signame, addr));
		return addr;
	}
	else return iter->second;
}

M0Pointer M0Library::FindInterface(const char* name)
{
	IFuncThunk<M0Pointer, const char*, int*> CreateInterface(FindPattern("CreateInterface"));
	return CreateInterface(name, nullptr);
}


namespace M0Libraries
{
	M0Library* Client = nullptr;
	M0Library* Engine = nullptr;
	M0Library* ValveSTD = nullptr;
	M0Library* D3DX9 = nullptr;
	M0Library* MatSurface = nullptr;

#define TO_DLL_STR(NAME) #NAME##".dll"

	void InitLibrary()
	{
		Client = new M0Library(TO_DLL_STR(client));
		Engine = new M0Library(TO_DLL_STR(engine));
		ValveSTD = new M0Library(TO_DLL_STR(vstdlib));
		D3DX9 = new M0Library(TO_DLL_STR(shaderapidx9));
		MatSurface = new M0Library(TO_DLL_STR(vguimatsurface));
	}

#undef TO_DLL_STR
}