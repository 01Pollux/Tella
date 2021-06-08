#pragma once

#include "BaseEntity.hpp"
#include "Helper/Offsets.hpp"


class Vector;

enum ETFCond
{
	TF_COND_INVALID = -1,

	TF_COND_AIMING,
	TF_COND_ZOOMED,
	TF_COND_DISGUISING,
	TF_COND_DISGUISED,
	TF_COND_STEALTHED,
	TF_COND_INVULNERABLE,
	TF_COND_TELEPORTED,
	TF_COND_TAUNTING,
	TF_COND_INVULNERABLE_WEARINGOFF,
	TF_COND_STEALTHED_BLINK,
	TF_COND_SELECTED_TO_TELEPORT,
	TF_COND_CRITBOOSTED,
	TF_COND_TMPDAMAGEBONUS,
	TF_COND_FEIGN_DEATH,
	TF_COND_PHASE,
	TF_COND_STUNNED,
	TF_COND_OFFENSEBUFF,
	TF_COND_SHIELD_CHARGE,
	TF_COND_DEMO_BUFF,
	TF_COND_ENERGY_BUFF,
	TF_COND_RADIUSHEAL,
	TF_COND_HEALTH_BUFF,
	TF_COND_BURNING,
	TF_COND_HEALTH_OVERHEALED,
	TF_COND_URINE,
	TF_COND_BLEEDING,
	TF_COND_DEFENSEBUFF,
	TF_COND_MAD_MILK,
	TF_COND_MEGAHEAL,
	TF_COND_REGENONDAMAGEBUFF,
	TF_COND_MARKEDFORDEATH,
	TF_COND_NOHEALINGDAMAGEBUFF,
	TF_COND_SPEED_BOOST,
	TF_COND_CRITBOOSTED_PUMPKIN,
	TF_COND_CRITBOOSTED_USER_BUFF,
	TF_COND_CRITBOOSTED_DEMO_CHARGE,
	TF_COND_SODAPOPPER_HYPE,
	TF_COND_CRITBOOSTED_FIRST_BLOOD,
	TF_COND_CRITBOOSTED_BONUS_TIME,
	TF_COND_CRITBOOSTED_CTF_CAPTURE,
	TF_COND_CRITBOOSTED_ON_KILL,
	TF_COND_CANNOT_SWITCH_FROM_MELEE,
	TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,
	TF_COND_REPROGRAMMED,
	TF_COND_CRITBOOSTED_RAGE_BUFF,
	TF_COND_DEFENSEBUFF_HIGH,
	TF_COND_SNIPERCHARGE_RAGE_BUFF,
	TF_COND_DISGUISE_WEARINGOFF,
	TF_COND_MARKEDFORDEATH_SILENT,
	TF_COND_DISGUISED_AS_DISPENSER,
	TF_COND_SAPPED,
	TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED,
	TF_COND_INVULNERABLE_USER_BUFF,
	TF_COND_HALLOWEEN_BOMB_HEAD,
	TF_COND_HALLOWEEN_THRILLER,
	TF_COND_RADIUSHEAL_ON_DAMAGE,
	TF_COND_CRITBOOSTED_CARD_EFFECT,
	TF_COND_INVULNERABLE_CARD_EFFECT,
	TF_COND_MEDIGUN_UBER_BULLET_RESIST,
	TF_COND_MEDIGUN_UBER_BLAST_RESIST,
	TF_COND_MEDIGUN_UBER_FIRE_RESIST,
	TF_COND_MEDIGUN_SMALL_BULLET_RESIST,
	TF_COND_MEDIGUN_SMALL_BLAST_RESIST,
	TF_COND_MEDIGUN_SMALL_FIRE_RESIST,
	TF_COND_STEALTHED_USER_BUFF,
	TF_COND_MEDIGUN_DEBUFF,
	TF_COND_STEALTHED_USER_BUFF_FADING,
	TF_COND_BULLET_IMMUNE,
	TF_COND_BLAST_IMMUNE,
	TF_COND_FIRE_IMMUNE,
	TF_COND_PREVENT_DEATH,
	TF_COND_MVM_BOT_STUN_RADIOWAVE,
	TF_COND_HALLOWEEN_SPEED_BOOST,
	TF_COND_HALLOWEEN_QUICK_HEAL,
	TF_COND_HALLOWEEN_GIANT,
	TF_COND_HALLOWEEN_TINY,
	TF_COND_HALLOWEEN_IN_HELL,
	TF_COND_HALLOWEEN_GHOST_MODE,
	TF_COND_MINICRITBOOSTED_ON_KILL,
	TF_COND_OBSCURED_SMOKE,
	TF_COND_PARACHUTE_ACTIVE = 8,
	TF_COND_BLASTJUMPING,
	TF_COND_HALLOWEEN_KART,
	TF_COND_HALLOWEEN_KART_DASH,
	TF_COND_BALLOON_HEAD,
	TF_COND_MELEE_ONLY,
	TF_COND_SWIMMING_CURSE,
	TF_COND_FREEZE_INPUT,
	TF_COND_HALLOWEEN_KART_CAGE,
	TF_COND_DONOTUSE_0,
	TF_COND_RUNE_STRENGTH,
	TF_COND_RUNE_HASTE,
	TF_COND_RUNE_REGEN,
	TF_COND_RUNE_RESIST,
	TF_COND_RUNE_VAMPIRE,
	TF_COND_RUNE_REFLECT,
	TF_COND_RUNE_PRECISION,
	TF_COND_RUNE_AGILITY,
	TF_COND_GRAPPLINGHOOK,
	TF_COND_GRAPPLINGHOOK_SAFEFALL,
	TF_COND_GRAPPLINGHOOK_LATCHED,
	TF_COND_GRAPPLINGHOOK_BLEEDING,
	TF_COND_AFTERBURN_IMMUNE,
	TF_COND_RUNE_KNOCKOUT,
	TF_COND_RUNE_IMBALANCE,
	TF_COND_CRITBOOSTED_RUNE_TEMP,
	TF_COND_PASSTIME_INTERCEPTION,
	TF_COND_SWIMMING_NO_EFFECTS,
	TF_COND_PURGATORY,
	TF_COND_RUNE_KING,
	TF_COND_RUNE_PLAGUE,
	TF_COND_RUNE_SUPERNOVA,
	TF_COND_PLAGUE,
	TF_COND_KING_BUFFED,
	TF_COND_TEAM_GLOWS,
	TF_COND_KNOCKED_INTO_AIR,
	TF_COND_COMPETITIVE_WINNER,
	TF_COND_COMPETITIVE_LOSER,
	TF_COND_HEALING_DEBUFF,
	TF_COND_PASSTIME_PENALTY_DEBUFF,
	TF_COND_GRAPPLED_TO_PLAYER,
	TF_COND_GRAPPLED_BY_PLAYER,
	TF_COND_PARACHUTE_DEPLOYED,
	TF_COND_GAS,
	TF_COND_BURNING_PYRO,
	TF_COND_ROCKETPACK,
	TF_COND_LOST_FOOTING,
	TF_COND_AIR_CURRENT,
	TF_COND_HALLOWEEN_HELL_HEAL,

