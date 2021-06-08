#pragma once 

#include "EconEntity.hpp"

enum class EntGrenadeProjType
{
	Regular,
	Detonate,
	DetonateRemote,
	CannonBall
};

class GameTrace;

class IBaseWeaponInternal : public IEconEntityInternal
{
public:
	static IBaseWeaponInternal* GetEntity(int index)
	{
		return static_cast<IBaseWeaponInternal*>(IBaseEntityInternal::GetEntity(index));
	}
	static IBaseWeaponInternal* GetEntity(IBaseHandle hndl)
	{
		return static_cast<IBaseWeaponInternal*>(IBaseEntityInternal::GetEntity(hndl));
	}

	bool IsBaseCombatWeapon() const noexcept;

	int GetWeaponSlot() const noexcept;

	bool DoSwingTrace(GameTrace& trace) noexcept;

	int	GetSwingRange() const noexcept;

	GAMEPROP_DECL_RECV(int,					"CTFWeaponBase",					"m_iViewModelIndex",		ViewModelIndex);
	GAMEPROP_DECL_RECV(int,					"CTFWeaponBase",					"m_iWorldModelIndex",		WorldModelIndex);

	GAMEPROP_DECL_RECV(float,				"CTFWeaponBase",					"m_flLastFireTime",			LastFireTime);
	GAMEPROP_DECL_RECV(float,				"CTFWeaponBase",					"m_flObservedCritChance",	ObservedCritChance);

	GAMEPROP_DECL_RECV(float,				"CTFSniperRifle",					"m_flChargedDamage",		ChargedDamage);

	GAMEPROP_DECL_RECV(EntGrenadeProjType,	"CTFGrenadePipebombProjectile",		"m_iType",					Type);
	GAMEPROP_DECL_RECV(IHandleEntity,		"CTFGrenadePipebombProjectile",		"m_hLauncher",				Launcher);

	GAMEPROP_DECL_RECV(float,				"CWeaponMedigun",					"m_flChargeLevel",			ChargeLevel);
	GAMEPROP_DECL_RECV(float,				"CWeaponMedigun",					"m_hHealingTarget",			HealingTarget);
	GAMEPROP_DECL_RECV(bool,				"CWeaponMedigun",					"m_bAttacking",				IsAttacking);
	GAMEPROP_DECL_RECV(bool,				"CWeaponMedigun",					"m_bChargeRelease",			IsReleasingCharge);
	GAMEPROP_DECL_RECV(int,					"CWeaponMedigun",					"m_nChargeResistType",		ChargeResistType);

	GAMEPROP_DECL_RECV(bool,				"CTFKnife",							"m_bReadyToBackstab",		ReadyToBackstab);
};
using IBaseWeapon = IBaseEntityWrapper<IBaseWeaponInternal>;

