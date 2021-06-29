#pragma once

#include "ClientEntity.hpp"
#include "HandleEntity.hpp"
#include "BoneCache.hpp"
#include "Const.hpp"

#include "Helper/GameProp.hpp"
#include "Helper/Offsets.hpp"
#include "Helper/Color.hpp"


class IGlowObject;
class IBoneCache;
class ITFParticleFactory;

enum class TFTeam
{
	UNASSIGNED,
	SPECTATOR,
	RED,
	BLU,

	MAX_TEAMS
};

enum class PropType : char8_t
{
	Recv,
	Data
};

enum class PlayerHitboxIdx
{
	Invalid = -1,
	Head,
	Pelvis,
	Spine_0,
	Spine_1,
	Spine_2,
	Spine_3,
	UpperArm_L,
	LowerArm_L,
	Hand_L,
	UpperArm_R,
	LowerArm_R,
	Hand_R,
	Hip_L,
	Knee_L,
	Foot_L,
	Hip_R,
	Knee_R,
	Foot_R
};

namespace TFTeamsNames
{
	constexpr const char* CStrings[]{
		"Spectator",
		"Red",
		"Blue"
	};

	constexpr const wchar_t* WStrings[]{
		L"Spectator",
		L"Red",
		L"Blue"
	};
}

class IBaseEntityInternal : public IClientEntity
{
public:
	static IBaseEntityInternal* GetEntity(int);
	static IBaseEntityInternal* GetEntity(IBaseHandle);
	static IBaseEntityInternal* GetLocalPlayer();
	static int					GetHighestEntityIndex();

	static bool			BadEntity(IBaseEntityInternal*);
	static bool			BadLocal();


	IGlowObject*	AllocateVirtualGlowObject(const color::u8rgba&, bool create_if_deleted = true);
	IGlowObject*	QueryVirtualGlowObject();
	void			DestroyVirtualGlowObject();

	void UpdateGlowEffect() noexcept;
	void DestroyGlowEffect() noexcept;

	const EntityDataMap* GetDataMapDesc() const noexcept;

	template<PropType type>
	bool HasEntProp(const char* prop) const
	{
		IGamePropHelper prop_finder;
		if constexpr (type == PropType::Recv)
		{
			CachedRecvInfo info;
			return prop_finder.FindRecvProp(this->GetClientClass(), prop, &info) ? true : false;
		}
		else if constexpr (type == PropType::Data)
		{
			CachedDataMapInfo info;
			return prop_finder.FindDataMap(this->GetDataMapDesc(), prop, &info, DataMapType::DataMap) ? true : false;
		}
		else
			return false;
	}

	bool IsClassID(EntClassID id) const 
	{
		return this->GetClientClass()->ClassID == id;
	}

	void DrawHitboxes(const std::array<char8_t, 4>(&colors)[8], float duration);

	void DrawHitboxes(const std::array<char8_t, 3>(&colors)[8], float duration);

	void DrawHitboxes(float duration)
	{
		constexpr std::array<char8_t, 3> colors[]
		{
			{ 255, 255, 255 },
			{ 255, 175, 255 },
			{ 175, 175, 175 },
			{ 255, 175, 175 },
			{ 175, 175, 255 },
			{ 255, 175, 255 },
			{ 175, 255, 255 },
			{ 255, 255, 255 }
		};
		DrawHitboxes(colors, duration);
	}

	const IBoneCache* GetBoneCache() const;
	const IBoneInfo QueryBoneInfo() const;
	bool GetBonePosition(PlayerHitboxIdx, BoneResult*) const;

	void SetModel(int modelidx);
	bool IsHealthKit() const noexcept;
	
	GAMEPROP_DECL_RECV(int,					"CBaseAnimating", "m_nModelIndex",		ModelIndex);
	GAMEPROP_DECL_RECV(Vector,				"CBaseAnimating", "m_vecOrigin",		VecOrigin);
	GAMEPROP_DECL_RECV(QAngle,				"CBaseAnimating", "m_angRotation",		AngRotation);

	GAMEPROP_DECL_RECV(ICollideable,		"CBaseAnimating", "m_Collision",		CollisionProp);
	GAMEPROP_DECL_RECV(int,					"CBaseAnimating", "m_CollisionGroup",	CollisionGroup);
	GAMEPROP_DECL_DTM(int,									  "m_iEFlags",			EFlags);

