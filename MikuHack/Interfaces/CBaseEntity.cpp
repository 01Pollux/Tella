#include "../Helpers/Commons.h"

#include "CBaseEntity.h"
#include "IBaseClientDLL.h"

#include "../Helpers/DrawTools.h"

#include "IVEClientTrace.h"
#include "../Interfaces/IClientMode.h"
#include "../Source/Debug.h"

const char* m_szTeams[] = 
{
	"Red",
	"Blue",
};


CTFPlayerResource ctfresource;
class EmptyClass { };

CUtlVector<IClientEntityListener*>* FindEntityListener()
{
	uintptr_t pEntListener = Library::clientlib.FindPattern("pCEntityListPtr") + Offsets::ClientDLL::To_EntListenerVec;
	return reinterpret_cast<CUtlVector<IClientEntityListener*>*>(pEntListener);
}

int IClientShared::GetTeam()
{
	return *this->GetEntProp<int>("m_iTeamNum");
}

void IClientShared::UpdateGlowEffect()
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		void (EmptyClass::* fn)();
		void* ptr;
	} u{ .ptr = reinterpret_cast<void*>(vtable[Offsets::IBaseEntity::VTIdx_UpdateGlowEffect]) };
	
	(reinterpret_cast<EmptyClass*>(this)->*u.fn)();
}

void IClientShared::DestroyGlowEffect()
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		void (EmptyClass::* fn)();
		void* ptr;
	} u{ .ptr = reinterpret_cast<void*>(vtable[Offsets::IBaseEntity::VTIdx_DestroyGlowEffect]) };

	(reinterpret_cast<EmptyClass*>(this)->*u.fn)();
}

void IClientShared::SetModel(int index)
{
	uintptr_t ptr = Library::clientlib.FindPattern("CBaseEntity::SetModelIndex");

	union {
		void (EmptyClass::* fn)(int);
		void* ptr;
	} u{ .ptr = reinterpret_cast<void*>(ptr) };

	(reinterpret_cast<EmptyClass*>(this)->*u.fn)(index);
}

int IBaseObject::GetWeaponSlot()
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		int (EmptyClass::* fn)();
		void* ptr;
	} u{ .ptr = reinterpret_cast<void*>(vtable[Offsets::IBaseEntity::VTIdx_GetSlot]) };

	return (reinterpret_cast<EmptyClass*>(this)->*u.fn)();
}

bool IBaseObject::Melee_DoSwingTrace(trace_t& trace)
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		bool (EmptyClass::* fn)(trace_t&);
		void* ptr;
	} u{ .ptr = vtable[Offsets::IBaseEntity::VTIdx_DoSwingTrace] };

	return (reinterpret_cast<EmptyClass*>(this)->*u.fn)(trace);
}

float IBaseObject::Melee_GetSwingRange()
{
	uintptr_t ptr = Library::clientlib.FindPattern("Melee_GetSwingRangeFn");

	union {
		float (IBaseObject::* fn)();
		void* ptr;
	} u{ .ptr = reinterpret_cast<void*>(ptr) };

	return (this->*u.fn)();
}

void* ITFPlayer::GetShared()
{
	return *this->GetEntProp<void*>("m_Shared");
}

ITFPlayer* ITFPlayer::FromShared(const void* pEnt)
{
	return *reinterpret_cast<ITFPlayer**>(reinterpret_cast<uintptr_t>(pEnt) + Offsets::IBaseEntity::TFPlayerShared::m_pOuter);
}

void* ITFPlayer::m_PlayerClass()
{
	return *this->GetEntProp<void*>("m_PlayerClass");
}

Vector ITFPlayer::EyePosition()
{
	static IBoneCache pCache;
	if (this->GetHitbox(0, &pCache))
		return pCache.center;

	return vec3_origin;
}

int ITFPlayer::GetHealth()
{
	return *this->GetEntProp<int>("m_iHealth");
}

