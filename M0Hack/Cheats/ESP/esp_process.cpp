#include "esp.hpp"
#include "cdll_int.hpp"
#include "Helper/Format.hpp"
#include "GlobalVars.hpp"

void IESPHack::ProcessPlayer_SecondPass(const ITFPlayer player, float dist)
{
	PROFILE_USECTION("ESP::ProcessPlayer", M0PROFILER_GROUP::CHEAT_PROFILE);

	int team = static_cast<int>(player->TeamNum.get());
	const auto& PlayerESP = PlayerESPInfo[team - 2];

	ESPData data{ .Entity = player, .Distance = dist, .Type = ESPType::Player };
	IFormatter fmt(48U);

	data.AddEntityString(PlayerESP.DrawDistance, fmt("Distance: {:4d} HU"sv, static_cast<int>(dist)));
	data.AddEntityString(PlayerESP.DrawTeam, fmt("Team: {}"sv, TFTeamsNames::CStrings[team - 1]));

	//Draw Class
	{
		data.AddEntityString(PlayerESP.DrawClass, fmt("Class: {}"sv, TFClassNames[static_cast<size_t>(player->Class.get()) - 1]), data.Strings.begin());
	}

	//Draw Name
	if (PlayerESP.DrawName)
	{
		PlayerInfo info;
		if (Interfaces::EngineClient->GetPlayerInfo(player->entindex(), &info))
			data.AddEntityString(true, fmt("Name: {}"sv, info.Name), data.Strings.begin());
	}

	//Draw Uber
	{
		if (player->Class == TFClass::Medic && PlayerESP.DrawUberPerc)
		{
			const auto& Weapons = player->MyWeapons;
			for (size_t i = 0; Weapons[i] && i < 3; i++)
			{
				const IBaseWeapon pWep(Weapons[i]);
				if (pWep && pWep->IsClassID(EntClassID::CWeaponMedigun))
				{
					const float flcharge = floorf(pWep->ChargeLevel * 100.f);

					if (pWep->ItemDefinitionIndex != 998) // The Vaccinator
						data.AddEntityString(true, fmt("Uber: {}%%"sv, static_cast<int>(flcharge)));
					else data.AddEntityString(true, fmt("Uber: {}%% | Charges: {}"sv, static_cast<int>(flcharge), static_cast<int>(flcharge / 0.25f)));

					break;
				}
			}
		}
	}

	//Draw Conds
	{
		if (PlayerESP.DrawCond)
		{
			for (const auto& [cond, name] : TFCondMap)
				if (player->InCond(cond))
					data.AddEntityString(true, fmt("Cond: {}"sv, name));
		}
	}

	DrawData.emplace_back(std::move(data));
}

void IESPHack::ProcessPlayer_FirstPass(const ITFPlayer player, const Vector& my_pos)
{
	if (player->LifeState != PlayerLifeState::Alive)
		return;

	const int team = static_cast<int>(player->TeamNum.get()) - 2;
	if (team < 0)
		return;

	const auto& PlayerESP = PlayerESPInfo[team];
	if (!PlayerESP.Enable)
		return;

	const Vector& org = player->GetAbsOrigin();
	const float dist = static_cast<float>(org.DistTo(my_pos));

	if (PlayerESP.MaxDistance > 50.0f && PlayerESP.MaxDistance < static_cast<int>(dist))
		return;

	else if (PlayerESP.IgnoreCloak && player->InCond(TF_COND_STEALTHED))
		return;

	else if (!DrawTools::IsVectorInHudSpace(org))
		return;

	TFClass tfcls = player->Class;
	if (tfcls == TFClass::Unknown)
		return;

	ProcessPlayer_SecondPass(player, dist);
}


