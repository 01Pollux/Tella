#pragma once

#include "../Source/Debug.h"

#include "../Helpers/NetVars.h"
#include "../Helpers/sdk.h"

#include "IClientEntityList.h"
#include "Color.h"

#include "../Helpers/BoneCache.h"
#include <string_t.h>


#define MAX_PLAYERS 32


using IBaseHandle = CBaseHandle;


enum class LIFE_STATE
{
	ALIVE,
	DYING,
	DEAD
};

enum TFClass
{
	UNKNOWN,

	Scout,
	Sniper,
	Soldier,
	Demoman,
	Medic,
	Heavy,
	Pyro,
	Spy,
	Engineer,

	MAX_CLASS
};

static const char* m_szTFClasses[] =
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

enum EntFlag
{
	EF_INVALID,
	EF_PLAYER,
	EF_BUILDING,
	EF_EXTRA
};

enum TFTeam
{
	UNASSIGNED,
	SPECTATOR,
	RED,
	BLUE,

	MAX_TEAMS
};

extern const char* m_szTeams[];

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

enum ETFStreak
{
	kTFStreak_Kills = 0,
	kTFStreak_KillsAll = 1,	// Counts all kills not just attr based killstreak.  For Data collection purposes
	kTFStreak_Ducks = 2,
	kTFStreak_Duck_levelup = 3,
	kTFStreak_COUNT = 4,
};

class IBaseObject;
class IAttributeList;
class ITFPlayer;

inline int FindRecvOffset(const char* cls, const char* name)
{
	recvprop_info_t infos;

	if (!LookupRecvProp(cls, name, &infos))
	{
		Warning("Failed to Find offset for \"%s::%s\"\n", cls, name);
		return 0;
	}

	return infos.offset;
}

class IGlowObject;
class IClientShared: public IClientEntity
{
public:
	IGlowObject* AllocateVirtualGlowObject(const Color&, bool = true);

	IGlowObject* QueryVirtualGlowObject();

	void		 DestroyVirtualGlowObject();

	int		GetTeam();

	void	UpdateGlowEffect();

	void	DestroyGlowEffect();

	bool&	HasGlow()
	{
		return *this->GetEntProp<bool>("m_bGlowEnabled");
	}

	Color GetTeamColor()
	{
		switch (static_cast<TFTeam>(this->GetTeam()))
		{
		case TFTeam::BLUE:	return Color(0, 0, 255);
		case TFTeam::RED:	return Color(255, 0, 0);
		default:			return Color(255, 255, 255, 155);
		}
	}

	bool IsAmmoPack()
	{
		return this->IsClassID(ClassID_CTFAmmoPack);
	}

	bool IsHealthPack()
	{
		const char* model_name = modelinfo->GetModelName(this->GetModel());
		return std::strstr(model_name, "medkit") != nullptr;
	}

	bool IsClassID(ClassID cls)
	{
		if (!this->GetClientClass())
			return -1;

		return (this->GetClientClass()->m_ClassID == cls);
	}

	template<typename _ReturnType>
	_ReturnType* GetEntProp(const char* prop, int extra_offset = 0)
	{
		recvprop_info_t info;
		LookupRecvPropC(this->GetClientClass(), prop, &info);

		return reinterpret_cast<_ReturnType*>(reinterpret_cast<uintptr_t>(this) + info.offset + extra_offset);
	}

	bool HasEntProp(const char* prop)
	{
		static recvprop_info_t info;
		return LookupRecvPropC(this->GetClientClass(), prop, &info);
	}

	bool GetHitbox(int id, IBoneCache* pCache)
	{
		return IBoneCache::GetHitbox(this, id, pCache);
	}

	int HitboxSet()
	{
		return *this->GetEntProp<int>("m_nHitboxSet");
	}

	void SetModel(int);
};


class CTFPlayerResource
{
	IBaseHandle Hndl;

public:
	IClientEntity* Update();
	void Invalidate() { }

	template<typename _ReturnType>
	_ReturnType& GetEntProp(IClientShared* pEnt, const char* prop)
	{
		IClientEntity* pRes = nullptr;

		if (!this->Hndl.IsValid())	pRes = Update();
		else						pRes = clientlist->GetClientEntityFromHandle(this->Hndl);

		int offset = FindRecvOffset("CTFPlayerResource", prop);

		return *reinterpret_cast<_ReturnType*>(reinterpret_cast<uintptr_t>(pRes) + offset + pEnt->entindex() * 4);
	}
};
extern CTFPlayerResource ctfresource;


class ITFPlayer: public IClientShared
{
public:
	void*					GetShared();
	static ITFPlayer*		FromShared(const void* shared);

	void*					m_PlayerClass();
	Vector					EyePosition();

	int						GetHealth();
	Vector					GetPosition();
	LIFE_STATE				GetLifeState();
	IBaseHandle*			GetWeaponList();
	IBaseHandle&			GetActiveWeapon();

	TFClass					GetClass();
	int						GetKillCount();

	bool					InCond(ETFCond cond);
	void					AddCond(ETFCond cond, float duration = -1.0);
	void					RemoveCond(ETFCond cond);
	int&					GetStreaks(ETFStreak type);

	Vector					LocalEyePosition()
	{
		return this->GetAbsOrigin() + *this->GetEntProp<Vector>("m_vecViewOffset[0]");
	}
};


class IBaseObject: public IClientShared
{
public:
	bool					IsBaseCombatWeapon();
	IBaseHandle&			GetOwnerEntity();
	int						GetWeaponSlot();
	int&					GetItemDefinitionIndex();
	IAttributeList*			GetAttributeList();

	bool					Melee_DoSwingTrace(trace_t& trace);
	float					Melee_GetSwingRange();

	int						GetUpgradeLvl();

	int						GetBuildingHealth();
	int						GetBuildingMaxHealth();
};



enum class HookRes: char8_t;

struct MyClientCacheList
{
	IClientShared*	pEnt;
	EntFlag			flag;
};


class IEntityCached: public IClientEntityListener
{
	bool m_bLoaded{ };

public:
	~IEntityCached() { OnDLLDetach(); }
	IEntityCached();

	const std::forward_list<MyClientCacheList>& GetInfos() const { return m_EntInfos; }

public:
	HookRes OnDLLAttach();
	HookRes OnDLLDetach();

public:	//IClientEntityListener
	void OnEntityCreated(IClientShared* pEnt) override;
	void OnEntityDeleted(IClientShared* pEnt) override;

private:
	std::forward_list<MyClientCacheList> m_EntInfos;
};

extern IEntityCached ent_infos;



enum hitbox_t
{
	invalid = -1,
	head,
	pelvis,
	spine_0,
	spine_1,
	spine_2,
	spine_3,
	upperArm_L,
	lowerArm_L,
	hand_L,
	upperArm_R,
	lowerArm_R,
	hand_R,
	hip_L,
	knee_L,
	foot_L,
	hip_R,
	knee_R,
	foot_R
};

inline void ClampAngle(QAngle& ang)
{
	ang.x = AngleNormalize(ang.x);
	ang.y = AngleNormalize(ang.y);
	ang.z = 0;
}

QAngle GetAimAngle(const Vector& vecTarget, bool useLocalPunchAng = false);
float GetFOV(ITFPlayer* pViewer, const Vector& vecEnd);
float GetLocalFOV(const Vector& vecEnd);

string_t AllocPooledString(const char*);

using IBaseCombatCharacter	= IBaseObject;
using IBaseCombatWeapon		= IBaseObject;
using IBaseAnimating		= IClientShared;
