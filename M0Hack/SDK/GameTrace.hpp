#pragma once

#include "Entity/EntityList.hpp"
#include "GameEnumerator.hpp"

#include "MathLib/Vector3D.hpp"
#include "UtlVector.hpp"

#include "BaseEntity.hpp"

// Note: These flags need to match the bspfile.h DISPTRI_TAG_* flags.
#define DISPSURF_FLAG_SURFACE		(1<<0)
#define DISPSURF_FLAG_WALKABLE		(1<<1)
#define DISPSURF_FLAG_BUILDABLE		(1<<2)
#define DISPSURF_FLAG_SURFPROP1		(1<<3)
#define DISPSURF_FLAG_SURFPROP2		(1<<4)

class IPhysCollide;

struct cplane_t
{
	Vector	Normal;
	float	Dist;
	uint8_t	Type;			// for fast side tests
	uint8_t	SignBits;		// signx + (signy<<1) + (signz<<1)
	uint8_t	_pad[2];
};

struct vcollide_t
{
	unsigned short SolidCount : 15;
	unsigned short IsPacked : 1;
	unsigned short DescSize;
	IPhysCollide** Solids;
	char* KeyValues;
};

struct cModelInfo
{
	Vector		Mins, Maxs;
	Vector		Origin;		// for sounds or lights
	int			HeadNode;

	vcollide_t	CollisionData;
};

struct csurface_t
{
	const char* Name;
	short		SurfaceProps;
	unsigned short	Flags; // BUGBUG: These are declared per surface, not per material, but this database is per-material now
};


class GameTrace
{
public:
	// Returns true if hEnt points at the world entity.
	// If this returns true, then you can't use GetHitBoxIndex().
	bool DidHitWorld() const;

	// Returns true if we hit something and it wasn't the world.
	bool DidHitNonWorldEntity() const;

	// Returns true if there was any kind of impact at all
	bool DidHit() const;

	bool DidHit(const IBaseEntityInternal*) const;

	// Displacement flags tests.
	bool IsDispSurface() const noexcept { return ((DispFlags & DISPSURF_FLAG_SURFACE) != 0); }
	bool IsDispSurfaceWalkable() const noexcept { return ((DispFlags & DISPSURF_FLAG_WALKABLE) != 0); }
	bool IsDispSurfaceBuildable() const noexcept { return ((DispFlags & DISPSURF_FLAG_BUILDABLE) != 0); }
	bool IsDispSurfaceProp1() const noexcept { return ((DispFlags & DISPSURF_FLAG_SURFPROP1) != 0); }
	bool IsDispSurfaceProp2() const noexcept { return ((DispFlags & DISPSURF_FLAG_SURFPROP2) != 0); }

public:

	// these members are aligned!!
	Vector			StartPos;				// start position
	Vector			EndPos;					// final position
	cplane_t		Plane;					// surface normal at impact

	float			Fraction;				// time completed, 1.0 = didn't hit anything

	int				Contents;				// contents on other side of surface hit
	unsigned short	DispFlags;				// displacement flags for marking surfaces with data

	bool			AllSolid;				// if true, plane is not valid
	bool			StartSolid;				// if true, the initial point was in a solid area

	float		Fractionleftsolid;		// time we left a solid, only valid if we started in solid
	csurface_t	Surface;				// surface hit (impact surface)

	int			HitGroup;				// 0 == generic, non-zero is specific body part
	short		PhysicsBone;			// physics bone hit by trace in studio

	IBaseEntityInternal* Entity;

	// NOTE: this member is overloaded.
	// If hEnt points at the world entity, then this is the static prop index.
	// Otherwise, this is the hitbox index.
	int			Hitbox;					// box hit by trace in studio

	GameTrace() = default;
	GameTrace(const GameTrace&) = delete;
	GameTrace& operator=(const GameTrace&) = delete;
};

inline bool GameTrace::DidHit() const
{
	return Fraction < 1.0f || AllSolid || StartSolid;
}

