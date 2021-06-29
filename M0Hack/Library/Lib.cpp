
#include "Lib.hpp"
#include "Lib_Sig.hpp"

#include "Helper/Debug.hpp"

void* M0Library::FindPattern(const char* signame)
{
	void* addr = M0SigSearch::FindSig(M0SigSearch::SigInfo{ this, signame });

	if (!addr)
	{
		M0Logger::Err(
			"[Library: {}] Empty Signature for: \"{}\".",
			this->Name,
			signame
		);
	}

	return addr;
}

void* M0Library::FindInterface(const char* name)
{
	IFuncThunk<void*, const char*, int*> CreateInterface(FindPattern("CreateInterface"));
	return CreateInterface(name, nullptr);
}