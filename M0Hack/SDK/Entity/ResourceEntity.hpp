#pragma once

#include "BaseEntity.hpp"
#include "EntityList.hpp"


class TFPlayerResourceEntity
{
public:
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iPing",				Ping);
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iScore",				Score);
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iTotalScore",		TotalScore);
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iDeaths",			Deaths);
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iTeam",				Team);
	GAMEPROP_DECL_RECV(bool[MAX_PLAYERS],	"CTFPlayerResource",	"m_bAlive",				IsAlive);
	GAMEPROP_DECL_RECV(bool[MAX_PLAYERS],	"CTFPlayerResource",	"m_bValid",				IsValid);
	GAMEPROP_DECL_RECV(bool[MAX_PLAYERS],	"CTFPlayerResource",	"m_bConnected",			IsConnected);

	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iDamage",			Damage);
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iDamageBoss",		DamageBoss);
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iDamageAssist",		DamageAssist);

	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iHealth",			Health);
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iMaxHealth",			MaxHealth);
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iMaxBuffedHealth",	MaxBuffedHealth);

	GAMEPROP_DECL_RECV(float[MAX_PLAYERS],	"CTFPlayerResource",	"m_flNextRespawnTime",	NextSpawnTime);
	GAMEPROP_DECL_RECV(int[MAX_PLAYERS],	"CTFPlayerResource",	"m_iChargeLevel",		ChargeLevel);

	bool IsValidPlayer(int index) const noexcept
	{
		return IsConnected[index] && IsValid[index];
	}
};

class TFMonsterResourceEntity
{
public:
	GAMEPROP_DECL_RECV(int,		"CMonsterResource",		"m_iBossHealthPercentageByte",		HealthPerc);
	GAMEPROP_DECL_RECV(int,		"CMonsterResource",		"m_iBossStunPercentageByte",		StunPerc);

	GAMEPROP_DECL_RECV(int,		"CMonsterResource",		"m_iBossState",		State);
};

namespace TFResourceEntity
{
	extern TFPlayerResourceEntity*	Player;
	extern TFMonsterResourceEntity* Monster;
}