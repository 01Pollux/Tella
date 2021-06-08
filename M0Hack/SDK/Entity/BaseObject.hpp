#pragma once 

#include "BaseEntity.hpp"


class IBaseObjectInternal : public IBaseEntityInternal
{
public:
	static IBaseObjectInternal* GetEntity(int index)
	{
		return static_cast<IBaseObjectInternal*>(IBaseEntityInternal::GetEntity(index));
	}
	static IBaseObjectInternal* GetEntity(IBaseHandle hndl)
	{
		return static_cast<IBaseObjectInternal*>(IBaseEntityInternal::GetEntity(hndl));
	}

	GAMEPROP_DECL_RECV(int,					"CBaseObject", 			"m_iHealth",				Health);
	GAMEPROP_DECL_RECV(int,					"CBaseObject", 			"m_iMaxHealth",				MaxHealth);
	GAMEPROP_DECL_RECV(int,					"CBaseObject", 			"m_bHasSapper",				HasSapper);
	GAMEPROP_DECL_RECV(EntBuildingObjType,	"CBaseObject", 			"m_iObjectType",			ObjectType);
	GAMEPROP_DECL_RECV(EntBuildingObjMode,	"CBaseObject", 			"m_iObjectMode",			ObjectMode);
	GAMEPROP_DECL_RECV(bool,				"CBaseObject", 			"m_bBuilding",				IsBuilding);
	GAMEPROP_DECL_RECV(bool,				"CBaseObject", 			"m_bPlacing",				IsPlacing);
	GAMEPROP_DECL_RECV(bool,				"CBaseObject", 			"m_bCarried",				IsCarried);
	GAMEPROP_DECL_RECV(bool,				"CBaseObject", 			"m_bMiniBuilding",			IsMiniBuilding);
	GAMEPROP_DECL_RECV(IBaseHandle,			"CBaseObject", 			"m_hBuilder",				Builder);
	GAMEPROP_DECL_RECV(int,					"CBaseObject",			"m_iUpgradeLevel",			UpgradeLevel);
	GAMEPROP_DECL_RECV(int,					"CBaseObject",			"m_iUpgradeMetal",			UpgradeMetal);
};
using IBaseObject = IBaseEntityWrapper<IBaseObjectInternal>;


class ISentryGunInternal : public IBaseObjectInternal
{
public:
	GAMEPROP_DECL_RECV(int,					"CObjectSentrygun",		"m_iAmmoShells",			AmmoShells);
	GAMEPROP_DECL_RECV(int,					"CObjectSentrygun",		"m_iAmmoShells",			MaxAmmoShells, 4);
	GAMEPROP_DECL_RECV(int,					"CObjectSentrygun",		"m_iAmmoRockets",			AmmoRockets);
	GAMEPROP_DECL_RECV(int,					"CObjectSentrygun",		"m_iAmmoRockets",			MaxAmmoRockets, 4);
	GAMEPROP_DECL_RECV(bool,				"CObjectSentrygun",		"m_bShielded",				IsShielded);
	GAMEPROP_DECL_RECV(IBaseHandle,			"CObjectSentrygun",		"m_hEnemy",					EnemyTarget);
	GAMEPROP_DECL_RECV(IBaseHandle,			"CObjectSentrygun",		"m_hAutoAimTarget",			AutoTarget);
	GAMEPROP_DECL_RECV(SentryGunState,		"CObjectSentrygun",		"m_iState",					State);
	GAMEPROP_DECL_RECV(bool,				"CObjectSentrygun",		"m_bPlayerControlled",		IsPlayerControlled);
};
using ISentryGun = IBaseEntityWrapper<ISentryGunInternal>;


class IDispenserInternal : public IBaseObjectInternal
{
public:
	GAMEPROP_DECL_RECV(int,							"CObjectDispenser",		"m_iAmmoMetal",				AmmoMetal);
	GAMEPROP_DECL_RECV(int,							"CObjectDispenser",		"m_iMiniBombCounter",		MiniBombCounter);
	GAMEPROP_DECL_RECV(DispenserState,				"CObjectDispenser",		"m_iState",					State);
	GAMEPROP_DECL_RECV(ValveUtlVector<IBaseHandle>,	"CObjectDispenser",		"m_iMiniBombCounter",		HealingTargets,		Offsets::IBaseEntity::Dispenser::m_iMiniBombCounter__To__m_hHealingTargets);
};
using IDispenser = IBaseEntityWrapper<IDispenserInternal>;


class ITeleporterInternal : public IBaseObjectInternal
{
public:
	GAMEPROP_DECL_RECV(bool,				"CObjectTeleporter",		"m_bMatchBuilding",				IsMatchBuilding);
	GAMEPROP_DECL_RECV(int,					"CObjectTeleporter",		"m_iTimesUsed",					TimesUsed);
	GAMEPROP_DECL_RECV(float,				"CObjectTeleporter",		"m_flRechargeTime",				RechargeTime);
	GAMEPROP_DECL_RECV(float,				"CObjectTeleporter",		"m_flCurrentRechargeDuration",	CurrentRechargeDuration);
	GAMEPROP_DECL_RECV(TeleporterState,		"CObjectTeleporter",		"m_iState",						State);
};
using ITeleporter = IBaseEntityWrapper<ITeleporterInternal>;