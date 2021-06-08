#include "ResourceEntity.hpp"
#include "GlobalVars.hpp"
#include "cdll_int.hpp"

#include "GlobalHook/listener.hpp"

#include <iostream>

GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, Ping);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, Score);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, Deaths);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, TotalScore);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, Team);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, IsAlive);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, IsValid);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, IsConnected);

GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, Damage);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, DamageBoss);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, DamageAssist);

GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, Health);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, MaxHealth);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, MaxBuffedHealth);

GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, NextSpawnTime);
GAMEPROP_IMPL_RECV(TFPlayerResourceEntity, ChargeLevel);


GAMEPROP_IMPL_RECV(TFMonsterResourceEntity, HealthPerc);
GAMEPROP_IMPL_RECV(TFMonsterResourceEntity, StunPerc);
GAMEPROP_IMPL_RECV(TFMonsterResourceEntity, State);


namespace TFResourceEntity
{
	TFPlayerResourceEntity* Player;
	TFMonsterResourceEntity* Monster;
}

class ResourceEntManager
{
public:
	ResourceEntManager()
	{
		M0EventManager::AddListener(
			EVENT_KEY_ENTITY_CREATED,
			[this](M0EventData* EventData)
			{
				IBaseEntity pEnt(reinterpret_cast<IBaseEntityInternal*>(EventData));

				switch (pEnt->GetClientClass()->ClassID)
				{
				case EntClassID::CTFPlayerResource:
				{
					TFResourceEntity::Player = reinterpret_cast<TFPlayerResourceEntity*>(pEnt.get());
					break;
				}
				case EntClassID::CMonsterResource:
				{
					TFResourceEntity::Monster = reinterpret_cast<TFMonsterResourceEntity*>(pEnt.get());
					break;
				}
				}
			},
			"ResourceEntManager::OnEntityCreated"
		);

		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL,
			[this](M0EventData*)
			{
				if (IBaseEntityInternal::BadLocal())
					return;

				for (int i = Interfaces::GlobalVars->MaxClients + 1; i <= IBaseEntityInternal::GetHighestEntityIndex(); i++)
				{
					IBaseEntity pEnt(i);
					if (!pEnt)
						continue;

					ClientClass* pCls = pEnt->GetClientClass();
					if (!pCls)
						continue;

					switch (pCls->ClassID)
					{
					case EntClassID::CTFPlayerResource:
					{
						TFResourceEntity::Player = reinterpret_cast<TFPlayerResourceEntity*>(pEnt.get());
						break;
					}
					case EntClassID::CMonsterResource:
					{
						TFResourceEntity::Monster = reinterpret_cast<TFMonsterResourceEntity*>(pEnt.get());
						break;
					}
					}
				}
			},
			EVENT_NULL_NAME
		);
	}
};

static ResourceEntManager dummy_entity_manager;