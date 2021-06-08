#pragma once

#include "MathLib/Vector3D.hpp"
#include "MathLib/Matrix3x4.hpp"
#include "Const.hpp"

class IBaseHandle;
class ICollideable;
class IClientNetworkable;
class IClientRenderable;
class IClientEntity;
class IBaseEntityInternal;
class IClientThinkable;
class IPVSNotify;
class ClientClass;
class bf_read;
class CMouthInfo;
struct SpatializationInfo;
struct ModelInfo;
struct GameRay;
class GameTrace;


class IHandleEntity
{
public:
	virtual ~IHandleEntity() = default;
	virtual void SetRefEHandle(const IBaseHandle& handle) abstract;
	virtual const IBaseHandle& GetRefEHandle() const abstract;
};

class IClientUnknown : public IHandleEntity
{
public:
	virtual ICollideable* GetCollideable() abstract;
	virtual IClientNetworkable* GetClientNetworkable() abstract;
	virtual IClientRenderable* GetClientRenderable() abstract;
	virtual IClientEntity* GetIClientEntity() abstract;
	virtual IBaseEntityInternal* GetBaseEntity() abstract;
	virtual IClientThinkable* GetClientThinkable() abstract;
};

class ICollideable
{
public:
	virtual IHandleEntity* GetEntityHandle() abstract;

	// These methods return the bounds of an OBB measured in "collision" space
	// which can be retreived through the CollisionToWorldTransform or
	// GetCollisionOrigin/GetCollisionAngles methods
	virtual const Vector& OBBMinsPreScaled() const abstract;
	virtual const Vector& OBBMaxsPreScaled() const abstract;
	virtual const Vector& OBBMins() const abstract;
	virtual const Vector& OBBMaxs() const abstract;

	// Returns the bounds of a world-space box used when the collideable is being traced
	// against as a trigger. It's only valid to call these methods if the solid flags
	// have the FSOLID_USE_TRIGGER_BOUNDS flag set.
	virtual void			WorldSpaceTriggerBounds(Vector* pVecWorldMins, Vector* pVecWorldMaxs) const abstract;

	// custom collision test
	virtual bool			TestCollision(const GameRay& ray, unsigned int fContentsMask, GameTrace& tr) abstract;

	// Perform hitbox test, returns true *if hitboxes were tested at all*!!
	virtual bool			TestHitboxes(const GameRay& ray, unsigned int fContentsMask, GameTrace& tr) abstract;

	// Returns the BRUSH model index if this is a brush model. Otherwise, returns -1.
	virtual int				GetCollisionModelIndex() abstract;

	// Return the model, if it's a studio model.
	virtual const ModelInfo* GetCollisionModel() abstract;

	// Get angles and origin.
	virtual const Vector& GetCollisionOrigin() const abstract;
	virtual const QAngle& GetCollisionAngles() const abstract;
	virtual const Matrix3x4& CollisionToWorldTransform() const abstract;

	// Return a SOLID_ define.
	virtual EntSolidType		GetSolid() const abstract;
	virtual int				GetSolidFlags() const abstract;

	// Gets at the containing class...
	virtual IClientUnknown* GetIClientUnknown() abstract;

	// We can filter out collisions based on collision group
	virtual int				GetCollisionGroup() const abstract;

	// Returns a world-aligned box guaranteed to surround *everything* in the collision representation
	// Note that this will surround hitboxes, trigger bounds, physics.
	// It may or may not be a tight-fitting box and its volume may suddenly change
	virtual void			WorldSpaceSurroundingBounds(Vector* pVecMins, Vector* pVecMaxs) abstract;

	virtual bool			ShouldTouchTrigger(int triggerSolidFlags) const abstract;

	// returns NULL unless this collideable has specified FSOLID_ROOT_PARENT_ALIGNED
	virtual const Matrix3x4* GetRootParentToWorldTransform() const abstract;
};

class IClientRenderable
{
public:
	using ClientShadowHandle = unsigned short;
	using ClientRenderHandle = unsigned short;
	using ModelInstanceHandle = unsigned short;

	static constexpr ClientRenderHandle BadHandle()
	{
		return 0xffff;
	}

