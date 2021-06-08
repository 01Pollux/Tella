#include "vtable.hpp"
#include "Helper/Debug.hpp"
#include "winmain.hpp"


class GuardVirtualProtect
{
	size_t size;
	M0Pointer const ptr;
	DWORD protection;
	bool is_on = true;
public:
	GuardVirtualProtect(M0Pointer ptr, size_t size) : size(size), ptr(ptr)
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


void ISingleVHook::Init(M0Pointer ptr, M0Pointer callback)
{
	ThisPointer = reinterpret_cast<M0Pointer*>(ptr);
	M0Pointer* vfnp = static_cast<M0Pointer*>(*ThisPointer) + VTIdx;

	CurrentFunc = reinterpret_cast<M0Pointer>(callback);

	GuardVirtualProtect protect(vfnp, sizeof(M0Pointer));

	OriginalFunc = *vfnp;
	*vfnp = CurrentFunc;
}


void ISingleVHook::Shutdown()
{
	if (!ThisPointer)
		return;

	M0Pointer* vfnp = static_cast<M0Pointer*>(*ThisPointer) + VTIdx;

	GuardVirtualProtect protect(vfnp, sizeof(M0Pointer));
	*vfnp = OriginalFunc;

	ThisPointer = nullptr;
}