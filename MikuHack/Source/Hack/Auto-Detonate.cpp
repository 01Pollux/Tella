
#include "../Source/Main.h"
#include "../GlobalHook/vhook.h"
#include "../GlobalHook/load_routine.h"

#include "../Helpers/Commons.h"
#include "../Interfaces/HatCommand.h"
#include "../Interfaces/IClientMode.h"
#include "../Interfaces/IVEClientTrace.h"
#include "../Interfaces/IClientListener.h"


class AutoDetonate : public ExtraPanel, public IMainRoutine
{
	AutoBool bEnabled		{ "AutoDetonate::Enable", true };
	AutoBool bIgnoreStealth	{ "AutoDetonate::bIgnoreStealth", true };
	IGlobalVHook<bool, float, CUserCmd*>* CreateMove;

public:	//	AutoDetonate
	HookRes OnCreateMove(CUserCmd*);

public:	//	ExtraPanel
	void OnRenderExtra() final
	{
		ImGui::Checkbox("Auto-Detonate", &bEnabled);
	}

	void JsonCallback(Json::Value& cfg, bool read) final
	{
		Json::Value& extra = cfg["Extra"]["Auto-Detonate"];
		if (read)
		{
			PROCESS_JSON_READ(extra, "Enabled", Bool, bEnabled);
		}
		else {
			PROCESS_JSON_WRITE(extra, "Enabled", *bEnabled);
		}
	}

public:	//	IMainRoutine
	void OnLoadDLL() final
	{
		using namespace IGlobalVHookPolicy;
		CreateMove = CreateMove::Hook::QueryHook(CreateMove::Name);
		CreateMove->AddPostHook(HookCall::Any, std::bind(&AutoDetonate::OnCreateMove, this, std::placeholders::_2));
	}
} static auto_detonate;


HookRes AutoDetonate::OnCreateMove(CUserCmd* cmd)
{
	if (!bEnabled)
		return HookRes::Continue;

	ITFPlayer* pMe = ::ILocalPtr();

	if (pMe->GetClass() != TF_Demoman)
		return HookRes::Continue;
	
	static Timer timer_leftclick;
	static Timer timer_checkscottish;

	if (cmd->buttons & IN_ATTACK)
		timer_leftclick.update();

	if (!timer_leftclick.has_elapsed(500ms))
		return HookRes::Continue;

	static bool IsScottishResistance = false;
	if (timer_checkscottish.trigger_if_elapsed(2500ms))
	{
		IBaseObject* pWeapon = ::GetIBaseObject(pMe->GetWeaponList()[1]);
		IsScottishResistance = pWeapon && pWeapon->GetItemDefinitionIndex() == 130;
	}

	static std::vector<IBaseObject*> stickies;	stickies.clear();
	static std::vector<ITFPlayer*> players;		players.clear();
	
	{
		for (const auto& infos = ent_infos.GetInfos(); 
			const MyClientCacheList& cache : infos)
		{
			IClientShared* pEnt = cache.pEnt;
			if(cache.flag == EntFlag::Extra && pEnt->IsClassID(ClassID_CTFGrenadePipebombProjectile) && *pEnt->GetEntProp<int, PropType::Recv>("m_iType") == 1)
			{
				stickies.push_back(reinterpret_cast<IBaseObject*>(pEnt));
			}
			else if (cache.flag == EntFlag::Player)
			{
				ITFPlayer* pPlayer = reinterpret_cast<ITFPlayer*>(pEnt);

				if (pPlayer == pMe)
					continue;

				if (pPlayer->GetLifeState() != LIFE_STATE::ALIVE)
					continue;

				if (pPlayer->InCond(TF_COND_STEALTHED) && bIgnoreStealth)
					continue;

				if (pPlayer->InCond(TF_COND_INVULNERABLE) || pPlayer->InCond(TF_COND_PHASE))
					continue;

				if (pPlayer->GetTeam() == pMe->GetTeam())
					continue;

				players.push_back(pPlayer);
			}
		}
	}

	ICollideable* pCollide = pMe->GetCollideable();

	bool change_hr = false;

	trace_t tr;
	Trace::ILocalFilterSimple filter;

	for (const IBaseObject* stickie : stickies)
	{
		Vector vecOrigin = stickie->GetAbsOrigin();
		{
			const Vector position = pMe->GetAbsOrigin() + ((pCollide->OBBMins() + pCollide->OBBMaxs()) / 2);
			if (vecOrigin.DistTo(position) < 170.f)
			{
				Trace::TraceLine(vecOrigin, position, COLLISION_GROUP_PLAYER_MOVEMENT, &tr, &filter);
				if (tr.fraction >= 1.0)
					return HookRes::Continue;
			}
		}

		for (const ITFPlayer* target: players)
		{
			const Vector position = target->GetAbsOrigin() + ((pCollide->OBBMins() + pCollide->OBBMaxs()) / 2);

			if (vecOrigin.DistTo(position) > 150.f)
				continue;

			Trace::TraceLine(vecOrigin, position, COLLISION_GROUP_PLAYER_MOVEMENT, &tr, &filter);

			if (tr.fraction < 1.0)
				continue;

			if (IsScottishResistance)
			{
				QAngle angRes = GetAimAngle(stickie->GetAbsOrigin());
				cmd->viewangles = angRes;
				CreateMove->SetReturnInfo(false);
				change_hr = true;
			}

			cmd->buttons |= IN_ATTACK2;
			break;
		}
	}

	return change_hr ? HookRes::ChangeReturnValue : HookRes::Continue;
}


HAT_COMMAND(autodetonate_toggle, "Turn off/on auto-backstab")
{
	AutoBool enable("AutoDetonate::Enable");
	enable = !enable;
	REPLY_TO_TARGET(return, "Auto-Detonate is now %s\n", enable ? "ON":"OFF");
}