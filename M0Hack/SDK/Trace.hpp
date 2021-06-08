#pragma once 

#include <functional>
#include <array>

#include "Const.hpp"
#include "GameTrace.hpp"
#include "Entity/BasePlayer.hpp"
#include "MathLib/Vector3D.hpp"

class IEngineTrace;

namespace Interfaces
{
	extern IEngineTrace* ClientTrace;
	constexpr const char* ClientTraceName = "EngineTraceClient003";
}


class ICollideable;
class Vector;
struct GameRay;
class GameTrace;
namespace Trace { class ITraceFilter; }
class ITraceListData;
class IEntityEnumerator;
class IPhysCollide;

template<typename T> class ValveUtlVector;


class IEngineTrace
{
public:
	// Returns the contents mask + entity at a particular world-space position
	virtual int		GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity = NULL) abstract;

	// Get the point contents, but only test the specific entity. This works
	// on static props and brush models.
	//
	// If the entity isn't a static prop or a brush model, it returns CONTENTS_EMPTY and sets
	// bFailed to true if bFailed is non-null.
	virtual int		GetPointContents_Collideable(ICollideable* pCollide, const Vector& vecAbsPosition) abstract;

	// Traces a ray against a particular entity
	virtual void	ClipRayToEntity(const GameRay& ray, unsigned int fMask, IHandleEntity* pEnt, GameTrace* pTrace) abstract;

	// Traces a ray against a particular entity
	virtual void	ClipRayToCollideable(const GameRay& ray, unsigned int fMask, ICollideable* pCollide, GameTrace* pTrace) abstract;

	// A version that simply accepts a ray (can work as a traceline or tracehull)
	virtual void	TraceRay(const GameRay& ray, unsigned int fMask, Trace::ITraceFilter* pTraceFilter, GameTrace* pTrace) abstract;

	// A version that sets up the leaf and entity lists and allows you to pass those in for collision.
	virtual void	SetupLeafAndEntityListRay(const GameRay& ray, ITraceListData& traceData) abstract;
	virtual void    SetupLeafAndEntityListBox(const Vector& vecBoxMin, const Vector& vecBoxMax, ITraceListData& traceData) abstract;
	virtual void	TraceRayAgainstLeafAndEntityList(const GameRay& ray, ITraceListData& traceData, unsigned int fMask, Trace::ITraceFilter* pTraceFilter, GameTrace* pTrace) abstract;

	// A version that sweeps a collideable through the world
	// abs start + abs end represents the collision origins you want to sweep the collideable through
	// vecAngles represents the collision angles of the collideable during the sweep
	virtual void	SweepCollideable(ICollideable* pCollide, const Vector& vecAbsStart, const Vector& vecAbsEnd,
		const QAngle& vecAngles, unsigned int fMask, Trace::ITraceFilter* pTraceFilter, GameTrace* pTrace) abstract;

	// Enumerates over all entities along a ray
	// If triggers == true, it enumerates all triggers along a ray
	virtual void	EnumerateEntities(const GameRay& ray, bool triggers, IEntityEnumerator* pEnumerator) abstract;

	// Same thing, but enumerate entitys within a box
	virtual void	EnumerateEntities(const Vector& vecAbsMins, const Vector& vecAbsMaxs, IEntityEnumerator* pEnumerator) abstract;

	// Convert a handle entity to a collideable.  Useful inside enumer
	virtual ICollideable* GetCollideable(IHandleEntity* pEntity) abstract;

	// HACKHACK: Temp for performance measurments
	virtual int GetStatByIndex(int index, bool bClear) abstract;


	//finds brushes in an AABB, prone to some false positives
	virtual void GetBrushesInAABB(const Vector& vMins, const Vector& vMaxs, ValveUtlVector<int>* pOutput, int iContentsMask = 0xFFFFFFFF) abstract;

	//Creates a CPhysCollide out of all displacements wholly or partially contained in the specified AABB
	virtual IPhysCollide* GetCollidableFromDisplacementsInAABB(const Vector& vMins, const Vector& vMaxs) abstract;

	//retrieve brush planes and contents, returns true if data is being returned in the output pointers, false if the brush doesn't exist
	virtual bool GetBrushInfo(int iBrush, ValveUtlVector<Vector4D>* pPlanesOut, int* pContentsOut) abstract;

	virtual bool PointOutsideWorld(const Vector& ptTest) abstract; //Tests a point to see if it's outside any playable area

	// Walks bsp to find the leaf containing the specified point
	virtual int GetLeafContainingPoint(const Vector& ptTest) abstract;
};


