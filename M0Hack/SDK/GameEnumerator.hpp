#pragma once

class ISpatialPartition;
namespace Interfaces
{
	extern ISpatialPartition* SpatialPartition;
	constexpr const char* SpatialPartitionName = "SpatialPartition001";
}

class Vector;
struct GameRay;
class IBaseEntityInternal;


enum class GameTracePartition
{
	EngineSolidEdicts = (1 << 0),		// every edict_t that isn't SOLID_TRIGGER or SOLID_NOT (and static props)
	EngineTriggerEdicts = (1 << 1),		// every edict_t that IS SOLID_TRIGGER
	ClientSolidEdicts = (1 << 2),
	ClientResponsiveEdicts = (1 << 3),		// these are client-side only objects that respond to being forces, etc.
	EngineNonStaticEdicts = (1 << 4),		// everything in solid & trigger except the static props, includes SOLID_NOTs
	ClientStaticProps = (1 << 5),
	EngineStaticProps = (1 << 6),
	ClientNonStaticEdicts = (1 << 7),		// everything except the static props


	// These are the only handles in the spatial partition that the game is controlling (everything but static props)
	// These masks are used to handle updating the dirty spatial partition list in each game DLL
	ClientGameEdicts = ClientNonStaticEdicts | ClientResponsiveEdicts | ClientSolidEdicts,
	ServerGameEdicts = EngineSolidEdicts | EngineTriggerEdicts | EngineNonStaticEdicts,

	ClientAllEdicts = ClientGameEdicts | ClientStaticProps, // Use this to look for all client edicts.
};

//-----------------------------------------------------------------------------
// Clients that want to know about all elements within a particular
// volume must inherit from this
//-----------------------------------------------------------------------------

enum class IterationRetval
{
	Continue,
	Stop
};

using SpatialPartitionHandle = unsigned short;
using SpatialPartitionListMask = int;
using SpatialTempHandle = int;

constexpr SpatialPartitionHandle PARTITION_INVALID_HANDLE = static_cast<SpatialPartitionHandle>(~0);

//-----------------------------------------------------------------------------
// Any search in the CSpatialPartition must use this to filter out entities it doesn't want.
// You're forced to use listMasks because it can filter by listMasks really fast. Any other
// filtering can be done by EnumElement.
//-----------------------------------------------------------------------------

class IPartitionEnumerator
{
public:
	virtual IterationRetval EnumElement(IBaseEntityInternal* pEntity) abstract;
};


//-----------------------------------------------------------------------------
// Installs a callback to call right before a spatial partition query occurs
//-----------------------------------------------------------------------------
class IPartitionQueryCallback
{
public:
	virtual void OnPreQuery_V1() abstract;
	virtual void OnPreQuery(SpatialPartitionListMask listMask) abstract;
	virtual void OnPostQuery(SpatialPartitionListMask listMask) abstract;
};


//-----------------------------------------------------------------------------
// This is the spatial partition manager, groups objects into buckets
//-----------------------------------------------------------------------------
class ISpatialPartition
{
public:
	// Add a virtual destructor to silence the clang warning.
	// This is harmless but not important since the only derived class
	// doesn't have a destructor.
	virtual ~ISpatialPartition() = default;

	// Create/destroy a handle for this dude in our system. Destroy
	// will also remove it from all lists it happens to be in
	virtual SpatialPartitionHandle CreateHandle(IBaseEntityInternal* pEntity) abstract;

	// A fast method of creating a handle + inserting into the tree in the right place
	virtual SpatialPartitionHandle CreateHandle(IBaseEntityInternal* pEntity,
		SpatialPartitionListMask listMask, const Vector& mins, const Vector& maxs) abstract;

	virtual void DestroyHandle(SpatialPartitionHandle handle) abstract;

	// Adds, removes an handle from a particular spatial partition list
	// There can be multiple partition lists; each has a unique id
	virtual void Insert(SpatialPartitionListMask listMask,
		SpatialPartitionHandle handle) abstract;
	virtual void Remove(SpatialPartitionListMask listMask,
		SpatialPartitionHandle handle) abstract;

	// Same as calling Remove() then Insert(). For performance-sensitive areas where you want to save a call.
	virtual void RemoveAndInsert(SpatialPartitionListMask removeMask, SpatialPartitionListMask insertMask,
		SpatialPartitionHandle handle) abstract;

	// This will remove a particular handle from all lists
	virtual void Remove(SpatialPartitionHandle handle) abstract;

	// Call this when an entity moves...
	virtual void ElementMoved(SpatialPartitionHandle handle,
		const Vector& mins, const Vector& maxs) abstract;

	// A fast method to insert + remove a handle from the tree...
	// This is used to suppress collision of a single model..
	virtual SpatialTempHandle HideElement(SpatialPartitionHandle handle) abstract;
	virtual void UnhideElement(SpatialPartitionHandle handle, SpatialTempHandle tempHandle) abstract;

	// Installs callbacks to get called right before a query occurs
	virtual void InstallQueryCallback_V1(IPartitionQueryCallback* pCallback) abstract;
	virtual void RemoveQueryCallback(IPartitionQueryCallback* pCallback) abstract;

	// Gets all entities in a particular volume...
	// if coarseTest == true, it'll return all elements that are in
	// spatial partitions that intersect the box
	// if coarseTest == false, it'll return only elements that truly intersect
	virtual void EnumerateElementsInBox(
		SpatialPartitionListMask listMask,
		const Vector& mins,
		const Vector& maxs,
		bool coarseTest,
		IPartitionEnumerator* pIterator
		) abstract;

	virtual void EnumerateElementsInSphere(
		SpatialPartitionListMask listMask,
		const Vector& origin,
		float radius,
		bool coarseTest,
		IPartitionEnumerator* pIterator
		) abstract;

	virtual void EnumerateElementsAlongRay(
		SpatialPartitionListMask listMask,
		const GameRay& ray,
		bool coarseTest,
		IPartitionEnumerator* pIterator
		) abstract;

	virtual void EnumerateElementsAtPoint(
		SpatialPartitionListMask listMask,
		const Vector& pt,
		bool coarseTest,
		IPartitionEnumerator* pIterator
		) abstract;

	// For debugging.... suppress queries on particular lists
	virtual void SuppressLists(SpatialPartitionListMask nListMask, bool bSuppress) abstract;
	virtual SpatialPartitionListMask GetSuppressedLists() abstract;

	virtual void RenderAllObjectsInTree(float flTime) abstract;
	virtual void RenderObjectsInPlayerLeafs(const Vector& vecPlayerMin, const Vector& vecPlayerMax, float flTime) abstract;
	virtual void RenderLeafsForRayTraceStart(float flTime) abstract;
	virtual void RenderLeafsForRayTraceEnd() abstract;
	virtual void RenderLeafsForHullTraceStart(float flTime) abstract;
	virtual void RenderLeafsForHullTraceEnd() abstract;
	virtual void RenderLeafsForBoxStart(float flTime) abstract;
	virtual void RenderLeafsForBoxEnd() abstract;
	virtual void RenderLeafsForSphereStart(float flTime) abstract;
	virtual void RenderLeafsForSphereEnd() abstract;

	virtual void RenderObjectsInBox(const Vector& vecMin, const Vector& vecMax, float flTime) abstract;
	virtual void RenderObjectsInSphere(const Vector& vecCenter, float flRadius, float flTime) abstract;
	virtual void RenderObjectsAlongRay(const GameRay& ray, float flTime) abstract;

	virtual void ReportStats(const char* pFileName) abstract;

	virtual void InstallQueryCallback(IPartitionQueryCallback* pCallback) abstract;
};



