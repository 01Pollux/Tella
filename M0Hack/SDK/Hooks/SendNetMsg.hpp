#pragma once

#include "GlobalHook/detour.hpp"
#include "NetMessage.hpp"

namespace M0HookManager
{
	namespace Policy
	{
		MAKE_DHOOK_POLICY(SendNetMsg, "SendNetMsg", bool, INetMessage&, bool, bool);
	}
}