struct GameRay
{
	VectorAligned  Start;	// starting point, centered within the extents
	VectorAligned  Delta;	// direction + length of the ray
	VectorAligned  StartOffset;	// Add this to m_Start to get the actual ray start
	VectorAligned  Extents;	// Describes an axis aligned box extruded along a ray
	bool		   IsRay;	// are the extents zero?
	bool		   IsSwept;	// is delta != 0?

	GameRay() = default;
	GameRay(const Vector& start, const Vector& end) noexcept { Init(start, end); }
	GameRay(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs) noexcept { Init(start, end, mins, maxs); }

	void Init(const Vector& start, const Vector& end) noexcept
	{ 
		Delta = end - start;

		IsSwept = (Delta.LengthSqr() != 0);

		Extents = 0;
		IsRay = true;

		StartOffset = 0;
		Start = start;
	}

	void Init(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs) noexcept
	{
		Delta = end - start;

		IsSwept = (Delta.LengthSqr() != 0);

		Extents = (maxs - mins) / 2;
		IsRay = (Extents.LengthSqr() < 1e-6);

		StartOffset = (maxs + mins) / 2;
		Start = start + StartOffset;
		StartOffset.Negate();
	
	}

	// compute inverse delta
	Vector InvDelta() const noexcept
	{
		Vector vecInvDelta;
		for (int i = 0; i < 3; ++i)
		{
			if (Delta[i] != 0.0f)
				vecInvDelta[i] = 1.0f / Delta[i];
			else
				vecInvDelta[i] = FLT_MAX;
		}
		return vecInvDelta;
	}
};


namespace Trace
{
	using ShouldHitCallback = std::function<bool(IBaseEntity, int)>;

	enum class GameTraceType
	{
		Everything = 0,
		WorldOnly,				// NOTE: This does *not* test static props!!!
		EntitiesOnly,			// NOTE: This version will *not* test static props
		EverythingFilterProps,	// NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
	};

	class ITraceFilter
	{
	public:
		virtual bool ShouldHitEntity(IBaseEntityInternal* pEntity, int  contentsMask) abstract;
		virtual GameTraceType GetTraceType() const abstract;
	};

	class GenericFilter : public ITraceFilter
	{
	public:
		GameTraceType GetTraceType() const override
		{
			return GameTraceType::Everything;
		}
	};


	class FilterEntitiesOnly : public ITraceFilter
	{
	public:
		GameTraceType GetTraceType() const override
		{
			return GameTraceType::EntitiesOnly;
		}
	};


	class FilterWorldOnly : public ITraceFilter
	{
	public:
		bool ShouldHitEntity(IBaseEntityInternal*, int) final
		{
			return false;
		}

		GameTraceType GetTraceType() const final
		{
			return GameTraceType::WorldOnly;
		}
	};

	class FilterWorldAndPropsOnly : public ITraceFilter
	{
	public:
		bool ShouldHitEntity(IBaseEntityInternal*, int) final
		{
			return false;
		}

		GameTraceType GetTraceType() const final
		{
			return GameTraceType::Everything;
		}
	};

	class FilterHitAll : public GenericFilter
	{
	public:
		bool ShouldHitEntity(IBaseEntityInternal*, int) final
		{
			return true;
		}
		GameTraceType GetTraceType() const final
		{
			return GameTraceType::Everything;
		}
	};