Vector ITFPlayer::GetPosition()
{
	return *this->GetEntProp<Vector>("m_vecOrigin");
}

LIFE_STATE ITFPlayer::GetLifeState()
{
	return static_cast<LIFE_STATE>(*this->GetEntProp<uint8_t>("m_lifeState"));
}

TFClass ITFPlayer::GetClass()
{
	return *this->GetEntProp<TFClass>("m_iClass");
}

int ITFPlayer::GetKillCount()
{
	return *this->GetEntProp<int>("m_iKills");
}


IBaseHandle* ITFPlayer::GetWeaponList()
{
	return this->GetEntProp<IBaseHandle>("m_hMyWeapons");
}

IBaseHandle& ITFPlayer::GetActiveWeapon()
{
	return *this->GetEntProp<IBaseHandle>("m_hActiveWeapon");
}

struct CondVars
{
	const int cond_offsets[6] = {
	FindRecvOffset("CTFPlayer", "_condition_bits"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCond"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx2"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx3"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx4") };
};

bool ITFPlayer::InCond(ETFCond cond)
{
	switch (static_cast<int>(cond / 32))
	{
	case 0:
	{
		int bit = 1 << cond;
		if (*this->GetEntProp<uint32_t>("_condition_bits") & bit || *this->GetEntProp<uint32_t>("m_nPlayerCond") & bit)
			return true;
		break;
	}

	case 1:
	{
		int bit = 1 << (cond - 32);
		if (*this->GetEntProp<uint32_t>("m_nPlayerCondEx") & bit)
			return true;
		break;
	}

	case 2:
	{
		int bit = 1 << (cond - 64);
		if (*this->GetEntProp<uint32_t>("m_nPlayerCondEx2") & bit)
			return true;
		break;
	}

	case 3:
	{
		int bit = 1 << (cond - 96);
		if (*this->GetEntProp<uint32_t>("m_nPlayerCondEx3") & bit)
			return true;
		break;
	}

	case 4:
	{
		int bit = 1 << (cond - 128);
		if (*this->GetEntProp<uint32_t>("m_nPlayerCondEx4") & bit)
			return true;
		break;
	}
	}

	return false;
}


static void ChangeCond(ITFPlayer* pEnt, ETFCond cond, bool add)
{
	static int cond_offsets[6] = {
	FindRecvOffset("CTFPlayer", "_condition_bits"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCond"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx2"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx3"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx4") };


	switch (static_cast<int>(cond / 32))
	{
	case 0:
	{
		int bit = 1 << cond;
		uint32& cond = *reinterpret_cast<uint32*>(reinterpret_cast<uintptr_t>(pEnt) + cond_offsets[0]);
		uint32& cond2 = *reinterpret_cast<uint32*>(reinterpret_cast<uintptr_t>(pEnt) + cond_offsets[1]);

		if (add) {
			cond |= bit;
			cond2 |= bit;
		}
		else {
			cond &= ~bit;
			cond2 &= ~bit;
		}

		break;
	}

	case 1:
	{
		int bit = 1 << (cond - 32);
		uint32& cond = *reinterpret_cast<uint32*>(reinterpret_cast<uintptr_t>(pEnt) + cond_offsets[2]);

		if (add) cond |= bit;
		else cond &= ~bit;

		break;
	}

	case 2:
	{
		int bit = 1 << (cond - 64);
		uint32& cond = *reinterpret_cast<uint32*>(reinterpret_cast<uintptr_t>(pEnt) + cond_offsets[3]);

		if (add) cond |= bit;
		else cond &= ~bit;

		break;
	}

	case 3:
	{
		int bit = 1 << (cond - 96);
		uint32& cond = *reinterpret_cast<uint32*>(reinterpret_cast<uintptr_t>(pEnt) + cond_offsets[4]);

		if (add) cond |= bit;
		else cond &= ~bit;

		break;
	}

	case 4:
	{
		int bit = 1 << (cond - 128);
		uint32& cond = *reinterpret_cast<uint32*>(reinterpret_cast<uintptr_t>(pEnt) + cond_offsets[5]);

		if (add) cond |= bit;
		else cond &= ~bit;

		break;
	}
	}
}

