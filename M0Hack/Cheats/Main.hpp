#pragma once

#include "GlobalHook/listener.hpp"

#include "Profiler/mprofiler.hpp"
#include "Helper/Config.hpp"

#include "Helper/Timer.hpp"

#include "EntityList.hpp"
#include "BasePlayer.hpp"
#include "BaseObject.hpp"
#include "BaseWeapon.hpp"

#include "GlobalVars.hpp"
#include "cdll_int.hpp"

namespace MenuPanel
{
	extern bool IsMenuActive;

	inline void Toggle() noexcept
	{
		IsMenuActive = !IsMenuActive;
	}

	inline bool IsActive() noexcept
	{
		return IsMenuActive;
	}

	void Render();
}