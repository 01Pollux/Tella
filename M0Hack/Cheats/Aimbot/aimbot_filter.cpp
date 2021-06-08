#include "aimbot.hpp"

#include "ResourceEntity.hpp"

#include "GlobalVars.hpp"

constexpr double bad_val = 999.9 * 999.9;

template<bool IsPlayer>
[[nodiscard]] double
Filter_GetTargetValue(
	const AimTargetPriority priority, 
	const float min_fov, 
	const IBaseEntity pEnt, 
	const UserCmd* cmd, 
	const Vector& my_pos
)
{
	double cur = bad_val;

	if constexpr (!IsPlayer)
	{
		const IBaseObject pObj(pEnt);
		switch (priority)
		{
		case AimTargetPriority::Fov:
		case AimTargetPriority::FovAndDist:
		{
			cur = VectorHelper::GetFOV(my_pos, pObj->VecOrigin.get(), cmd->ViewAngles);
			if (cur > min_fov)
				return bad_val;
			break;
		}

		case AimTargetPriority::Closest:
		{
			cur = pObj->VecOrigin.get().DistToSqr(my_pos);
			break;
		}

		case AimTargetPriority::Furthest:
		{
			cur = -pObj->VecOrigin.get().DistToSqr(my_pos);
			break;
		}

		// if entity doesn't have 'm_iHealth' prop, skip it 
		case AimTargetPriority::Highest_Health:
		{
			if (pEnt->HasEntProp<PropType::Recv>("m_iHealth"))
				cur = static_cast<double>(pObj->Health.get());
			break;
		}

		case AimTargetPriority::Lowest_Health:
		{
			if (pEnt->HasEntProp<PropType::Recv>("m_iHealth"))
				cur = -static_cast<double>(pObj->Health.get());
			break;
		}

		default:
			break;
		}
	}
	else
	{
		const ITFPlayer pPlayer(pEnt);
		switch (priority)
		{
		case AimTargetPriority::Fov:
		case AimTargetPriority::FovAndDist:
		{
			cur = VectorHelper::GetFOV(my_pos, pPlayer->VecOrigin.get(), cmd->ViewAngles);
			if (cur > min_fov)
				return bad_val;
			break;
		}

		case AimTargetPriority::Closest:
		{
			cur = pPlayer->VecOrigin.get().DistToSqr(my_pos);
			break;
		}

		case AimTargetPriority::Furthest:
		{
			cur = -pPlayer->VecOrigin.get().DistToSqr(my_pos);
			break;
		}

		case AimTargetPriority::Highest_Health:
		{
			cur = -static_cast<double>(pPlayer->CurrentHealth.get());
			break;
		}

		case AimTargetPriority::Lowest_Health:
		{
			cur = static_cast<double>(pPlayer->CurrentHealth.get());
			break;
		}

		case AimTargetPriority::Highest_Score:
		{
			cur = -static_cast<double>(TFResourceEntity::Player->Score[pEnt->entindex()]);
			break;
		}

		case AimTargetPriority::Lowest_Score:
		{
			cur = static_cast<double>(TFResourceEntity::Player->Score[pEnt->entindex()]);
			break;
		}
		}
	}

	return cur;
}


void IAimbotHack::GetBestPlayer(const UserCmd* cmd)
{
	ILocalPlayer pMe;
	ITFPlayer pBest;
	const TFTeam my_team = pMe->TeamNum;

	const Vector my_eyepos = pMe->EyePosition();

	double nearest = bad_val, best = bad_val;

	int shoot_range;
	if (IBaseWeapon pWpn = pMe->ActiveWeapon.get())
	{
		if (pWpn->GetWeaponSlot() == 2)
			shoot_range = pWpn->GetSwingRange();
		else shoot_range = GetShootRange(pWpn->GetClientClass()->ClassID);
	}
	else shoot_range = Vars.MaxRange;

	for (int i = 1; i <= Interfaces::GlobalVars->MaxClients; i++)
	{
		ITFPlayer player(i);
		if (!player)
			continue;

		if (!IsValidPlayer(player))
			continue;

		if (player->TeamNum == my_team)
			continue;

		const Vector& player_pos = player->GetAbsOrigin();
		const double delta_dist = my_eyepos.DistTo(player_pos);
		if (delta_dist >= shoot_range)
			continue;

		const double cur = Filter_GetTargetValue<true>(Vars.TargetPriority, Vars.FOV, player, cmd, my_eyepos);

		if (best > cur || cur != bad_val && Vars.TargetPriority == AimTargetPriority::FovAndDist)
		{
			if (Vars.TargetPriority == AimTargetPriority::FovAndDist)
			{
				const double dist = player->GetAbsOrigin().DistToSqr(my_eyepos);
				if (dist >= nearest)
					continue;
				nearest = dist;
			}

			best = cur;
			pBest = player;
		}
	}

	AimbotState.set(static_cast<IBaseEntity>(pBest), true);
}

