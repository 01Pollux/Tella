
#include "../Source/Main.h"
#include "../GlobalHook/vhook.h"
#include "../GlobalHook/load_routine.h"

#include "../Helpers/Commons.h"
#include "../Interfaces/HatCommand.h"
#include "../Interfaces/IClientMode.h"

class AutoBackstab : public ExtraPanel, public IMainRoutine
{
	AutoBool bEnabled{ "AutoBackstab::Enable", true };
	
public:	//	AutoBackstab
	HookRes OnCreateMove(CUserCmd*);

public:	//	ExtraPanel
	void OnRenderExtra() final
	{
		ImGui::Checkbox("Auto-Backstab", &bEnabled);
	}

	void JsonCallback(Json::Value& cfg, bool read) final
	{
		Json::Value& extra = cfg["Extra"]["Auto-Backstab"];
		if (read)	PROCESS_JSON_READ(extra, "Enabled", Bool, bEnabled);
		else		PROCESS_JSON_WRITE(extra, "Enabled", *bEnabled);
	}

public:	//	IMainRoutine
	void OnLoadDLL() final
	{
		using namespace IGlobalVHookPolicy;
		auto create_move = CreateMove::Hook::QueryHook(CreateMove::Name);
		create_move->AddPostHook(HookCall::Any, std::bind(&AutoBackstab::OnCreateMove, this, std::placeholders::_2));
	}
} autostab;


HookRes AutoBackstab::OnCreateMove(CUserCmd* cmd)
{
	if (!cmd || !cmd->command_number)
		return HookRes::BreakImmediate;

	if (!bEnabled)
		return HookRes::Continue;

	ITFPlayer* pMe = ::ILocalPtr();

	if (pMe->GetClass() != TF_Spy)
		return HookRes::Continue;

	if (*pMe->GetEntProp<bool, PropType::Recv>("m_bFeignDeathReady"))
		return HookRes::Continue;

	IBaseObject* pCurWeapon = ::GetIBaseObject(pMe->GetActiveWeapon());

	if (!pCurWeapon || pCurWeapon->GetWeaponSlot() != 2)
		return HookRes::Continue;

	if (*pCurWeapon->GetEntProp<bool, PropType::Recv>("m_bReadyToBackstab"))
		cmd->buttons |= IN_ATTACK;

	return HookRes::Continue;
}

HAT_COMMAND(autostab_toggle, "Turn off/on auto-backstab")
{
	AutoBool enable("AutoBackstab::Enable");
	enable = !enable;
	REPLY_TO_TARGET(return, "Auto-Backstab is now %s\n", enable ? "ON":"OFF");
}