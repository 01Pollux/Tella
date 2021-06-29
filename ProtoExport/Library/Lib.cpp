
#include "Lib.hpp"
#include "Lib_Sig.hpp"

M0Pointer M0Library::FindPattern(const char* signame)
{
	return M0SigSearch::FindSig({ this, signame });
}

M0Pointer M0Library::FindInterface(const char* name)
{
	IFuncThunk<M0Pointer, const char*, int*> CreateInterface(FindPattern("CreateInterface"));
	return CreateInterface(name, nullptr);
}