	TF_COND_COUNT,
};

struct TFCondAndName
{
	using CondName = const char*;

	ETFCond		Cond;
	CondName	Name;

	constexpr TFCondAndName(const ETFCond& cond, const CondName& name) noexcept : Cond(cond), Name(name) { };
};

constexpr TFCondAndName TFCondMap[] =
{
	{ TF_COND_MEDIGUN_UBER_BLAST_RESIST,	"Blast Resist" },
	{ TF_COND_FEIGN_DEATH,					"Dead Ringer"},
	{ TF_COND_MEDIGUN_UBER_FIRE_RESIST,		"Fire Resist"},
	{ TF_COND_BLAST_IMMUNE,					"Blast Resist"},
	{ TF_COND_BULLET_IMMUNE,				"Bullet Resist"},
	{ TF_COND_INVULNERABLE,					"Uber"},
	{ TF_COND_TELEPORTED,					"Teleported"},
	{ TF_COND_SPEED_BOOST,					"Speed Boosted"},
	{ TF_COND_CRITBOOSTED,					"Crit Boosted"},
	{ TF_COND_CRITBOOSTED_DEMO_CHARGE,		"Charging"},
	{ TF_COND_DEFENSEBUFF,					"Defense Buff"},
	{ TF_COND_DEFENSEBUFF_HIGH,				"Defense Buff HIGH"},
	{ TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,	"Defense Buff CRIT"},
	{ TF_COND_URINE,						"Jarated"},
	{ TF_COND_TAUNTING,						"Taunting"}
};

