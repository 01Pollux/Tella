#pragma once

#include "GlobalHook/vhook.hpp"

#include "VGUI.hpp"
#include "Huds/HudElement.h"

namespace M0HookManager
{
	namespace Policy
	{
		MAKE_VHOOK_POLICY(PaintTraverse, "PaintTraverse", void, uint32_t, bool, bool);
	}
}