	// Gets at the containing class...
	virtual IClientUnknown* GetIClientUnknown() abstract;

	// Data accessors
	virtual Vector const& GetRenderOrigin() abstract;
	virtual QAngle const& GetRenderAngles() abstract;
	virtual bool		  ShouldDraw() abstract;
	virtual bool		  IsTransparent() abstract;
	virtual bool		  UsesPowerOfTwoFrameBufferTexture() abstract;
	virtual bool		  UsesFullFrameBufferTexture() abstract;

	virtual ClientShadowHandle	GetShadowHandle() const abstract;

	// Used by the leaf system to store its render handle.
	virtual ClientRenderHandle& RenderHandle() abstract;

	// Render baby!
	virtual const ModelInfo* GetModel() const abstract;
	virtual int						DrawModel(int flags) abstract;

	// Get the body parameter
	virtual int		GetBody() abstract;

	// Determine alpha and blend amount for transparent objects based on render state info
	virtual void	ComputeFxBlend() abstract;
	virtual int		GetFxBlend() abstract;

	// Determine the color modulation amount
	virtual void	GetColorModulation(float* color) abstract;

	// Returns false if the entity shouldn't be drawn due to LOD. 
	// (NOTE: This is no longer used/supported, but kept in the vtable for backwards compat)
	virtual bool	LODTest() abstract;

	// Call this to get the current bone transforms for the model.
	// currentTime parameter will affect interpolation
	// nMaxBones specifies how many matrices pBoneToWorldOut can hold. (Should be greater than or
	// equal to studiohdr_t::numbones. Use MAXSTUDIOBONES to be safe.)
	virtual bool	SetupBones(Matrix3x4* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) abstract;

	virtual void	SetupWeights(const Matrix3x4* pBoneToWorld, int nFlexWeightCount, float* pFlexWeights, float* pFlexDelayedWeights) abstract;
	virtual void	DoAnimationEvents() abstract;

	// Return this if you want PVS notifications. See IPVSNotify for more info.	
	// Note: you must always return the same value from this function. If you don't,
	// undefined things will occur, and they won't be good.
	virtual IPVSNotify* GetPVSNotifyInterface() abstract;

	// Returns the bounds relative to the origin (render bounds)
	virtual void	GetRenderBounds(Vector& mins, Vector& maxs) abstract;

	// returns the bounds as an AABB in worldspace
	virtual void	GetRenderBoundsWorldspace(Vector& mins, Vector& maxs) abstract;

	// These normally call through to GetRenderAngles/GetRenderBounds, but some entities custom implement them.
	virtual void	GetShadowRenderBounds(Vector& mins, Vector& maxs, int shadowType) abstract;

	// Should this object be able to have shadows cast onto it?
	virtual bool	ShouldReceiveProjectedTextures(int flags) abstract;

	// These methods return true if we want a per-renderable shadow cast direction + distance
	virtual bool	GetShadowCastDistance(float* pDist, int shadowType) const abstract;
	virtual bool	GetShadowCastDirection(Vector* pDirection, int shadowType) const abstract;

	// Other methods related to shadow rendering
	virtual bool	IsShadowDirty() abstract;
	virtual void	MarkShadowDirty(bool bDirty) abstract;

	// Iteration over shadow hierarchy
	virtual IClientRenderable* GetShadowParent() abstract;
	virtual IClientRenderable* FirstShadowChild() abstract;
	virtual IClientRenderable* NextShadowPeer() abstract;

	// Returns the shadow cast type
	virtual int ShadowCastType() abstract;

	// Create/get/destroy model instance
	virtual void CreateModelInstance() abstract;
	virtual ModelInstanceHandle GetModelInstance() abstract;

	// Returns the transform from RenderOrigin/RenderAngles to world
	virtual const Matrix3x4& RenderableToWorldTransform() abstract;

	// Attachments
	virtual int LookupAttachment(const char* pAttachmentName) abstract;
	virtual	bool GetAttachment(int number, Vector& origin, QAngle& angles) abstract;
	virtual bool GetAttachment(int number, Matrix3x4& matrix) abstract;

