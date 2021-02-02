#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <memory>
#include "Library.h"

#include "../Interfaces/IVEngineClient.h"
#include "../Interfaces/IClientEntityList.h"



class CUserCmd;
class INetMessage;

#define iLocalPlayer			engineclient->GetLocalPlayer()

#define pLocalPlayer			reinterpret_cast<ITFPlayer*>(clientlist->GetClientEntity(iLocalPlayer))

#define pLocalWeapon			reinterpret_cast<IBaseObject*>(clientlist->GetClientEntityFromHandle(pLocalPlayer->GetActiveWeapon()))

#define GetClientEntityW(IDX)	reinterpret_cast<IClientShared*>(clientlist->GetClientEntity(IDX))

#define GetTFPlayerW(IDX)		reinterpret_cast<ITFPlayer*>(clientlist->GetClientEntity(IDX))

#define GetActiveWeaponW(ENT)	reinterpret_cast<IClientShared*>(clientlist->GetClientEntityFromHandle(ENT->GetActiveWeapon()))

#define BAD_PLAYER(PLAYER)		(!PLAYER || PLAYER ->IsDormant())
#define BAD_LOCAL()				(BAD_PLAYER(pLocalPlayer) || !engineclient->IsInGame())

namespace Globals
{
	extern CUserCmd*		m_pUserCmd;
	extern bool*			bSendPacket;
}


class IClientEntityListener;
class IClientShared;

CUtlVector<IClientEntityListener*>* FindEntityListener();

class IClientEntityListener
{
public:
	virtual void OnEntityCreated(IClientShared* pEnt) { };
	virtual void OnEntityDeleted(IClientShared* pEnt) { };

public:
	void AddEntityListener();
	void RemoveEntityListener();
};