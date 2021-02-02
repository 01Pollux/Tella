#include "VTable.h"

#include <Windows.h>

#include <mutex>

void ISingleVHook::Init(void* ptr, void* callback)
{
	using namespace std;

	base = reinterpret_cast<uintptr_t**>(ptr);
	uintptr_t* vfnp = *base + offset;

	constexpr size_t size_of_dword = sizeof(uintptr_t);

	actual = reinterpret_cast<uintptr_t>(callback);

	BEGIN_VIRTUAL_PROTECT(vfnp, size_of_dword);

	original = *vfnp;
	*vfnp = actual;

	RESTORE_VIRTUAL_PROTECT();
}

void ISingleVHook::Shutdown()
{
	if (!base)
		return;

	uintptr_t* vtable = *base;

	constexpr size_t size_of_dword = sizeof(uintptr_t);

	BEGIN_VIRTUAL_PROTECT(vtable + offset, size_of_dword);
	vtable[offset] = original;
	RESTORE_VIRTUAL_PROTECT();

	base = nullptr;
}

/*void InCallback(int )
{

}

void EmitIDK()
{
	IGlobalHook::MyHook* test = IGlobalHook::MyHook::NewHook({nullptr, 0}, std::placeholders::_1);
	test->AddPostHook(InCallback);
}*/