inline bool GameTrace::DidHit(const IBaseEntityInternal* pEnt) const
{
	return Fraction != 1.0f ? Entity == pEnt : true;
}

inline bool GameTrace::DidHitWorld() const
{
	return Entity == IBaseEntityInternal::GetEntity(0);
}
inline bool GameTrace::DidHitNonWorldEntity() const
{
	return Entity && !DidHitWorld();
}



constexpr size_t TLD_DEF_LEAF_MAX = 256;
constexpr size_t TLD_DEF_ENTITY_MAX = 1024;

class ITraceListData : public IPartitionEnumerator
{
public:
	ITraceListData(int nLeafMax = TLD_DEF_LEAF_MAX, int nEntityMax = TLD_DEF_ENTITY_MAX) noexcept
	{
		LeafList.SetSize(nLeafMax);
		EntityList.SetSize(nEntityMax);
	}

	void Reset() noexcept
	{
		LeafCount = 0;
		EntityCount = 0;
	}

	bool	IsEmpty() const noexcept { return (LeafCount == 0 && EntityCount == 0); }

	int		LeafCountMax() const noexcept { return LeafList.Count(); }
	void    LeafCountReset() noexcept { LeafCount = 0; }

	int		EntityCountMax() const noexcept { return EntityList.Count(); }
	void	EntityCountReset() noexcept { EntityCount = 0; }

	// For leaves...
	void AddLeaf(int iLeaf) noexcept
	{
		if (LeafCount >= LeafList.Count())
			LeafList.AddMultipleToTail(LeafList.Count());

		LeafList[LeafCount] = iLeaf;
		LeafCount++;
	}

	// For entities...
	IterationRetval EnumElement(IBaseEntityInternal* pHandleEntity) override
	{
		if (EntityCount >= EntityList.Count())
			EntityList.AddMultipleToTail(EntityList.Count());

		EntityList[EntityCount] = pHandleEntity;
		EntityCount++;

		return IterationRetval::Continue;
	}

public:

	int							LeafCount{ };
	ValveUtlVector<int>					LeafList;

	int							EntityCount{ };
	ValveUtlVector<IBaseEntityInternal*>	EntityList;
};


// these definitions also need to be in q_shared.h!

// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_EMPTY			0		// No contents

#define	CONTENTS_SOLID			0x1		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			0x2		// translucent, but not watery (glass)
#define	CONTENTS_AUX			0x4
#define	CONTENTS_GRATE			0x8		// alpha-tested "grate" textures.  Bullets/sight pass through, but solids don't
#define	CONTENTS_SLIME			0x10
#define	CONTENTS_WATER			0x20
#define	CONTENTS_BLOCKLOS		0x40	// block AI line of sight
#define CONTENTS_OPAQUE			0x80	// things that cannot be seen through (may be non-solid though)
#define	LAST_VISIBLE_CONTENTS	0x80

#define ALL_VISIBLE_CONTENTS (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS-1))

#define CONTENTS_TESTFOGVOLUME	0x100
#define CONTENTS_UNUSED			0x200	

// unused 
// NOTE: If it's visible, grab from the top + update LAST_VISIBLE_CONTENTS
// if not visible, then grab from the bottom.
#define CONTENTS_UNUSED6		0x400

#define CONTENTS_TEAM1			0x800	// per team contents used to differentiate collisions 
#define CONTENTS_TEAM2			0x1000	// between players and objects on different teams

// ignore CONTENTS_OPAQUE on surfaces that have SURF_NODRAW
#define CONTENTS_IGNORE_NODRAW_OPAQUE	0x2000

// hits entities which are MOVETYPE_PUSH (doors, plats, etc.)
#define CONTENTS_MOVEABLE		0x4000

// remaining contents are non-visible, and don't eat brushes
#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000

// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_0		0x40000
#define	CONTENTS_CURRENT_90		0x80000
#define	CONTENTS_CURRENT_180	0x100000
#define	CONTENTS_CURRENT_270	0x200000
#define	CONTENTS_CURRENT_UP		0x400000
#define	CONTENTS_CURRENT_DOWN	0x800000

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEBRIS			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000
#define CONTENTS_HITBOX			0x40000000	// use accurate hitboxes on trace


// NOTE: These are stored in a short in the engine now.  Don't use more than 16 bits
#define	SURF_LIGHT		0x0001		// value will hold the light strength
#define	SURF_SKY2D		0x0002		// don't draw, indicates we should skylight + draw 2d sky but not draw the 3D skybox
#define	SURF_SKY		0x0004		// don't draw, but add to skybox
#define	SURF_WARP		0x0008		// turbulent water warp
#define	SURF_TRANS		0x0010
#define SURF_NOPORTAL	0x0020	// the surface can not have a portal placed on it
#define	SURF_TRIGGER	0x0040	// FIXME: This is an xbox hack to work around elimination of trigger surfaces, which breaks occluders
#define	SURF_NODRAW		0x0080	// don't bother referencing the texture

#define	SURF_HINT		0x0100	// make a primary bsp splitter

#define	SURF_SKIP		0x0200	// completely ignore, allowing non-closed brushes
#define SURF_NOLIGHT	0x0400	// Don't calculate light
#define SURF_BUMPLIGHT	0x0800	// calculate three lightmaps for the surface for bumpmapping
#define SURF_NOSHADOWS	0x1000	// Don't receive shadows
#define SURF_NODECALS	0x2000	// Don't receive decals
#define SURF_NOCHOP		0x4000	// Don't subdivide patches on this surface 
#define SURF_HITBOX		0x8000	// surface is part of a hitbox



// -----------------------------------------------------
// spatial content masks - used for spatial queries (traceline,etc.)
// -----------------------------------------------------
#define	MASK_ALL					(0xFFFFFFFF)
// everything that is normally solid
#define	MASK_SOLID					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
// everything that blocks player movement
#define	MASK_PLAYERSOLID			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
// blocks npc movement
#define	MASK_NPCSOLID				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
// water physics in these contents
#define	MASK_WATER					(CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME)
// everything that blocks lighting
#define	MASK_OPAQUE					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE)
// everything that blocks lighting, but with monsters added.
#define MASK_OPAQUE_AND_NPCS		(MASK_OPAQUE|CONTENTS_MONSTER)
// everything that blocks line of sight for AI
#define MASK_BLOCKLOS				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_BLOCKLOS)
// everything that blocks line of sight for AI plus NPCs
#define MASK_BLOCKLOS_AND_NPCS		(MASK_BLOCKLOS|CONTENTS_MONSTER)
// everything that blocks line of sight for players
#define	MASK_VISIBLE					(MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE)
// everything that blocks line of sight for players, but with monsters added.
#define MASK_VISIBLE_AND_NPCS		(MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE)
// bullets see these as solid
#define	MASK_SHOT					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX)
// non-raycasted weapons see this as solid (includes grates)
#define MASK_SHOT_HULL				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE)
// hits solids (not grates) and passes through everything else
#define MASK_SHOT_PORTAL			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER)
// everything normally solid, except monsters (world+brush only)
#define MASK_SOLID_BRUSHONLY		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE)
// everything normally solid for player movement, except monsters (world+brush only)
#define MASK_PLAYERSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE)
// everything normally solid for npc movement, except monsters (world+brush only)
#define MASK_NPCSOLID_BRUSHONLY		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
// just the world, used for route rebuilding
#define MASK_NPCWORLDSTATIC			(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
// These are things that can split areaportals
#define MASK_SPLITAREAPORTAL		(CONTENTS_WATER|CONTENTS_SLIME)

// UNDONE: This is untested, any moving water
#define MASK_CURRENT				(CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)

// everything that blocks corpse movement
// UNDONE: Not used yet / may be deleted
#define	MASK_DEADSOLID				(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_GRATE)

