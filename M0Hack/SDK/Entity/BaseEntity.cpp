#include "BaseEntity.hpp"
#include "Glow.hpp"

#include "Helper/Offsets.hpp"
#include "Library/Lib.hpp"

#include "ModelInfo.hpp"
#include "DebugOverlay.hpp"

#include "EntityList.hpp"
#include "cdll_int.hpp"

#include "GlobalHook/listener.hpp"
#include <iostream>


GAMEPROP_IMPL_RECV(IBaseEntityInternal, ModelIndex);
GAMEPROP_IMPL_RECV(IBaseEntityInternal, VecOrigin);
GAMEPROP_IMPL_RECV(IBaseEntityInternal, AngRotation);

GAMEPROP_IMPL_RECV(IBaseEntityInternal, CollisionProp);
GAMEPROP_IMPL_RECV(IBaseEntityInternal, CollisionGroup);
GAMEPROP_IMPL_DTM(IBaseEntityInternal, EFlags);

GAMEPROP_IMPL_RECV(IBaseEntityInternal, SimulationTime);
GAMEPROP_IMPL_RECV(IBaseEntityInternal, AnimationTime);
GAMEPROP_IMPL_RECV(IBaseEntityInternal, Cycle);
GAMEPROP_IMPL_RECV(IBaseEntityInternal, Sequence);

GAMEPROP_IMPL_RECV(IBaseEntityInternal, TeamNum);
GAMEPROP_IMPL_RECV(IBaseEntityInternal, OwnerEntity);
GAMEPROP_IMPL_RECV(IBaseEntityInternal, HitboxSet);

GAMEPROP_IMPL_RECV(IBaseEntityInternal, ParticleProp);


struct IClientPointers
{
	void Init()
	{
		M0Library clientdll{ M0CLIENT_DLL };
		pHighestEntity = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(clientdll.FindInterface("pCEntityListPtr")) + Offsets::ClientDLL::ClientEntList::HighestEntityIndex);
		
		pLocalPlayer = static_cast<IBaseEntityInternal**>(clientdll.FindInterface("pLocalPlayer"));
	}

	int* pHighestEntity{};
	IBaseEntityInternal** pLocalPlayer{};
};
static IClientPointers client_pointers;

class OnLoadDLLCallback
{
public:
	OnLoadDLLCallback()
	{
		M0EventManager::AddListener(EVENT_KEY_LOAD_DLL_EARLY, [](M0EventData*) { client_pointers.Init(); }, EVENT_NULL_NAME);
	}
} static loaddll_dummy;


bool IBaseHandle::valid_entity() const noexcept
{
	return IBaseEntityInternal::GetEntity(*this) != nullptr;
}


IBaseEntityInternal* IBaseEntityInternal::GetLocalPlayer()
{
	return *client_pointers.pLocalPlayer;
}

int IBaseEntityInternal::GetHighestEntityIndex()
{
	return *client_pointers.pHighestEntity;
}

IBaseEntityInternal* IBaseEntityInternal::GetEntity(IBaseHandle hndl)
{
	return Interfaces::EntityList->GetClientEntityFromHandle(hndl);
}

IBaseEntityInternal* IBaseEntityInternal::GetEntity(int idx)
{
	return Interfaces::EntityList->GetClientEntity(idx);
}

bool IBaseEntityInternal::BadEntity(IBaseEntityInternal* pEnt)
{
	return !pEnt || pEnt->IsDormant();
}

bool IBaseEntityInternal::BadLocal()
{
	return !Interfaces::EngineClient->IsInGame() || BadEntity(GetLocalPlayer());
}



void IBaseEntityInternal::UpdateGlowEffect() noexcept
{
	static IMemberVFuncThunk<void> update_glow(Offsets::IBaseEntity::VTIdx_UpdateGlowEffect);
	update_glow(this);
}

void IBaseEntityInternal::DestroyGlowEffect() noexcept
{
	static IMemberVFuncThunk<void> destroy_glow(Offsets::IBaseEntity::VTIdx_DestroyGlowEffect);
	destroy_glow(this);
}


const EntityDataMap* IBaseEntityInternal::GetDataMapDesc() const noexcept
{
	static IMemberVFuncThunk<const EntityDataMap*> getdatamap(Offsets::IBaseEntity::VTIdx_GetDataMapDesc);
	getdatamap(this);
}