void ITFPlayer::AddCond(ETFCond cond, float duration)
{
	ChangeCond(this, cond, true);
}

void ITFPlayer::RemoveCond(ETFCond cond)
{
	ChangeCond(this, cond, false);
}

int& ITFPlayer::GetStreaks(ETFStreak type)
{
	return *this->GetEntProp<int>("m_nStreaks");
}


bool IBaseObject::IsBaseCombatWeapon()
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		bool (EmptyClass::*fn)();
		void* ptr;
	} u{ .ptr = vtable[Offsets::IBaseEntity::VTIdx_IsBaseCombatWeapon] };
	
	return (reinterpret_cast<EmptyClass*>(this)->*u.fn)();
}

IBaseHandle& IBaseObject::GetOwnerEntity()
{
	return *this->GetEntProp<IBaseHandle>("m_hOwnerEntity");
}

int& IBaseObject::GetItemDefinitionIndex()
{
	return *this->GetEntProp<int>("m_iItemDefinitionIndex");
}

IAttributeList* IBaseObject::GetAttributeList()
{
	return this->GetEntProp<IAttributeList>("m_AttributeList");
}

int IBaseObject::GetUpgradeLvl()
{
	return *this->GetEntProp<int>("m_iUpgradeLevel");
}

int IBaseObject::GetBuildingHealth()
{
	return *this->GetEntProp<int>("m_iHealth");
}

int IBaseObject::GetBuildingMaxHealth()
{
	return *this->GetEntProp<int>("m_iMaxHealth");
}


IClientEntity* CTFPlayerResource::Update()
{
	IClientEntity* pEnt;

	for (int i = engineclient->GetMaxClients(); i <= clientlist->GetHighestEntityIndex(); i++)
	{
		pEnt = clientlist->GetClientEntity(i);
		if (pEnt && pEnt->GetClientClass()->m_ClassID == ClassID::ClassID_CTFPlayerResource)
		{
			this->Hndl = pEnt->GetRefEHandle();
			return pEnt;
		}
	}
}

#include "../Helpers/VTable.h"


IEntityCached ent_infos;

IEntityCached::IEntityCached()
{
	using namespace IGlobalEvent;

	LoadDLL::Hook::Register(std::bind(&IEntityCached::OnDLLAttach, this));
	UnloadDLL::Hook::Register(std::bind(&IEntityCached::OnDLLDetach, this));
	LevelInit::Hook::Register([this]() -> HookRes { m_EntInfos.clear(); return HookRes::Continue; });
	LevelShutdown::Hook::Register([this]() -> HookRes { m_EntInfos.clear(); return HookRes::Continue; });
}

HookRes IEntityCached::OnDLLAttach()
{
	AddEntityListener();
	m_EntInfos.clear();

	if (BAD_LOCAL())
		return HookRes::Continue;

	MyClientCacheList cache;
	EntFlag flag;

	int maxClients = gpGlobals->maxClients;
	int maxEntities = clientlist->GetHighestEntityIndex();

	for (int i = 1; i <= maxEntities; i++)
	{
		IClientShared* pEnt = GetClientEntityW(i);
		if (!pEnt)
			continue;

		if (i <= maxClients)
			flag = EntFlag::EF_PLAYER;
		else
		{
			ClientClass* cls = pEnt->GetClientClass();
			if (!cls)
				flag = EntFlag::EF_INVALID;
			else {
				switch (cls->m_ClassID)
				{
				case ClassID::ClassID_CObjectSentrygun:
				case ClassID::ClassID_CObjectTeleporter:
				case ClassID::ClassID_CObjectDispenser:
					flag = EntFlag::EF_BUILDING;
					break;

				default:
					flag = EntFlag::EF_EXTRA;
					break;
				}
			}
		}

		cache.pEnt = pEnt;
		cache.flag = flag;

		m_EntInfos.push_front(cache);
	}

	return HookRes::Continue;
}

