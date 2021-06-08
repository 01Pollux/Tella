#pragma once

#include "GlobalHook/vhook.hpp"
#include "cdll_int.hpp"
#include "UserCmd.hpp"

namespace M0HookManager
{
	namespace Policy
	{
		MAKE_VHOOK_POLICY(CreateMove, "CreateMove", bool, float, UserCmd*);
	}
}

M0_INTERFACE;
extern bool* pSendPacket;
M0_END;