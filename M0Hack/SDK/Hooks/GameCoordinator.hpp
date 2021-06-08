#pragma once

#include "GlobalHook/detour.hpp"
#include "Proto/GCSys.hpp"

namespace M0HookManager
{
	namespace Policy
	{
		MAKE_DHOOK_POLICY(GC_SendMessage,		"GC [SendMessage]", void);
		MAKE_DHOOK_POLICY(GC_YieldForMessage,	"GC [YieldForMessage]", void);
	}
}