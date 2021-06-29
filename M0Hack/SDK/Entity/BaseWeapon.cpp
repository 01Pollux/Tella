#include "BaseWeapon.hpp"
#include "Library/Lib.hpp"
#include "Helper/Offsets.hpp"

GAMEPROP_IMPL_RECV(IBaseWeaponInternal, ViewModelIndex);
GAMEPROP_IMPL_RECV(IBaseWeaponInternal, WorldModelIndex);

GAMEPROP_IMPL_RECV(IBaseWeaponInternal, ChargedDamage);
GAMEPROP_IMPL_RECV(IBaseWeaponInternal, LastFireTime);
GAMEPROP_IMPL_RECV(IBaseWeaponInternal, ObservedCritChance);

GAMEPROP_IMPL_RECV(IBaseWeaponInternal, Type);
GAMEPROP_IMPL_RECV(IBaseWeaponInternal, Launcher);

GAMEPROP_IMPL_RECV(IBaseWeaponInternal, ChargeLevel);
GAMEPROP_IMPL_RECV(IBaseWeaponInternal, HealingTarget);
GAMEPROP_IMPL_RECV(IBaseWeaponInternal, IsAttacking);
GAMEPROP_IMPL_RECV(IBaseWeaponInternal, IsReleasingCharge);
GAMEPROP_IMPL_RECV(IBaseWeaponInternal, ChargeResistType);

GAMEPROP_IMPL_RECV(IBaseWeaponInternal, ReadyToBackstab);

bool IBaseWeaponInternal::IsBaseCombatWeapon() const noexcept
{
	static IMemberVFuncThunk<bool> iscombatweapon(Offsets::IBaseEntity::VTIdx_IsBaseCombatWeapon);
	return iscombatweapon(this);
}

int IBaseWeaponInternal::GetWeaponSlot() const noexcept
{
	static IMemberVFuncThunk<int> getslot(Offsets::IBaseEntity::VTIdx_GetSlot);
	return getslot(this);
}

bool IBaseWeaponInternal::DoSwingTrace(GameTrace& trace) noexcept
{
	static IMemberVFuncThunk<bool, GameTrace&> swingtrace(Offsets::IBaseEntity::VTIdx_DoSwingTrace);
	return swingtrace(this, trace);
}

int IBaseWeaponInternal::GetSwingRange() const noexcept
{
	static IMemberFuncThunk<int> getswingrange(M0CLIENT_DLL, "Melee_GetSwingRangeFn");
	return getswingrange(this);
}