	GAMEPROP_DECL_RECV(float,				"CBaseAnimating", "m_flSimulationTime",	SimulationTime);
	GAMEPROP_DECL_RECV(float,				"CBaseAnimating", "m_flAnimTime",		AnimationTime);
	GAMEPROP_DECL_RECV(float,				"CBaseAnimating", "m_flCycle",			Cycle);
	GAMEPROP_DECL_RECV(int,					"CBaseAnimating", "m_nSequence",		Sequence);

	GAMEPROP_DECL_RECV(TFTeam,				"CBaseAnimating", "m_iTeamNum",			TeamNum);
	GAMEPROP_DECL_RECV(IBaseHandle,			"CBaseAnimating", "m_hOwnerEntity",		OwnerEntity);
	GAMEPROP_DECL_RECV(int,					"CBaseAnimating", "m_nHitboxSet",		HitboxSet);

	GAMEPROP_DECL_RECV(ITFParticleFactory,	"CBaseAnimating", "m_flElasticity",		ParticleProp, Offsets::IBaseEntity::m_flElasticity__To__ParticleProp);
};


template<class EntityClassType>
class IBaseEntityWrapper
{
	template<typename OtherType>
	using IsBaseEntity = std::enable_if_t<std::is_base_of_v<IBaseEntityInternal, OtherType>>;

public:
	IBaseEntityWrapper(int index)							noexcept { set(EntityClassType::GetEntity(index)); };

	IBaseEntityWrapper(const IBaseHandle& hndl)				noexcept { set(EntityClassType::GetEntity(hndl)); };

	IBaseEntityWrapper(EntityClassType* ptr)				noexcept { set(ptr); };
	IBaseEntityWrapper(const EntityClassType* ptr)			noexcept { set(ptr); };
	IBaseEntityWrapper(void* ptr)							noexcept { set(reinterpret_cast<EntityClassType*>(ptr)); };
	IBaseEntityWrapper<EntityClassType>& operator=(void* p) noexcept { set(reinterpret_cast<EntityClassType*>(p)); return *this; }

	IBaseEntityWrapper(nullptr_t)							noexcept { set(nullptr); }

	IBaseEntityWrapper()									= default;
	IBaseEntityWrapper(const IBaseEntityWrapper&)			= default;
	IBaseEntityWrapper& operator=(const IBaseEntityWrapper&)= default;
	IBaseEntityWrapper(IBaseEntityWrapper&&)				= default;
	IBaseEntityWrapper& operator=(IBaseEntityWrapper&&)		= default;

	template<typename OtherType, typename = IsBaseEntity<OtherType>>
	IBaseEntityWrapper(const IBaseEntityWrapper<OtherType>& o)				noexcept { set((EntityClassType*)o.get()); }
	template<typename OtherType, typename = IsBaseEntity<OtherType>>
	IBaseEntityWrapper& operator=(const IBaseEntityWrapper<OtherType>& o)	noexcept { set((EntityClassType*)o.get()); return *this; }
	template<typename OtherType, typename = IsBaseEntity<OtherType>>
	IBaseEntityWrapper(IBaseEntityWrapper<OtherType>&& o)					noexcept { set((EntityClassType*)o.get()); o.set(nullptr); }
	template<typename OtherType, typename = IsBaseEntity<OtherType>>
	IBaseEntityWrapper& operator=(IBaseEntityWrapper<OtherType>&& o)		noexcept { set((EntityClassType*)o.get()); o.set(nullptr); return *this; }

public:
	const EntityClassType* operator->() const noexcept
	{
		return Entity;
	}

	EntityClassType* operator->() noexcept
	{
		return Entity;
	}

	const EntityClassType* get() const noexcept
	{
		return Entity;
	}

	EntityClassType* get() noexcept
	{
		return Entity;
	}

	void set(EntityClassType* ent) noexcept
	{
		Entity = ent;
	}

	operator bool() const noexcept
	{
		return !EntityClassType::BadEntity(Entity);
	}

	bool operator==(const EntityClassType* other) const noexcept { return Entity == other; }

	bool operator==(const IBaseEntityWrapper&) const = default;

protected:
	EntityClassType* Entity{ nullptr };
};

using IBaseEntity = IBaseEntityWrapper<IBaseEntityInternal>;