static const char* TFClassNames[] =
{
	"Scout",
	"Sniper",
	"Soldier",
	"Demoman",
	"Medic",
	"Heavy",
	"Pyro",
	"Spy",
	"Engineer"
};

enum class PlayerLifeState : uint8_t
{
	Alive,
	Dying,
	Dead
};

enum class TFClass
{
	Unknown,

	Scout,
	Sniper,
	Soldier,
	Demoman,
	Medic,
	Heavy,
	Pyro,
	Spy,
	Engineer,

	Count,
};

enum class TFStreakType
{
	Kills,
	KillsAll,
	Ducks,
	DuckLevelUp,

	Count,
};

class TFPlayerStreak
{
	using streak_type = std::underlying_type_t<TFStreakType>;
public:
	int& operator[](TFStreakType i) noexcept { return Streaks[static_cast<streak_type>(i)]; };
	int operator[](TFStreakType i) const noexcept { return Streaks[static_cast<streak_type>(i)]; };

	void set(TFStreakType i, const int val) noexcept { Streaks[static_cast<streak_type>(i)] = val; }
	int get(TFStreakType i) const noexcept { return Streaks[static_cast<streak_type>(i)]; }

private:
	int Streaks[static_cast<streak_type>(TFStreakType::Count)];
};


class ITFPlayerShared;

struct ITFPlayerClass
{
public:
	TFClass Class;

	char ClassIcon[260];
	char CustomModel[260];

	Vector CustomModelOffset;
	QAngle CustomModelRotation;

	bool CustomModelRotates;
	bool CustomModelRotationSet;
	bool CustomModelVisibleToSelf;
	bool UseClassAnimation;

	int ClassModelParity;
	//int OldClassModelParity;
};

class ITFPlayerInternal : public IBaseEntityInternal
{
public:
	static ITFPlayerInternal* GetEntity(int index)
	{
		return static_cast<ITFPlayerInternal*>(IBaseEntityInternal::GetEntity(index));
	}
	static ITFPlayerInternal* GetEntity(IBaseHandle hndl)
	{
		return static_cast<ITFPlayerInternal*>(IBaseEntityInternal::GetEntity(hndl));
	}
	static ITFPlayerInternal* GetLocalPlayer()
	{
		return static_cast<ITFPlayerInternal*>(IBaseEntityInternal::GetLocalPlayer());
	}

	static ITFPlayerInternal* FromShared(ITFPlayerShared* pSharedEnt)
	{
		return *(reinterpret_cast<ITFPlayerInternal**>(pSharedEnt) + Offsets::IBaseEntity::TFPlayerShared::m_pOuter);
	}

	Vector EyePosition() const
	{
		return VecOrigin.get() + ViewOffset;
	}

	void EyeVectors(Vector* fwd = nullptr, Vector* right = nullptr, Vector* up = nullptr) const
	{
		VectorHelper::AngleVectors(EyeAngles.get(), fwd, right, up);
	}

	bool InCond(ETFCond cond) const;

	void AddCond(ETFCond cond, float duration = -1.0);

	void RemoveCond(ETFCond cond);

	GAMEPROP_DECL_RECV(ITFPlayerShared,			"CTFPlayer", "m_Shared",				PlayerShared);
	GAMEPROP_DECL_RECV(ITFPlayerClass,			"CTFPlayer", "m_PlayerClass",			PlayerClass);