void IEntityCached::OnEntityCreated(IClientShared* pEnt)
{
	if (!pEnt || !pEnt->GetClientNetworkable())
		return;

	int ent_index = pEnt->entindex();
	if (ent_index < 0)
		return;

	MyClientCacheList cache;
	EntFlag flag;

	if (ent_index <= gpGlobals->maxClients)
		flag = EntFlag::EF_PLAYER;
	else
	{
		ClientClass* cls = pEnt->GetClientClass();
		if (!cls)
			flag = EntFlag::EF_INVALID;
		else {
			switch (cls->m_ClassID)
			{
			case ClassID::ClassID_CObjectSentrygun:
			case ClassID::ClassID_CObjectTeleporter:
			case ClassID::ClassID_CObjectDispenser:
				flag = EntFlag::EF_BUILDING;
				break;

			default:
				flag = EntFlag::EF_EXTRA;
				break;
			}
		}
	}

	cache.pEnt = pEnt;
	cache.flag = flag;

	m_EntInfos.push_front(cache);
//	NBoneCache::AddEntity(pEnt);
}

void IEntityCached::OnEntityDeleted(IClientShared* pEnt)
{
	if (!pEnt || !pEnt->GetClientNetworkable())
		return;
	if (m_EntInfos.remove_if([&pEnt](const MyClientCacheList& cache) -> bool { return cache.pEnt == pEnt; }))
		pEnt->DestroyVirtualGlowObject();
}

HookRes IEntityCached::OnDLLDetach()
{
	RemoveEntityListener(); m_EntInfos.clear();
	return HookRes::Continue;
}



QAngle GetAimAngle(const Vector& vecTarget, bool useLocalPunchAng)
{
	QAngle res;
	Vector vecToTarget; vecToTarget = vecTarget - pLocalPlayer->LocalEyePosition();
	VectorAngles(vecToTarget, res);

	if(useLocalPunchAng)
		res -= *pLocalPlayer->GetEntProp<QAngle>("m_vecPunchAngle");

	ClampAngle(res);
	return res;
}

float GetFOV(ITFPlayer* pViewer, const Vector& vecEnd)
{
	QAngle angView; angView = pViewer->GetAbsAngles();
	Vector vecStartFwd; AngleVectors(angView, &vecStartFwd);
	vecStartFwd.NormalizeInPlace();

	Vector vecStart = pViewer->EyePosition();
	Vector vecDelta = vecEnd - vecStart;
	vecDelta.NormalizeInPlace();

	vec_t dot = vecStartFwd.Dot(vecDelta);
	vec_t pow_len = std::pow(FloatMakePositive(vecStartFwd.Length()), 2);

	return RAD2DEG(acosf(dot / (pow_len)));
}

float GetLocalFOV(const Vector& vecEnd)
{
	Vector vecFwd; AngleVectors(Globals::m_pUserCmd->viewangles, &vecFwd);
	vecFwd.NormalizeInPlace();

	Vector vecDelta = vecEnd - pLocalPlayer->LocalEyePosition();
	vecDelta.z = 0;
	vecDelta.NormalizeInPlace();

	vec_t dot = vecFwd.Dot(vecDelta);
	vec_t pow_len = std::pow(FloatMakePositive(vecFwd.Length()), 2);

	return RAD2DEG(acosf(dot / (pow_len)));
}

string_t AllocPooledString(const char* name)
{
	uintptr_t ptr = Library::clientlib.FindPattern("AllocPooledString");

	union {
		string_t(*fn)(const char*);
		void* ptr;
	} u{ .ptr = reinterpret_cast<void*>(ptr) };
	assert(u.fn);

	return (*u.fn)(name);
}