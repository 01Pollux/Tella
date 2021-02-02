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

void InitIClientModeVFn();