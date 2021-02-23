#include "VTable.h"
#include "../Source/Debug.h"

#include <Windows.h>


class GuardVirtualProtect
{
	size_t size;
	void* const ptr;
	DWORD protection;
	bool is_on = true;
public:
	GuardVirtualProtect(void* ptr, size_t size) : size(size), ptr(ptr)
	{
		if (!VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &protection))
		{
			is_on = false;
			throw std::runtime_error("VirtualProtect Failed");
		}
	}
	~GuardVirtualProtect()
	{
		if (is_on)
			VirtualProtect(ptr, size, protection, &protection);
	}

public:
	GuardVirtualProtect(const GuardVirtualProtect&)				= delete;
	GuardVirtualProtect& operator=(const GuardVirtualProtect&)	= delete;
	GuardVirtualProtect(GuardVirtualProtect&&)					= delete;
	GuardVirtualProtect& operator=(GuardVirtualProtect&&)		= delete;
};

void ISingleVHook::Init(void* ptr, void* callback)
{
	using namespace std;

	base = reinterpret_cast<uintptr_t**>(ptr);
	uintptr_t* vfnp = *base + offset;

	constexpr size_t size_of_dword = sizeof(uintptr_t);

	actual = reinterpret_cast<uintptr_t>(callback);

	GuardVirtualProtect protect(vfnp, size_of_dword);

	original = *vfnp;
	*vfnp = actual;
}

void ISingleVHook::Shutdown()
{
	if (!base)
		return;

	uintptr_t* vtable = *base;

	constexpr size_t size_of_dword = sizeof(uintptr_t);

	GuardVirtualProtect protect(vtable + offset, size_of_dword);
	vtable[offset] = original;

	base = nullptr;
}