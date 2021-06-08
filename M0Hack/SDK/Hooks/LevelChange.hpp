#pragma once

#include "GlobalHook/vhook.hpp"

namespace M0HookManager
{
	namespace Policy
	{
		MAKE_VHOOK_POLICY(LevelInit, "LevelInit", void, const char*);
		MAKE_VHOOK_POLICY(LevelShutdown, "LevelShutdown", void);
	}
}