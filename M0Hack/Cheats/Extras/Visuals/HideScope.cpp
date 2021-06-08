#include "Cheats/Main.hpp"

#include "GlobalHook/detour.hpp"
#include "Helper/Debug.hpp"

namespace Visuals
{
	class HideScope
	{
	public:
		HideScope();

		operator bool() const noexcept { return Enable; }

	private:
		M0Config::Bool Enable{
			"Visual.Hide Scope",
			true,
			"Hide sniper zoom overlay"
		};
		IDetour detour_CHudScope_ShouldDraw;
	};

	HideScope hide_scope;

	DETOUR_CREATE_MEMBER(bool, CHudScope_ShouldDraw)
	{
		if (hide_scope)
			return false;

		return DETOUR_MEMBER_CALL(CHudScope_ShouldDraw);
	}
}

Visuals::HideScope::HideScope()
{
	M0EventManager::AddListener(
		EVENT_KEY_LOAD_DLL,
		[](M0EventData*)
		{
			M0Pointer ptr = M0Libraries::Client->FindPattern("CHudScope::ShouldDraw");
			if (!ptr)
				M0Logger::Err("Failed to attach detour for CHudScope::ShouldDraw");
			else
				DETOUR_LINK_TO_MEMBER(CHudScope_ShouldDraw, ptr);
		},
		EVENT_NULL_NAME
	);

	M0EventManager::AddListener(
		EVENT_KEY_UNLOAD_DLL,
		[](M0EventData*)
		{
			DETOUR_UNLINK_FROM_MEMBER(CHudScope_ShouldDraw);
		},
		EVENT_NULL_NAME
	);
}