void IESPHack::ProcessBuilding(const EntClassID clsID, const IBaseObject entity, const float dist)
{
	const int team = static_cast<int>(entity->TeamNum.get()) - 2;
	if (team < 0)
		return;

	const auto& BuildingESP = BuildingESPInfo[team];
	if (!BuildingESP.Enable)
		return;
	else if (BuildingESP.MaxDistance > 50.0f && BuildingESP.MaxDistance < dist)
		return;

	PROFILE_USECTION("ESP::ProcessBuilding", M0PROFILER_GROUP::CHEAT_PROFILE);

	ESPData data{ .Entity = entity, .Distance = dist, .Type = ESPType::Building };
	IFormatter fmt(32U);

	data.AddEntityString(BuildingESP.DrawTeam, fmt("Team: {}"sv, TFTeamsNames::CStrings[team - 1]));

	switch (clsID)
	{
	case EntClassID::CObjectTeleporter:
	{
		data.AddEntityString(BuildingESP.DrawName, L"Teleporter"sv, data.Strings.cbegin());

		ITeleporter tele{ entity };
		switch (tele->State)
		{
		case TeleporterState::Building:
		{
			data.AddEntityString(BuildingESP.DrawBState, L"<Building>"sv);
			break;
		}

		case TeleporterState::Idle:
		{
			data.AddEntityString(BuildingESP.DrawBState, L"<Ready>"sv);
			break;
		}


		case TeleporterState::Recharging:	//Recharging
		{
			const float recharge_elapsed = tele->RechargeTime - Interfaces::GlobalVars->CurTime;
			data.AddEntityString(BuildingESP.DrawBState, fmt("Charging: {:1f}"sv, tele->RechargeTime.get()));
			break;
		}

		}

		break;
	}

	case EntClassID::CObjectSentrygun:
	{
		data.AddEntityString(BuildingESP.DrawName, L"Sentrygun"sv, data.Strings.cbegin());


		ISentryGun sentry{ entity };
		const bool player_controlled = sentry->IsPlayerControlled;
		const int shell = sentry->AmmoShells;
		const int maxshell = sentry->MaxAmmoShells;

		if (player_controlled)
			data.AddEntityString(BuildingESP.DrawBState, L"<Wrangled>"sv);

		data.AddEntityString(BuildingESP.DrawAmmo, fmt("Ammo: {} / {}"sv, shell, maxshell));

		if (entity->UpgradeLevel == 3)
			data.AddEntityString(BuildingESP.DrawAmmo, fmt("Rockets: {} / 20"sv, sentry->AmmoRockets.get()));

		break;
	}

	case EntClassID::CObjectDispenser:
	{
		data.AddEntityString(BuildingESP.DrawName, L"Dispenser"sv, data.Strings.cbegin());

		IDispenser disp{ entity };
		if (const int num_players = disp->HealingTargets->Count())
			data.AddEntityString(BuildingESP.DrawBState, fmt("Healing: {} Players"sv, num_players));

		const int num_ammo = disp->AmmoMetal;
		data.AddEntityString(BuildingESP.DrawBState, fmt("Ammo: {}"sv, num_ammo));
		break;
	}
	}
	
	if (BuildingESP.DrawLevel)
	{
		if (!entity->IsMiniBuilding) 
			data.AddEntityString(true, fmt("Level: {}"sv, entity->UpgradeLevel.get()));
		else	
			data.AddEntityString(true, L"<Mini>"sv);
	}
	
	data.AddEntityString(entity->HasSapper && BuildingESP.DrawBState, L"<Sapped>"sv);
	data.AddEntityString(entity->IsCarried && BuildingESP.DrawBState, L"<Carried>"sv);
	data.AddEntityString(entity->IsBuilding && BuildingESP.DrawBState, L"<Building>"sv);

	DrawData.emplace_back(std::move(data));
}


void IESPHack::ProcessObject(const EntClassID clsID, const IBaseEntity entity, const float dist)
{
	PROFILE_USECTION("ESP::ProcessObject", M0PROFILER_GROUP::CHEAT_PROFILE);
	ESPData data{ .Entity = entity, .Distance = dist, .Type = ESPType::Objects };

	switch (clsID)
	{
	case EntClassID::CTFProjectile_Rocket:
	case EntClassID::CTFProjectile_SentryRocket:
	{
		if (ObjectESPInfo.DrawRockets)
			data.AddEntityString(ObjectESPInfo.DrawName, L"Rocket"sv);
		else return;
		break;
	}
	case EntClassID::CTFGrenadePipebombProjectile:
	{
		IBaseWeapon Proj{ entity };
		switch (Proj->Type)
		{
		case EntGrenadeProjType::Detonate:
		case EntGrenadeProjType::DetonateRemote:
		{
			if (ObjectESPInfo.DrawStickies)
				data.AddEntityString(ObjectESPInfo.DrawStickies && ObjectESPInfo.DrawName, L"Stickie bomb"sv);
			else return;
			break;
		}

		default:
		{
			if (ObjectESPInfo.DrawPipes)
				data.AddEntityString(ObjectESPInfo.DrawName, L"Pipe Bomb"sv);
			else return;
			break;
		}
		}
		
		break;
	}
	case EntClassID::CTFAmmoPack:
	{
		if (ObjectESPInfo.DrawPacks)
			data.AddEntityString(ObjectESPInfo.DrawName, L"Ammo-pack"sv);
		else return;
		break;
	}

	case EntClassID::Reserved_HealthPack:
	{
		data.AddEntityString(ObjectESPInfo.DrawName, L"Health-pack"sv);
		break;
	}
	}

	DrawData.emplace_back(std::move(data));
}


void IESPHack::ProcessEntity(const IBaseEntity entity, const Vector& my_pos)
{
	const EntClassID cls = entity->GetClientClass()->ClassID;
	switch (cls)
	{
	case EntClassID::CObjectDispenser:
	case EntClassID::CObjectTeleporter:
	case EntClassID::CObjectSentrygun:
	{
		const Vector& org = entity->GetAbsOrigin();
		if (!DrawTools::IsVectorInHudSpace(org))
			return;

		const float dist = static_cast<float>(my_pos.DistTo(org));

		ProcessBuilding(cls, entity, dist);
		return;
	}
	case EntClassID::CTFProjectile_Rocket:
	case EntClassID::CTFProjectile_SentryRocket:
	case EntClassID::CTFGrenadePipebombProjectile:
	case EntClassID::CTFAmmoPack:
	case EntClassID::CBaseAnimating:
	{
		const Vector& org = entity->GetAbsOrigin();
		if (!DrawTools::IsVectorInHudSpace(org))
			return;

		const float dist = static_cast<float>(org.DistTo(my_pos));
		if (ObjectESPInfo.MaxDistance > 50.0f && ObjectESPInfo.MaxDistance < dist)
			return;

		if (ObjectESPInfo.Enable)
		{
			if (ObjectESPInfo.MaxDistance > 50.0f && ObjectESPInfo.MaxDistance < dist)
				return;
			else
			{
				if (cls == EntClassID::CBaseAnimating)
				{
					if (ObjectESPInfo.DrawPacks && entity->IsHealthKit())
						ProcessObject(EntClassID::Reserved_HealthPack, entity, dist);
				}
				else
					ProcessObject(cls, entity, dist);
			}
		}
		break;
	}
	}
}