	class FilterSimple : public GenericFilter
	{
	public:
		FilterSimple(IBaseEntity pIgnore = nullptr, ShouldHitCallback&& extra = nullptr) noexcept : IgnoreEntity(pIgnore), Callback(std::move(extra)) { };

		bool ShouldHitEntity(IBaseEntityInternal* entity, int mask) override
		{
			if (entity && IgnoreEntity == entity)
				return false;

			if (Callback && !Callback(entity, mask))
				return false;
			else return true;
		}

	protected:
		IBaseEntity IgnoreEntity{ };
		ShouldHitCallback Callback;
	};

	class LocalFilterSimple : public FilterSimple
	{
	public:
		LocalFilterSimple(ShouldHitCallback&& extra = nullptr) noexcept : FilterSimple(ILocalPlayer(), std::move(extra)) { };
	};

	class FilterIgnoreAllExcept : public GenericFilter
	{
	public:
		FilterIgnoreAllExcept(std::initializer_list<IBaseEntity> entries) noexcept : Entities(entries) { };

		bool ShouldHitEntity(IBaseEntityInternal* incoming, int) override
		{
			if (!incoming)
				return false;

			for (auto entity : Entities)
				if (incoming == entity)
					return true;

			return false;
		}

		void AddEntity(IBaseEntity ent) noexcept
		{
			Entities.push_back(ent);
		}

	protected:
		std::vector<IBaseEntity> Entities;
	};
	
	class FilterIgnoreAllExceptOne : public GenericFilter
	{
	public:
		FilterIgnoreAllExceptOne(const IBaseEntity ent) noexcept : Entity(ent) { };

		bool ShouldHitEntity(IBaseEntityInternal* incoming, int) override
		{
			if (incoming == Entity)
				return true;

			return false;
		}
	protected:
		const IBaseEntity Entity;
	};

	class FilterAcceptAllExcept : public GenericFilter
	{
	public:
		FilterAcceptAllExcept(std::initializer_list<IBaseEntity> entries) noexcept : Entities(entries) { };

		bool ShouldHitEntity(IBaseEntityInternal* incoming, int) override
		{
			if (!incoming)
				return false;

			for (auto entity : Entities)
				if (incoming == entity)
					return false;

			return true;
		}

		void AddEntity(IBaseEntity ent) noexcept
		{
			Entities.push_back(ent);
		}

	protected:
		std::vector<IBaseEntity> Entities;
	};

	class FilterAcceptAllExceptOne : public GenericFilter
	{
	public:
		FilterAcceptAllExceptOne(const IBaseEntity ent) noexcept : Entity(ent) { };

		bool ShouldHitEntity(IBaseEntityInternal* incoming, int) override
		{
			if (Entity == incoming)
				return false;

			return incoming != nullptr;
		}

	protected:
		const IBaseEntity Entity;
	};

	void TraceLine(
		const Vector& vec1,
		const Vector& vec2,
		int mask,
		GameTrace* results,
		ITraceFilter* trace
	);

	void TraceHull(
		const Vector& vec1,
		const Vector& vec2,
		const Vector& mins,
		const Vector& maxs,
		int mask,
		GameTrace* results,
		ITraceFilter* trace
	);

	void DrawRay(
		const GameRay& ray,
		float duration
	);


	inline bool VecIsVisible(
		const Vector& start,
		const Vector& end,
		const IBaseEntity pEnt,
		const IBaseEntity pIgnore = nullptr,
		uint32_t mask = MASK_SHOT
	)
	{
		GameTrace res;
		FilterSimple filter(pIgnore);
		TraceLine(start, end, mask, &res, &filter);

		return res.Entity == pEnt;
	}
}

class IEntityEnumerator
{
public:
	// This gets called with each handle
	virtual bool EnumEntity(IBaseEntityInternal*) abstract;
};