
#include "../Interfaces/HatCommand.h"
#include "../Source/Main.h"
#include "../Interfaces/IVEClientTrace.h"
#include "../Interfaces/IClientMode.h"
#include "../Helpers/VTable.h"


class AutoBackstab : public ExtraPanel
{
public:	//	AutoBackstab
	AutoBool bEnabled{ "AutoBackstab::Enable", true };

public:	//	Globals::IGlobalHooks
	AutoBackstab()
	{
		IGlobalEvent::CreateMove::Hook::Register(std::bind(&AutoBackstab::OnCreateMove, this, std::placeholders::_1));
	};
	HookRes OnCreateMove(bool&);

public:	//	ExtraPanel
	void OnRenderExtra() override final
	{
		ImGui::Checkbox("Auto-Backstab", bEnabled.get());
	}

	void JsonCallback(Json::Value& cfg, bool read) override
	{
		Json::Value& extra = cfg["Extra"]["Auto-Backstab"];
		if (read)
		{
			PROCESS_JSON_READ(extra, "Enabled", Bool, bEnabled);
		}
		else {
			PROCESS_JSON_WRITE(extra, "Enabled", *bEnabled);
		}
	}
} autostab;


HookRes AutoBackstab::OnCreateMove(bool& res)
{
	if (!bEnabled)
		return HookRes::Continue;

	if (pLocalPlayer->GetClass() != TFClass::Spy)
		return HookRes::Continue;

	if (*pLocalPlayer->GetEntProp<bool>("m_bFeignDeathReady"))
		return HookRes::Continue;

	static IBaseObject* pCurWeapon;
	pCurWeapon = reinterpret_cast<IBaseObject*>(clientlist->GetClientEntityFromHandle(pLocalPlayer->GetActiveWeapon()));

	if (!pCurWeapon || pCurWeapon->GetWeaponSlot() != 2)
		return HookRes::Continue;

	if (*pCurWeapon->GetEntProp<bool>("m_bReadyToBackstab"))
		Globals::m_pUserCmd->buttons |= IN_ATTACK;

	return HookRes::Continue;
}

HAT_COMMAND(autostab_toggle, "Turn off/on auto-backstab")
{
	autostab.bEnabled = !autostab.bEnabled;
	REPLY_TO_TARGET(return, "Auto-Backstab is now %s\n", autostab.bEnabled ? "ON":"OFF");
}
