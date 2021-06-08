#include "BaseObject.hpp"
#include "Library/Lib.hpp"
#include "Helper/Offsets.hpp"

GAMEPROP_IMPL_RECV(IBaseObjectInternal, Health);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, MaxHealth);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, HasSapper);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, ObjectType);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, ObjectMode);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, IsBuilding);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, IsPlacing);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, IsCarried);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, IsMiniBuilding);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, Builder);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, UpgradeLevel);
GAMEPROP_IMPL_RECV(IBaseObjectInternal, UpgradeMetal);


GAMEPROP_IMPL_RECV(ISentryGunInternal, AmmoShells);
GAMEPROP_IMPL_RECV(ISentryGunInternal, MaxAmmoShells);
GAMEPROP_IMPL_RECV(ISentryGunInternal, AmmoRockets);
GAMEPROP_IMPL_RECV(ISentryGunInternal, MaxAmmoRockets);
GAMEPROP_IMPL_RECV(ISentryGunInternal, IsShielded);
GAMEPROP_IMPL_RECV(ISentryGunInternal, EnemyTarget);
GAMEPROP_IMPL_RECV(ISentryGunInternal, AutoTarget);
GAMEPROP_IMPL_RECV(ISentryGunInternal, State);
GAMEPROP_IMPL_RECV(ISentryGunInternal, IsPlayerControlled);


GAMEPROP_IMPL_RECV(IDispenserInternal, AmmoMetal);
GAMEPROP_IMPL_RECV(IDispenserInternal, MiniBombCounter);
GAMEPROP_IMPL_RECV(IDispenserInternal, State);
GAMEPROP_IMPL_RECV(IDispenserInternal, HealingTargets);


GAMEPROP_IMPL_RECV(ITeleporterInternal, IsMatchBuilding);
GAMEPROP_IMPL_RECV(ITeleporterInternal, TimesUsed);
GAMEPROP_IMPL_RECV(ITeleporterInternal, RechargeTime);
GAMEPROP_IMPL_RECV(ITeleporterInternal, CurrentRechargeDuration);
GAMEPROP_IMPL_RECV(ITeleporterInternal, State);