	// Rendering clip plane, should be 4 floats, return value of NULL indicates a disabled render clip plane
	virtual float* GetRenderClipPlane() abstract;

	// Get the skin parameter
	virtual int		GetSkin() abstract;

	// Is this a two-pass renderable?
	virtual bool	IsTwoPass() abstract;

	virtual void	OnThreadedDrawSetup() abstract;

	virtual bool	UsesFlexDelayedWeights() abstract;

	virtual void	RecordToolMessage() abstract;

	virtual bool	IgnoresZBuffer() const abstract;
};



class IClientNetworkable
{
public:
	enum class ShouldTransmitState
	{
		Start,	// The entity is starting to be transmitted (maybe it entered the PVS).
		End		// Called when the entity isn't being transmitted by the server.
				// This signals a good time to hide the entity until next time
				// the server wants to transmit its state.
	};

	enum class DataUpdateType
	{
		CREATED = 0,	// indicates it was created +and+ entered the pvs
		DATATABLE_CHANGE,
	};

	// Gets at the containing class...
	virtual IClientUnknown * GetIClientUnknown() abstract;

	// Called by the engine when the server deletes the entity.
	virtual void			Release() abstract;

	// Supplied automatically by the IMPLEMENT_CLIENTCLASS macros.
	virtual ClientClass* GetClientClass() abstract;
	const ClientClass* GetClientClass() const
	{
		return const_cast<IClientNetworkable*>(this)->GetClientClass();
	}

	// This tells the entity what the server says for ShouldTransmit on this entity.
	// Note: This used to be EntityEnteredPVS/EntityRemainedInPVS/EntityLeftPVS.
	virtual void			NotifyShouldTransmit(ShouldTransmitState state) abstract;


	//
	// NOTE FOR ENTITY WRITERS: 
	//
	// In 90% of the cases, you should hook OnPreDataChanged/OnDataChanged instead of 
	// PreDataUpdate/PostDataUpdate.
	//
	// The DataChanged events are only called once per frame whereas Pre/PostDataUpdate
	// are called once per packet (and sometimes multiple times per frame).
	//
	// OnDataChanged is called during simulation where entity origins are correct and 
	// attachments can be used. whereas PostDataUpdate is called while parsing packets
	// so attachments and other entity origins may not be valid yet.
	//

	virtual void			OnPreDataChanged(DataUpdateType updateType) abstract;
	virtual void			OnDataChanged(DataUpdateType updateType) abstract;

	// Called when data is being updated across the network.
	// Only low-level entities should need to know about these.
	virtual void			PreDataUpdate(DataUpdateType updateType) abstract;
	virtual void			PostDataUpdate(DataUpdateType updateType) abstract;


	// Objects become dormant on the client if they leave the PVS on the server.
	virtual bool			IsDormant() abstract;

	// Ent Index is the server handle used to reference this entity.
	// If the index is < 0, that indicates the entity is not known to the server
	virtual int				entindex() const abstract;

	// Server to client entity message received
	virtual void			ReceiveMessage(int classID, bf_read& msg) abstract;

	// Get the base pointer to the networked data that GetClientClass->m_pRecvTable starts at.
	// (This is usually just the "this" pointer).
	virtual void* GetDataTableBasePtr() abstract;

	// Tells the entity that it's about to be destroyed due to the client receiving
	// an uncompressed update that's caused it to destroy all entities & recreate them.
	virtual void			SetDestroyedOnRecreateEntities(void) abstract;

	virtual void			OnDataUnchangedInPVS() abstract;
};

class IClientThinkable
{
public:
	class IClientThinkHandle;

	// Gets at the containing class...
	virtual IClientUnknown*		GetIClientUnknown() abstract;

	virtual void				ClientThink() abstract;

	// Called when you're added to the think list.
	// GetThinkHandle's return value must be initialized to INVALID_THINK_HANDLE.
	virtual IClientThinkHandle*	GetThinkHandle() abstract;
	virtual void				SetThinkHandle(IClientThinkHandle* hThink) abstract;

	// Called by the client when it deletes the entity.
	virtual void				Release() abstract;
};