void IAimbotHack::GetBestEntity(const UserCmd* cmd)
{
	ILocalPlayer pMe;
	const TFTeam my_team = pMe->TeamNum;

	IBaseEntity pBestEnt;

	GetBestPlayer(cmd);
	const Vector& my_eyepos = pMe->GetAbsOrigin();

	double best = bad_val, nearest = bad_val;

	int shoot_range = 0;
	if (IBaseWeapon pWpn = pMe->ActiveWeapon.get())
	{
		if (pWpn->GetWeaponSlot() == 2)
			shoot_range = pWpn->GetSwingRange();
		else shoot_range = GetShootRange(pWpn->GetClientClass()->ClassID);
	}
	else shoot_range = Vars.MaxRange;

	for (int i = Interfaces::GlobalVars->MaxClients + 1; i <= IBaseEntityInternal::GetHighestEntityIndex(); i++)
	{
		IBaseEntity pEnt(i);
		if (!pEnt)
			continue;

		if (!IsValidEntity(pEnt))
			continue;

		//if (pEnt->GetTeam() == my_team)
		//	continue;

		const Vector& player_pos = pEnt->GetAbsOrigin();
		const double delta_dist = my_eyepos.DistTo(player_pos);

		if (delta_dist >= shoot_range)
			continue;

		const double cur = Filter_GetTargetValue<false>(Vars.TargetPriority, Vars.FOV, pEnt, cmd, my_eyepos);

		if (best > cur || cur != bad_val && Vars.TargetPriority == AimTargetPriority::FovAndDist)
		{
			if (Vars.TargetPriority == AimTargetPriority::FovAndDist)
			{
				double dist = pEnt->GetAbsOrigin().DistToSqr(my_eyepos);
				if (dist >= nearest)
					continue;
				nearest = dist;
			}

			best = cur;
			pBestEnt = pEnt;
		}
	}

	if (pBestEnt)
	{
		const double playerval = Filter_GetTargetValue<true>(Vars.TargetPriority, Vars.FOV, AimbotState.get().target, cmd, my_eyepos);
		const double entval = Filter_GetTargetValue<false>(Vars.TargetPriority, Vars.FOV, pBestEnt, cmd, my_eyepos);

		if (entval > playerval)
			AimbotState.set(pBestEnt, false);
	}
}


IAimbotHack::IAimbotState::IAimData& IAimbotHack::GetBestTarget(const UserCmd* cmd)
{
	auto& data = AimbotState.get();
	if (Vars.TargetLock.should_retarget())
	{
		if (IBaseEntity pEnt(data.target_hndl); pEnt)
		{
			if (data.is_player && ITFPlayer(pEnt.get())->LifeState != PlayerLifeState::Alive)
				data.target_hndl = nullptr;
			else
			{
				data.target = pEnt;
				return data;
			}
		}
		else data.target_hndl = nullptr;
	}

	// Choose entity thats (nearest / best score / ...)
	if (Vars.PlayerOnly)
		GetBestPlayer(cmd);
	else GetBestEntity(cmd);

	return data;
}


bool IAimbotHack::IsValidPlayer(const ITFPlayer pPlayer)
{
	if (pPlayer->LifeState != PlayerLifeState::Alive)
		return false;

	if (Vars.MedicOnly && pPlayer->Class != TFClass::Medic)
		return false;

	if (!Vars.BypassBadConds)
	{
		constexpr ETFCond BadConds[]{
			TF_COND_STEALTHED_USER_BUFF,
			TF_COND_BULLET_IMMUNE,
			TF_COND_INVULNERABLE,
			TF_COND_DEFENSEBUFF,
			TF_COND_DEFENSEBUFF_HIGH,
			TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,
			TF_COND_FEIGN_DEATH
		};

		for (const ETFCond cond : BadConds)
		{
			if (pPlayer->InCond(cond))
				return false;
		}
	}

	if (!Vars.BypassCloak && pPlayer->InCond(TF_COND_DISGUISED))
		return false;

	return true;
}

bool IAimbotHack::IsValidEntity(const IBaseEntity pEnt)
{
	const Vector& player_pos = pEnt->GetAbsOrigin();

	switch (pEnt->GetClientClass()->ClassID)
	{
	case EntClassID::CObjectSentrygun:
	case EntClassID::CObjectDispenser:
	case EntClassID::CObjectTeleporter:
	{
		if (!Vars.AimForBuildings)
			return true;
		break;
	}

	case EntClassID::CTFBaseRocket:
	case EntClassID::CTFProjectile_Arrow:
	case EntClassID::CTFProjectile_BallOfFire:
	case EntClassID::CTFProjectile_Cleaver:
	case EntClassID::CTFProjectile_EnergyBall:
	case EntClassID::CTFProjectile_Jar:
	case EntClassID::CTFProjectile_JarMilk:
	case EntClassID::CTFProjectile_JarGas:
	case EntClassID::CTFProjectile_Rocket:
	case EntClassID::CTFProjectile_SentryRocket:
	case EntClassID::CTFGrenadePipebombProjectile:
	{
		if (Vars.AimForProjectiles)
			return true;
		break;
	}

	default:
		break;
	}

	return false;
}


IAimbotHack::IAimbotState::IAimData& IAimbotHack::GetTargetNoReset()
{
	auto& data = AimbotState.get();
	if (IBaseEntity pEnt(data.target_hndl); pEnt)
		data.target = pEnt;

	return data;
}

void IAimbotHack::Invalidate()
{
	AimbotState.get().invalidate();
	Vars.TargetLock.Invalid_ = true;
}
