#include "BoneCache.h"
#include "../Interfaces/CBaseEntity.h"
#include "../Detour/detours.h"



bool IBoneCache::GetHitbox(IClientShared* pEnt, int index, IBoneCache* pSetup)
{
	const model_t* mdl = pEnt->GetModel();
	if (!mdl)
		return false;

	studiohdr_t* pStudioHDR; pStudioHDR = modelinfo->GetStudiomodel(mdl);
	if (!pStudioHDR)
		return false;

	mstudiohitboxset_t* pStudioSet; pStudioSet = pStudioHDR->pHitboxSet(pEnt->HitboxSet());
	if (!pStudioSet)
		return false;

	IBoneCache& setup = *pSetup;
	pEnt->SetupBones(setup.bones.data(), MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, gpGlobals->curtime);
	
	mstudiobbox_t* bbox = pStudioSet->pHitbox(index);
	pSetup->bbox = bbox;

	VectorTransform(bbox->bbmin, setup.bones[bbox->bone], setup.min);
	VectorTransform(bbox->bbmax, setup.bones[bbox->bone], setup.max);
	pSetup->center = (setup.max + setup.min) / 2;

	return true;
}