	GAMEPROP_DECL_RECV(int,						"CTFPlayer", "m_nDisguiseTeam",			DisguiseTeam);
	GAMEPROP_DECL_RECV(int,						"CTFPlayer", "m_nDisguiseClass",		DisguiseClass);
	GAMEPROP_DECL_RECV(int,						"CTFPlayer", "m_iDisguiseTargetIndex",	DisguiseTargetIndex);
	GAMEPROP_DECL_RECV(int,						"CTFPlayer", "m_iDisguiseHealth",		DisguiseHealth);
	GAMEPROP_DECL_RECV(IHandleEntity,			"CTFPlayer", "m_hDisguiseWeapon",		DisguiseWeaponb);

	GAMEPROP_DECL_RECV(float,					"CTFPlayer", "m_flMaxspeed",			MaxSpeed);
	GAMEPROP_DECL_RECV(float,					"CTFPlayer", "m_flCloakMeter",			CloakMeter);
	GAMEPROP_DECL_RECV(float,					"CTFPlayer", "m_flChargeMeter",			ChargeMeter);
	GAMEPROP_DECL_RECV(float,					"CTFPlayer", "m_flRageMeter",			RageMeter);
	GAMEPROP_DECL_RECV(float,					"CTFPlayer", "m_flHypeMeter",			HypeMeter);
	GAMEPROP_DECL_RECV(float,					"CTFPlayer", "m_flEnergyDrinkMeter",	EnergyDrinkMeter);
	GAMEPROP_DECL_RECV(bool,					"CTFPlayer", "m_bRageDraining",			RageDraining);
	GAMEPROP_DECL_RECV(bool,					"CTFPlayer", "m_bFeignDeathReady",		FeignDeathReady);
	GAMEPROP_DECL_RECV(float,					"CTFPlayer", "m_bShieldEquipped",		IsShieldEquipped);


	GAMEPROP_DECL_RECV(QAngle,					"CTFPlayer", "m_angEyeAngles[0]",		EyeAngles);
	GAMEPROP_DECL_RECV(QAngle,					"CTFPlayer", "m_vecPunchAngle",			PunchAngle);
	GAMEPROP_DECL_RECV(Vector,					"CTFPlayer", "m_vecViewOffset[0]",		ViewOffset);
	GAMEPROP_DECL_RECV(Vector,					"CTFPlayer", "m_vecVelocity[0]",		Velocity);

	GAMEPROP_DECL_RECV(IBaseHandle[MAX_WEAPONS],"CTFPlayer", "m_hMyWeapons",			MyWeapons);
	GAMEPROP_DECL_RECV(IBaseHandle,				"CTFPlayer", "m_hActiveWeapon",			ActiveWeapon);
	GAMEPROP_DECL_RECV(IBaseHandle,				"CTFPlayer", "m_hObserverTarget",		ObserverTarget);
	GAMEPROP_DECL_RECV(PlayerObserverMode,		"CTFPlayer", "m_iObserverMode",			ObserverMode);

	GAMEPROP_DECL_RECV(PlayerLifeState,			"CTFPlayer", "m_lifeState",				LifeState);
	GAMEPROP_DECL_RECV(TFClass,					"CTFPlayer", "m_iClass",				Class);
	GAMEPROP_DECL_RECV(TFPlayerStreak,			"CTFPlayer", "m_nStreaks",				Streaks);
	GAMEPROP_DECL_RECV(int,						"CTFPlayer", "m_iHealth",				CurrentHealth);
	GAMEPROP_DECL_RECV(float,					"CTFPlayer", "m_flDeathTime",			DeathTime);

	GAMEPROP_DECL_RECV(bool[MAX_PLAYERS],		"CTFPlayer", "m_bPlayerDominatingMe",	PlayerDominatingMe);
	GAMEPROP_DECL_RECV(bool[MAX_PLAYERS],		"CTFPlayer", "m_bPlayerDominated",		PlayerDominated);
};


template<bool IsLocalPlayer>
class IPlayerWrapper : public IBaseEntityWrapper<ITFPlayerInternal>
{
public:
	using IBaseEntityWrapper<ITFPlayerInternal>::IBaseEntityWrapper;

	IPlayerWrapper() noexcept
	{
		if constexpr (IsLocalPlayer)
			set(ITFPlayerInternal::GetLocalPlayer());
	}
};

using ITFPlayer = IPlayerWrapper<false>;
using ILocalPlayer = IPlayerWrapper<true>;
