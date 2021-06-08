#include "Cheats/Main.hpp"
#include "Hooks/CreateMove.hpp"

class AutoBackstab
{
public:
	AutoBackstab();
	HookRes OnCreateMove(UserCmd*);

private:
	M0Config::Bool Enable{ "AutoStab.Enable", true, "Automatically stab player's"};
} static autostab;

AutoBackstab::AutoBackstab()
{
	M0EventManager::AddListener(
		EVENT_KEY_LOAD_DLL,
		[this](M0EventData*)
		{
			M0HookManager::Policy::CreateMove CreateMove(true);
			CreateMove->AddPostHook(HookCall::Late, std::bind(&AutoBackstab::OnCreateMove, this, std::placeholders::_2));
		},
		EVENT_NULL_NAME
	);

	M0EventManager::AddListener(
		EVENT_KEY_RENDER_EXTRA,
		[this](M0EventData*)
		{
			if (ImGui::CollapsingHeader("Auto backstab"))
			{
				ImGui::Checkbox("Enabled", Enable.data());
				ImGui::SameLineHelp(Enable);
			}
		},
		EVENT_NULL_NAME
	);
}

HookRes AutoBackstab::OnCreateMove(UserCmd* cmd)
{
	using HookRes::Continue;

	if (!Enable)
		return Continue;

	const ILocalPlayer pMe;
	if (pMe->Class != TFClass::Spy || pMe->FeignDeathReady)
		return Continue;

	IBaseWeapon pWpn(pMe->ActiveWeapon);
	if (!pWpn || pWpn->GetWeaponSlot() != 2)
		return Continue;

	if (pWpn->ReadyToBackstab)
		cmd->Buttons |= IN_ATTACK;

	return Continue;
}