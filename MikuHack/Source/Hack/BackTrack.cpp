#include "BackTrack.h"

#if 0
#define SIMULATION_TIME_PROP "m_flSimulationTime"

static IBackTrackHack track_hack;

void QueryBackTrackTicks(int index, IBackTrackInfo::FilterCallback callback);


inline void ProcessCurrentData(ITFPlayer* pEnt, const IBackTrackData* data)
{
	if (!pEnt)
	{
		MIKUDebug::LogCritical("Entity is nullptr");
		return;
	}
	else if (!data)
	{
		MIKUDebug::LogCritical("Data is nullptr");
		return;
	}

	int was = Globals::m_pUserCmd->tick_count;
	Globals::m_pUserCmd->tick_count = data->tick;
	*pEnt->GetEntProp<float>(SIMULATION_TIME_PROP) = data->simtime;

	*pEnt->GetEntProp<Vector>("m_vecOrigin") = data->origin;
	*pEnt->GetEntProp<QAngle>("m_vecAngles") = data->angles;
}


void OnBacktrackAimbot(ITFPlayer* pPlayer, Vector& vecTarget, QAngle& angView)
{
	if (!track_hack.bEnable)
		return;

	ITFPlayer* pMe = pLocalPlayer;
	if (!pMe || !engineclient->IsInGame() || pMe->GetLifeState() != LIFE_STATE::ALIVE)
		return;

	if(!(Globals::m_pUserCmd->buttons & IN_ATTACK))
		return;

	Vector mypos = pMe->LocalEyePosition();
	Vector pos;
	float bestFOV = track_hack.flTrackFOV;
	const IBackTrackData* best_data = nullptr;

	int index = pPlayer->entindex();

	auto ProcessFilterPlayer = [&pPlayer, &best_data, &bestFOV](int client, const IBackTrackData& current) {
		Vector pos = pPlayer->GetAbsOrigin();
		float cur = GetLocalFOV(pos);

		if (cur < bestFOV)
		{
			bestFOV = cur;
			best_data = &current;
		}
	};

	QueryBackTrackTicks(index, ProcessFilterPlayer);
	if (!best_data)
		return;

	ProcessCurrentData(pPlayer, best_data);
}


void IBackTrackHack::OnCreateMove(bool& ret)
{
	if (!bEnable)
		return;

	int tick = Globals::m_pUserCmd->tick_count;
	size_t cur = tick % SIZE_OF_BACKTRACK;

	ITFPlayer* pPlayer;

	for (uint8_t i = 1; i < gpGlobals->maxClients; i++)
	{
		pPlayer = reinterpret_cast<ITFPlayer*>(GetClientEntityW(i));
		IBackTrackInfo& data = backtrack[i - 1];

		if (!pPlayer || pPlayer->GetLifeState() != LIFE_STATE::ALIVE)
		{
			data.reset();
			continue;
		}

		if (!data)
		{
			printf("Init: %i (tick, %i)\n", i, tick);
			data.init();
		}

		IBackTrackData& info = data[cur];

		info.tick = tick;
		info.simtime = *pPlayer->GetEntProp<float>(SIMULATION_TIME_PROP);
		info.origin = pPlayer->GetAbsOrigin();
		info.angles = pPlayer->GetAbsAngles();
	}
}


void QueryBackTrackTicks(int index, IBackTrackInfo::FilterCallback callback)
{
	IBackTrackInfo& data = track_hack.backtrack[index - 1];
	if (!data)
		return;

	size_t cur_tick = Globals::m_pUserCmd->tick_count;
	uint8_t pos;

	for (uint8_t i = 0; i < SIZE_OF_BACKTRACK - 1; i++)
	{
		pos = (cur_tick - i) % SIZE_OF_BACKTRACK;
		
		IBackTrackData& info = data[pos];
		if (info.valid())
			callback(index, info);
	}
}


void IBackTrackHack::OnLoad()
{
//	AddAimHook(OnAimbotEnable);
}

void IBackTrackHack::OnUnload()
{
//	RemoveAimHook(OnAimbotEnable);
}

HAT_COMMAND(backtrack_toggle, "Toggles backtrack")
{
	track_hack.bEnable = !track_hack.bEnable;
	REPLY_TO_TARGET("BackTrack is now \"%s\"", track_hack.bEnable ? "ON":"OFF");
}

#include "../Detour/detours.h"
union attribute_data_union_t
{
	float		asFloat;
	uint32		asUint32;
	uint8_t*	asBlobPointer;
};

struct static_attrib_t
{
	uint16_t	iDefIndex;
	attribute_data_union_t 
				m_value;
	bool		bForceGCToGenerate;
	KeyValues*	m_pKVCustomData;
};

struct lootlist_attrib_t
{
	static_attrib_t	m_staticAttrib;
	float	m_flWeight;
};

struct random_attrib_t
{
	float			m_flChanceOfRandomAttribute;
	float			m_flTotalAttributeWeight;
	bool			m_bPickAllAttributes;
	CUtlVector<lootlist_attrib_t>
					m_RandomAttributes;
};

struct LootListInfo
{
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t count;
};

class IEconItemInterface;
using PEconItemInterface = IEconItemInterface*;


#endif