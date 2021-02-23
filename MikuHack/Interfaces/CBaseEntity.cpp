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


int IClientShared::GetTeam()
{
	return *this->GetEntProp<int, PropType::Recv>("m_iTeamNum");
}

[[inline]] void IClientShared::UpdateGlowEffect() noexcept
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		void (IClientShared::* fn)();
		void* ptr;
	} u{ .ptr = vtable[Offsets::IBaseEntity::VTIdx_UpdateGlowEffect] };
	
	(this->*u.fn)();
}

[[inline]] void IClientShared::DestroyGlowEffect() noexcept
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		void (IClientShared::* fn)();
		void* ptr;
	} u{ .ptr = vtable[Offsets::IBaseEntity::VTIdx_DestroyGlowEffect] };

	(this->*u.fn)();
}

[[inline]] void IClientShared::SetModel(int index)
{
	union {
		void (IClientShared::* fn)(int);
		uintptr_t ptr;
	} u{ .ptr = Library::clientlib.FindPattern("CBaseEntity::SetModelIndex") };

	(this->*u.fn)(index);
}

[[inline]]int IBaseObject::GetWeaponSlot() noexcept
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		int (IClientShared::* fn)();
		void* ptr;
	} u{ .ptr = vtable[Offsets::IBaseEntity::VTIdx_GetSlot] };

	return (this->*u.fn)();
}

[[inline]] bool IBaseObject::Melee_DoSwingTrace(trace_t& trace) noexcept
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		bool (IClientShared::* fn)(trace_t&);
		void* ptr;
	} u{ .ptr = vtable[Offsets::IBaseEntity::VTIdx_DoSwingTrace] };

	return (this->*u.fn)(trace);
}

[[inline]] float IBaseObject::Melee_GetSwingRange()
{
	union {
		float (IBaseObject::* fn)();
		uintptr_t ptr;
	} u{ .ptr = Library::clientlib.FindPattern("Melee_GetSwingRangeFn") };

	return (this->*u.fn)();
}

void* ITFPlayer::GetShared()
{
	return *this->GetEntProp<void*, PropType::Recv>("m_Shared");
}

ITFPlayer* ITFPlayer::FromShared(const void* pEnt) noexcept
{
	return *reinterpret_cast<ITFPlayer**>(reinterpret_cast<uintptr_t>(pEnt) + Offsets::IBaseEntity::TFPlayerShared::m_pOuter);
}

void* ITFPlayer::m_PlayerClass()
{
	return *this->GetEntProp<void*, PropType::Recv>("m_PlayerClass");
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
	return *this->GetEntProp<int, PropType::Recv>("m_iHealth");
}

Vector ITFPlayer::GetPosition()
{
	return *this->GetEntProp<Vector, PropType::Recv>("m_vecOrigin");
}

LIFE_STATE ITFPlayer::GetLifeState()
{
	return *this->GetEntProp<LIFE_STATE, PropType::Recv>("m_lifeState");
}

TFClass ITFPlayer::GetClass()
{
	return *this->GetEntProp<TFClass, PropType::Recv>("m_iClass");
}

int ITFPlayer::GetKillCount()
{
	return *this->GetEntProp<int, PropType::Recv>("m_iKills");
}


IBaseHandle* ITFPlayer::GetWeaponList()
{
	return this->GetEntProp<IBaseHandle, PropType::Recv>("m_hMyWeapons");
}

IBaseHandle& ITFPlayer::GetActiveWeapon()
{
	return *this->GetEntProp<IBaseHandle, PropType::Recv>("m_hActiveWeapon");
}

/*
struct CondVars
{
	enum class CondType
	{
		CondBits,
		Cond0,
		CondEx, CondEx2,
		CondEx3, CondEx4,
	}
	static const uint32_t cond_offsets[6] = {
	FindRecvOffset("CTFPlayer", "_condition_bits"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCond"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx2"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx3"),
	FindRecvOffset("CTFPlayer", "m_nPlayerCondEx4") };
};
*/

bool ITFPlayer::InCond(ETFCond cond)
{
	switch (static_cast<int>(cond / 32))
	{
	case 0:
	{
		int bit = 1 << cond;
		if (*this->GetEntProp<uint32_t, PropType::Recv>("_condition_bits") & bit || *this->GetEntProp<uint32_t, PropType::Recv>("m_nPlayerCond") & bit)
			return true;
		break;
	}

	case 1:
	{
		int bit = 1 << (cond - 32);
		if (*this->GetEntProp<uint32_t, PropType::Recv>("m_nPlayerCondEx") & bit)
			return true;
		break;
	}

	case 2:
	{
		int bit = 1 << (cond - 64);
		if (*this->GetEntProp<uint32_t, PropType::Recv>("m_nPlayerCondEx2") & bit)
			return true;
		break;
	}

	case 3:
	{
		int bit = 1 << (cond - 96);
		if (*this->GetEntProp<uint32_t, PropType::Recv>("m_nPlayerCondEx3") & bit)
			return true;
		break;
	}

	case 4:
	{
		int bit = 1 << (cond - 128);
		if (*this->GetEntProp<uint32_t, PropType::Recv>("m_nPlayerCondEx4") & bit)
			return true;
	}
	default:
		break;
	}

	return false;
}


