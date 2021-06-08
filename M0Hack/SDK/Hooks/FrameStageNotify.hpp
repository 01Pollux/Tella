#pragma once

#include "GlobalHook/vhook.hpp"
#include "SDK/cdll_int.hpp"

namespace M0HookManager
{
	namespace Policy
	{
		MAKE_VHOOK_POLICY(FrameStageNotify, "FrameStageNotify", void, ClientFrameStage);
	}
}