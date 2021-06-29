#include "Cheats/Main.hpp"
#include "Trace.hpp"
#include "Hooks/CreateMove.hpp"
#include "cdll_int.hpp"

class AutoDetonate
{
public:
	AutoDetonate();
	MHookRes OnCreateMove(UserCmd*);

private:
	_NODISCARD std::vector<IBaseWeapon> CollectMyStickies();
	_NODISCARD std::vector<ITFPlayer> CollectEnemies();

	M0Config::Bool Enable{ "AutoDetonate.Enable", true, "Automatically Detonate sticky bomb" };
	M0Config::Bool IgnoreStealth{ "AutoDetonate.Ignore Stealth", true, M0CONFIG_NULL_DESCRIPTION };

	M0HookManager::Policy::CreateMove CreateMove;
} static autostab;

AutoDetonate::AutoDetonate()
{
	M0EventManager::AddListener(
		EVENT_KEY_LOAD_DLL,
		[this](M0EventData*)
		{
			CreateMove.init();
			CreateMove->AddPreHook(HookCall::Late, std::bind(&AutoDetonate::OnCreateMove, this, std::placeholders::_2));
		},
		EVENT_NULL_NAME
	);

	M0EventManager::AddListener(
		EVENT_KEY_RENDER_EXTRA,
		[this](M0EventData*)
		{
			if (ImGui::CollapsingHeader("Auto detonate"))
			{
				ImGui::Checkbox("Enabled", Enable.data());
				ImGui::SameLineHelp(Enable);

				ImGui::Checkbox("No stealth", IgnoreStealth.data());
				ImGui::SameLineHelp(IgnoreStealth);
			}
		},
		EVENT_NULL_NAME
	);
}

std::vector<IBaseWeapon> AutoDetonate::CollectMyStickies()
{
	std::vector<IBaseWeapon> stickies;
	stickies.reserve(8);

	for (int i = Interfaces::GlobalVars->MaxClients + 1; i < IBaseEntityInternal::GetHighestEntityIndex(); i++)
	{
		IBaseWeapon ent(i);
		if (!ent)
			continue;

		if (ent->IsClassID(EntClassID::CTFGrenadePipebombProjectile) && ent->Type == EntGrenadeProjType::Detonate)
			stickies.emplace_back(ent);
	}

	return stickies;
}

std::vector<ITFPlayer> AutoDetonate::CollectEnemies()
{
	std::vector<ITFPlayer> players;
	ILocalPlayer pMe;

	players.reserve(8);

	for (int i = 1; i < Interfaces::GlobalVars->MaxClients; i++)
	{
		ITFPlayer player(i);
		if (!player)
			continue;

		if (player->TeamNum == pMe->TeamNum)
			continue;

		else if (player->InCond(ETFCond::TF_COND_STEALTHED) && IgnoreStealth)
			continue;

		else if (player->InCond(TF_COND_INVULNERABLE) || player->InCond(TF_COND_PHASE))
			continue;

		else players.emplace_back(player);
	}

	return players;
}


MHookRes AutoDetonate::OnCreateMove(UserCmd* cmd)
{
	if (!Enable)
		return { };

	const ILocalPlayer pMe;
	if (pMe->Class != TFClass::Demoman)
		return { };

	{
		static AutoCTimer<0.5f> check_leftclick;
		if (cmd->Buttons & IN_ATTACK)
			check_leftclick.update();

		if (check_leftclick.has_elapsed())
			return { };
	}

	static AutoCTimer<2.5f> check_hasscottish;
	static bool has_scottish = false;
	if (check_hasscottish.trigger_if_elapsed())
	{
		IBaseWeapon pWpn(pMe->MyWeapons[1]);
		has_scottish = pWpn && pWpn->ItemDefinitionIndex == 130;
	}

	auto stickies = CollectMyStickies();
	if (stickies.empty())
		return { };
	
	auto players = CollectMyStickies();
	if (stickies.empty())
		return { };

	const Vector my_eyepos = pMe->EyePosition();
	const Vector mins = pMe->CollisionProp->OBBMins(), maxs = pMe->CollisionProp->OBBMaxs();
	const Vector middle = (mins + maxs) / 2;
	
	GameTrace tr;
	Trace::LocalFilterSimple localfilter;

	bool change_hr = false;
	for (const IBaseWeapon stickie : stickies)
	{
		const Vector stickie_pos = stickie->VecOrigin;
		{
			const Vector position = pMe->GetAbsOrigin() + middle;
			if (stickie_pos.DistTo(position) < 170.f)
			{
				Trace::TraceLine(stickie_pos, position, MASK_SOLID, &tr, &localfilter);
				if (tr.Fraction >= 1.0)
					return { };
			}
		}

		for (const ITFPlayer player : players)
		{
			const Vector position = player->VecOrigin + middle;

			if (stickie_pos.DistTo(position) > 150.f)
				continue;

			Trace::TraceLine(stickie_pos, position, MASK_SOLID, &tr, &localfilter);

			if (tr.Fraction < 1.0)
				continue;

			if (has_scottish)
			{
				QAngle angRes = VectorHelper::GetAimAngle(my_eyepos, stickie_pos);
				cmd->ViewAngles = angRes;
				CreateMove->SetReturnInfo(false);
				change_hr = true;
			}

			cmd->Buttons |= IN_ATTACK2;
			break;
		}
	}

	return change_hr ? MHookRes(bitmask::to_mask(HookRes::ChangeReturnValue)) : MHookRes{ };
}