static [[noinline]] void ChangeCond(ITFPlayer* pEnt, ETFCond cond, bool add)
{
	static int cond_offsets[6]{ };
	if (!cond_offsets[0])
	{
		[pEnt](int* conds)
		{
			recvprop_info_t info;
			ClientClass* cls = pEnt->GetClientClass();
			constexpr const char* names[] = {
					"_condition_bits", "m_nPlayerCond",
					"m_nPlayerCondEx", "m_nPlayerCondEx2",
					"m_nPlayerCondEx3", "m_nPlayerCondEx4"
			};
			for (int i = 0; i < SizeOfArray(names); i++)
			{
				LookupRecvPropC(cls, names[i], &info);
				conds[i] = info.offset;
			}
		}(cond_offsets);
	}


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
	return *this->GetEntProp<int, PropType::Recv>("m_nStreaks");
}


[[inline]] bool IBaseObject::IsBaseCombatWeapon() noexcept
{
	void** vtable = *reinterpret_cast<void***>(this);

	union {
		bool (IClientShared::*fn)();
		void* ptr;
	} u{ .ptr = vtable[Offsets::IBaseEntity::VTIdx_IsBaseCombatWeapon] };
	
	return (this->*u.fn)();
}

IBaseHandle& IBaseObject::GetOwnerEntity()
{
	return *this->GetEntProp<IBaseHandle, PropType::Recv>("m_hOwnerEntity");
}

int& IBaseObject::GetItemDefinitionIndex()
{
	return *this->GetEntProp<int, PropType::Recv>("m_iItemDefinitionIndex");
}

IAttributeList* IBaseObject::GetAttributeList()
{
	return this->GetEntProp<IAttributeList, PropType::Recv>("m_AttributeList");
}

int IBaseObject::GetUpgradeLvl()
{
	return *this->GetEntProp<int, PropType::Recv>("m_iUpgradeLevel");
}

int IBaseObject::GetBuildingHealth()
{
	return *this->GetEntProp<int, PropType::Recv>("m_iHealth");
}

int IBaseObject::GetBuildingMaxHealth()
{
	return *this->GetEntProp<int, PropType::Recv>("m_iMaxHealth");
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




QAngle GetAimAngle(const Vector& vecTarget, bool useLocalPunchAng)
{
	QAngle res;
	Vector vecToTarget; vecToTarget = vecTarget - ILocalPtr()->LocalEyePosition();
	VectorAngles(vecToTarget, res);

	if (useLocalPunchAng)
		res -= *ILocalPtr()->GetEntProp<QAngle, PropType::Recv>("m_vecPunchAngle");

	ClampAngle(res);
	return res;
}

float GetFOV(ITFPlayer* pViewer, const Vector& vecEnd)
{
	const QAngle& angView = pViewer->GetAbsAngles();
	Vector vecStartFwd; AngleVectors(angView, &vecStartFwd);
	vecStartFwd.NormalizeInPlace();

	const Vector& vecStart = pViewer->EyePosition();
	Vector vecDelta = vecEnd - vecStart;
	vecDelta.NormalizeInPlace();

	vec_t dot = vecStartFwd.Dot(vecDelta);
	vec_t pow_len = std::pow(FloatMakePositive(vecStartFwd.Length()), 2);

	return RAD2DEG(acosf(dot / (pow_len)));
}

float GetLocalFOV(const QAngle& angles, const Vector& vecEnd)
{
	Vector vecFwd; AngleVectors(angles, &vecFwd);
	vecFwd.NormalizeInPlace();

	Vector vecDelta = vecEnd - ILocalPtr()->LocalEyePosition();
	vecDelta.z = 0;
	vecDelta.NormalizeInPlace();

	vec_t dot = vecFwd.Dot(vecDelta);
	vec_t pow_len = std::pow(FloatMakePositive(vecFwd.Length()), 2);

	return RAD2DEG(acosf(dot / (pow_len)));
}

float GetLerpTime()
{
	static float latency;
	static Timer timer_update;

	if (timer_update.trigger_if_elapsed(5s))
	{
		constexpr auto FastCVar = [](const char* name)				{ return g_pCVar->FindVar(name); };

		constexpr auto get_max = [](auto&& a, auto&& b)				{ return a > b ? a : b; };
		constexpr auto do_clamp = [](auto&& val, auto&& min, auto&& max) 
		{ 
			if (min > max)
				return max;
			else if (val > max)
				return max;
			else if (val < min)
				return min;
			else return val;
		};


		static ConVar* cl_interp = FastCVar("cl_interp");
		static ConVar* cl_interp_ratio = FastCVar("cl_interp_ratio");
		static ConVar* cl_updaterate = FastCVar("cl_updaterate");

		float interp_ratio = cl_interp_ratio->GetFloat();
		if (!interp_ratio) interp_ratio = 1.0f;

		static const ConVar* sv_client_min_interp_ratio = FastCVar("sv_client_min_interp_ratio");
		static const ConVar* sv_client_max_interp_ratio = FastCVar("sv_client_max_interp_ratio");

		if (float tmp = sv_client_min_interp_ratio->GetFloat(); tmp != -1)
		{
			interp_ratio = do_clamp(interp_ratio, tmp, sv_client_max_interp_ratio->GetFloat());
		}

		latency = get_max(cl_interp->GetFloat(), interp_ratio / cl_updaterate->GetFloat());
	}

	return latency;
}

string_t AllocPooledString(const char* name)
{
	uintptr_t ptr = Library::clientlib.FindPattern("AllocPooledString");

	union {
		string_t(*fn)(const char*);
		void* ptr;
	} u{.ptr = reinterpret_cast<void*>(ptr) };

	return (*u.fn)(name);
}