void IBaseEntityInternal::DrawHitboxes(const std::array<char8_t, 4>(&colors)[8], float duration)
{
	const IBoneInfo binfo = QueryBoneInfo();
	if (!binfo)
		return;

	const IBoneCache* bonecache = binfo.GetBoneCache();
	const mstudiohitboxset_t* shbs = binfo.GetStudioHitbox();

	int r = 255, g{ }, b{ };
	Vector pos;
	QAngle ang;

	for (int i = 0; i < shbs->NumHitboxes; i++)
	{
		mstudiobbox_t* sbb = shbs->GetHitbox(i);

		const Matrix3x4* bone = bonecache->GetCachedBone(sbb->Bone);
		VectorHelper::MatrixAngles(*bone, ang, pos);

		int j = (sbb->Group % 8);

		NDebugOverlay::BoxAngles(pos, sbb->Min, sbb->Max, ang, colors[j][0], colors[j][1], colors[j][2], colors[j][3], duration);
	}
}

void IBaseEntityInternal::DrawHitboxes(const std::array<char8_t, 3>(&colors)[8], float duration)
{
	const IBoneInfo binfo = QueryBoneInfo();
	if (!binfo)
		return;

	const IBoneCache* bonecache = binfo.GetBoneCache();
	const mstudiohitboxset_t* shbs = binfo.GetStudioHitbox();

	int r = 255, g{ }, b{ };
	Vector pos;
	QAngle ang;

	for (int i = 0; i < shbs->NumHitboxes; i++)
	{
		mstudiobbox_t* sbb = shbs->GetHitbox(i);

		const Matrix3x4* bone = bonecache->GetCachedBone(sbb->Bone);
		VectorHelper::MatrixAngles(*bone, ang, pos);

		int j = (sbb->Group % 8);
		NDebugOverlay::BoxAngles(pos, sbb->Min, sbb->Max, ang, colors[j][0], colors[j][1], colors[j][2], colors[j][3], duration);
	}
}


const IBoneCache* IBaseEntityInternal::GetBoneCache() const
{
	static IMemberFuncThunk<IBoneCache*, studiohdr_t*> getbonecache(M0CLIENT_DLL, "CBaseEntity::GetBoneCache");
	return getbonecache(this, nullptr);
}

const IBoneInfo IBaseEntityInternal::QueryBoneInfo() const
{
	const ModelInfo* mdl = GetModel();
	if (!mdl)
		return { };

	const studiohdr_t* shdr = Interfaces::MdlInfo->GetStudiomodel(mdl);
	if (!shdr)
		return { };

	const mstudiohitboxset_t* shbs = shdr->GetHitboxSet(HitboxSet.get());
	if (!shbs)
		return { };

	const IBoneCache* cache = GetBoneCache();

	if (!cache)
		return { };

	return { cache, shbs, shdr };
}

bool IBaseEntityInternal::GetBonePosition(PlayerHitboxIdx index, BoneResult* results) const
{
	const ModelInfo* mdl = GetModel();
	if (!mdl)
		return false;

	const studiohdr_t* shdr = Interfaces::MdlInfo->GetStudiomodel(mdl);
	if (!shdr)
		return false;

	const mstudiohitboxset_t* shbs = shdr->GetHitboxSet(HitboxSet.get());
	if (!shbs)
		return false;

	const IBoneCache* cache = GetBoneCache();

	if (!cache)
		return false;

	const mstudiobbox_t* sbb = shbs->GetHitbox(static_cast<int>(index));
	const Matrix3x4* bone = cache->GetCachedBone(sbb->Bone);

	Vector pos;
	VectorHelper::MatrixPosition(*bone, pos);

	results->Position = pos;
	results->Min = sbb->Min;
	results->Max = sbb->Max;

	return true;
}

void IBaseEntityInternal::SetModel(int index)
{
	static IMemberFuncThunk<void, int> setmodelidx(M0CLIENT_DLL, "CBaseEntity::SetModelIndex");
	setmodelidx(this, index);
}

bool IBaseEntityInternal::IsHealthKit() const noexcept
{
	const ModelInfo* mdl = this->GetModel();
	return mdl ? strstr(mdl->ModelName, "healthkit") != nullptr : false;

}