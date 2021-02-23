#pragma once

#include <icliententitylist.h>

#include <basehandle.h>
#include <icliententity.h>
#include <engine/ICollideable.h>
#include <engine/ivmodelinfo.h>
#include <globalvars_base.h>


extern IVModelInfo* modelinfo;
extern CGlobalVarsBase* gpGlobals;
extern IClientEntityList* clientlist;


inline int TimeToTicks(float time)
{
	return static_cast<int>(0.5 + (time / gpGlobals->interval_per_tick));
}

inline float TicksToTime(int tick)
{
	return gpGlobals->interval_per_tick * static_cast<float>(tick);
}
