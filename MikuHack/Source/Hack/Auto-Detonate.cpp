
#include "../Interfaces/HatCommand.h"
#include "../Source/Main.h"
#include "../Helpers/VTable.h"
#include "../Interfaces/IVEClientTrace.h"
#include "../Interfaces/IClientMode.h"
#include "../Helpers/Timer.h"

class AutoDetonate : public MenuPanel
{
public:	//	AutoDetonate
	AutoBool bEnabled		{ "AutoDetonate::Enable", true };
	AutoBool bIgnoreStealth	{ "AutoDetonate::bIgnoreStealth", true };

public:	//	Globals::IGlobalHooks
	AutoDetonate() 
	{
		IGlobalEvent::CreateMove::Hook::Register(std::bind(&AutoDetonate::OnCreateMove, this, std::placeholders::_1));
	};
	HookRes OnCreateMove(bool&);

public:	//	ExtraPanel
	void OnRenderExtra() override final
	{
		ImGui::Checkbox("Auto-Detonate", bEnabled.get());
	}

	void JsonCallback(Json::Value& cfg, bool read) override
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
} static autodeto;


HookRes AutoDetonate::OnCreateMove(bool& ret)
{
	if (!bEnabled)
		return HookRes::Continue;

	if (pLocalPlayer->GetClass() != TFClass::Demoman)
		return HookRes::Continue;
	
	static Timer timer_leftclick;
	static Timer timer_checkscottish;

	if (Globals::m_pUserCmd->buttons & IN_ATTACK)
		timer_leftclick.update();

	if (!timer_leftclick.has_elapsed(500))
		return HookRes::Continue;

	static bool IsScottishResistance = false;
	if (timer_checkscottish.trigger_if_elapsed(2500))
	{
		IBaseHandle& pHndl = pLocalPlayer->GetWeaponList()[1];
		IBaseObject* pWeapon = reinterpret_cast<IBaseObject*>(clientlist->GetClientEntityFromHandle(pHndl));
		IsScottishResistance = pWeapon && pWeapon->GetItemDefinitionIndex() == 130;
	}

	static std::vector<IBaseObject*> stickies;	stickies.clear();
	static std::vector<ITFPlayer*> players;		players.clear();
	
	ITFPlayer* pPlayer;
	ITFPlayer* pMe = pLocalPlayer;

	{
		IClientShared* pEnt;
		for (const auto& infos = ent_infos.GetInfos(); 
			const MyClientCacheList& cache : infos)
		{
			pEnt = cache.pEnt;
			if(cache.flag == EntFlag::EF_EXTRA && pEnt->IsClassID(ClassID_CTFGrenadePipebombProjectile) && *pEnt->GetEntProp<int>("m_iType") == 1)
			{
				stickies.push_back(reinterpret_cast<IBaseObject*>(pEnt));
			}
			else if (cache.flag == EntFlag::EF_PLAYER)
			{
				pPlayer = reinterpret_cast<ITFPlayer*>(pEnt);

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

	ICollideable* pCollide;
	trace_t tr;

	pCollide = pLocalPlayer->GetCollideable();
	Vector position;
	bool bFound = false;
	static Vector sec1, sec2;
	Trace::ILocalFilterSimple filter;

	for (const IBaseObject* stickie : stickies)
	{
		Vector vecOrigin = stickie->GetAbsOrigin();
		{
			position = pLocalPlayer->GetAbsOrigin() + ((pCollide->OBBMins() + pCollide->OBBMaxs()) / 2);
			if (vecOrigin.DistTo(position) < 170.f)
			{
				Trace::TraceLine(vecOrigin, position, COLLISION_GROUP_PLAYER_MOVEMENT, &tr, &filter);
				if (tr.fraction >= 1.0)
					return HookRes::Continue;
			}
		}

		for (const ITFPlayer* target: players)
		{
			if (bFound)
				break;

			position = target->GetAbsOrigin() + ((pCollide->OBBMins() + pCollide->OBBMaxs()) / 2);

			if (vecOrigin.DistTo(position) > 150.f)
				continue;

			Trace::TraceLine(vecOrigin, position, COLLISION_GROUP_PLAYER_MOVEMENT, &tr, &filter);

			if (tr.fraction < 1.0)
				continue;

			if (IsScottishResistance)
			{
				QAngle angRes = GetAimAngle(stickie->GetAbsOrigin());
				Globals::m_pUserCmd->viewangles = angRes;
				ret = false;
			}

			bFound = true;
			break;
		}
	}

	if (bFound)
		Globals::m_pUserCmd->buttons |= IN_ATTACK2;

	return HookRes::Continue;
}


HAT_COMMAND(autodetonate_toggle, "Turn off/on auto-backstab")
{
	autodeto.bEnabled = !autodeto.bEnabled;
	REPLY_TO_TARGET(return, "Auto-Detonate is now %s\n", autodeto.bEnabled ? "ON":"OFF");
}