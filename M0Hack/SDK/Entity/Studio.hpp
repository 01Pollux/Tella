#pragma once

#include "MathLib/Vector3D.hpp"
#include "MathLib/VMatrix.hpp"
#include "UtlVector.hpp"

#define STUDIO_SEQUENCE_ACTIVITY_LOOKUPS_ARE_SLOW 0 
#define STUDIO_SEQUENCE_ACTIVITY_LAZY_INITIALIZE 1

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------

class IMaterial;
class IMesh;
class IMorph;
struct virtualmdl_t;

namespace OptimizedModel
{
	struct StripHeader_t;
}



#define STUDIO_VERSION		48

#define MAXSTUDIOTRIANGLES	65536
#define MAXSTUDIOVERTS		65536
#define	MAXSTUDIOFLEXVERTS	10000	// max number of verts that can be flexed per mesh.  TODO: tune this
#define MAXSTUDIOSKINS		32		// total textures
#define MAXSTUDIOBONES		128		// total bones actually used
#define MAXSTUDIOFLEXDESC	1024	// maximum number of low level flexes (actual morph targets)
#define MAXSTUDIOFLEXCTRL	96		// maximum number of flexcontrollers (input sliders)
#define MAXSTUDIOPOSEPARAM	24
#define MAXSTUDIOBONECTRLS	4
#define MAXSTUDIOANIMBLOCKS 256

#define MAXSTUDIOBONEBITS	7		// NOTE: MUST MATCH MAXSTUDIOBONES

// NOTE!!! : Changing this number also changes the vtx file format!!!!!
#define MAX_NUM_BONES_PER_VERT 3

//Adrian - Remove this when we completely phase out the old event system.
#define NEW_EVENT_STYLE ( 1 << 10 )

struct mstudiodata_t
{
	int		count;
	int		offset;
};

#define STUDIO_PROC_AXISINTERP	1
#define STUDIO_PROC_QUATINTERP	2
#define STUDIO_PROC_AIMATBONE	3
#define STUDIO_PROC_AIMATATTACH 4
#define STUDIO_PROC_JIGGLE 5

struct mstudioaxisinterpbone_t
{
	int				Control;// local transformation of this bone used to calc 3 point blend
	int				Axis;	// axis to check
	Vector			Pos[6];	// X+, X-, Y+, Y-, Z+, Z-
	Quaternion		Quat[6];// X+, X-, Y+, Y-, Z+, Z-

	mstudioaxisinterpbone_t() {}
private:
	// No copy constructors allowed
	mstudioaxisinterpbone_t(const mstudioaxisinterpbone_t& vOther);
};


struct mstudioquatinterpinfo_t
{
	float			InvTolerance;	// 1 / radian angle of trigger influence
	Quaternion		Trigger;	// angle to match
	Vector			Pos;		// new position
	Quaternion		Quat;		// new angle

	mstudioquatinterpinfo_t() {}
private:
	// No copy constructors allowed
	mstudioquatinterpinfo_t(const mstudioquatinterpinfo_t& vOther);
};

struct mstudioquatinterpbone_t
{
	int				Control;// local transformation to check
	int				NumTriggers;
	int				TriggerIndex;
	inline mstudioquatinterpinfo_t* GetTrigger(int i) const { return  (mstudioquatinterpinfo_t*)(((unsigned char*)this) + TriggerIndex) + i; };

	mstudioquatinterpbone_t() {}
private:
	// No copy constructors allowed
	mstudioquatinterpbone_t(const mstudioquatinterpbone_t& vOther);
};


#define JIGGLE_IS_FLEXIBLE				0x01
#define JIGGLE_IS_RIGID					0x02
#define JIGGLE_HAS_YAW_CONSTRAINT		0x04
#define JIGGLE_HAS_PITCH_CONSTRAINT		0x08
#define JIGGLE_HAS_ANGLE_CONSTRAINT		0x10
#define JIGGLE_HAS_LENGTH_CONSTRAINT	0x20
#define JIGGLE_HAS_BASE_SPRING			0x40
#define JIGGLE_IS_BOING					0x80		// simple squash and stretch sinusoid "boing"

struct mstudiojigglebone_t
{
	int				Flags;

	// general params
	float			Length;					// how from from bone base, along bone, is tip
	float			TipMass;

	// flexible params
	float			YawStiffness;
	float			YawDamping;
	float			PitchStiffness;
	float			PitchDamping;
	float			AlongStiffness;
	float			AlongDamping;

	// angle constraint
	float			AngleLimit;				// maximum deflection of tip in radians

	// yaw constraint
	float			MinYaw;					// in radians
	float			MaxYaw;					// in radians
	float			YawFriction;
	float			YawBounce;

	// pitch constraint
	float			MinPitch;				// in radians
	float			MaxPitch;				// in radians
	float			PitchFriction;
	float			PitchBounce;

	// base spring
	float			BaseMass;
	float			BaseStiffness;
	float			BaseDamping;
	float			BaseMinLeft;
	float			BaseMaxLeft;
	float			BaseLeftFriction;
	float			BaseMinUp;
	float			BaseMaxUp;
	float			BaseUpFriction;
	float			BaseMinForward;
	float			BaseMaxForward;
	float			BaseForwardFriction;

	// boing
	float			BoingImpactSpeed;
	float			BoingImpactAngle;
	float			BoingDampingRate;
	float			BoingFrequency;
	float			BoingAmplitude;

private:
	// No copy constructors allowed
	//mstudiojigglebone_t(const mstudiojigglebone_t& vOther);
};

struct mstudioaimatbone_t
{
	int				Parent;
	int				Aim;		// Might be bone or attach
	Vector			AimVector;
	Vector			UpVector;
	Vector			BasePos;

	mstudioaimatbone_t() {}
private:
	// No copy constructors allowed
	mstudioaimatbone_t(const mstudioaimatbone_t& vOther);
};

// bones
struct mstudiobone_t
{
	int					NameIndex;
	inline char* const	GetName() const { return ((char*)this) + NameIndex; }
	int		 			Parent;		// parent bone
	int					BoneController[6];	// bone controller index, -1 == none

	// default values
	Vector				Pos;
	Quaternion			Quat;
	RadianEuler			Rot;
	// compression scale
	Vector				Posscale;
	Vector				rotscale;

	Matrix3x4			PoseToBone;
	Quaternion			Alignment;
	int					Flags;
	int					ProcType;
	int					ProcIndex;		// procedural rule
	mutable int			PhysicsBone;	// index into physically simulated bone
	inline void*		GetProcedure() const { if (ProcIndex == 0) return NULL; else return  (void*)(((unsigned char*)this) + ProcIndex); };
	int					SurfacePropIndex;	// index into string tablefor property name
	inline char* const	GetSurfaceProp() const { return ((char*)this) + SurfacePropIndex; }
	int					Contents;		// See BSPFlags.h for the contents flags

	int					unused[8];		// remove as appropriate

	mstudiobone_t() {}
private:
	// No copy constructors allowed
	mstudiobone_t(const mstudiobone_t& vOther);
};

struct mstudiolinearbone_t
{
	int Numbones;

	int FlagsIndex;
	inline int* GetpFlags(int i) { return ((int*)(((unsigned char*)this) + FlagsIndex) + i); };
	inline int	GetFlags(int i) const { return *((int*)(((unsigned char*)this) + FlagsIndex) + i); };

	int	ParentIndex;
	inline int parent(int i) const { return *((int*)(((unsigned char*)this) + ParentIndex) + i); };

	int	PosIndex;
	inline Vector& GetPos(int i) const { return *((Vector*)(((unsigned char*)this) + PosIndex) + i); };

	int QuatIndex;
	inline Quaternion& GetQuat(int i) const { return *((Quaternion*)(((unsigned char*)this) + QuatIndex) + i); };

	int RotIndex;
	inline RadianEuler& GetRot(int i) const { return *((RadianEuler*)(((unsigned char*)this) + RotIndex) + i); };

	int PosToBoneIndex;
	inline Matrix3x4& poseToBone(int i) const { return *((Matrix3x4*)(((unsigned char*)this) + PosToBoneIndex) + i); };

	int	PosScaleIndex;
	inline Vector& GetPosScale(int i) const { return *((Vector*)(((unsigned char*)this) + PosScaleIndex) + i); };

	int	RotScaleIndex;
	inline Vector& rotscale(int i) const { return *((Vector*)(((unsigned char*)this) + RotScaleIndex) + i); };

	int	QalignmentIndex;
	inline Quaternion& qalignment(int i) const { return *((Quaternion*)(((unsigned char*)this) + QalignmentIndex) + i); };

	int unused[6];

	mstudiolinearbone_t() {}
private:
	// No copy constructors allowed
	mstudiolinearbone_t(const mstudiolinearbone_t& vOther);
};


//-----------------------------------------------------------------------------
// The component of the bone used by mstudioboneflexdriver_t
//-----------------------------------------------------------------------------
enum class StudioBoneFlexComponent
{
	INVALID = -1,	// Invalid
	TX = 0,			// Translate X
	TY = 1,			// Translate Y
	TZ = 2			// Translate Z
};


//-----------------------------------------------------------------------------
// Component is one of Translate X, Y or Z [0,2] (StudioBoneFlexComponent_t)
//-----------------------------------------------------------------------------
struct mstudioboneflexdrivercontrol_t
{
	int BoneComponent;		// Bone component that drives flex, StudioBoneFlexComponent_t
	int FlexControllerIndex;// Flex controller to drive
	float Min;				// Min value of bone component mapped to 0 on flex controller
	float Max;				// Max value of bone component mapped to 1 on flex controller

	mstudioboneflexdrivercontrol_t() {}
private:
	// No copy constructors allowed
	mstudioboneflexdrivercontrol_t(const mstudioboneflexdrivercontrol_t& vOther);
};


//-----------------------------------------------------------------------------
// Drive flex controllers from bone components
//-----------------------------------------------------------------------------
struct mstudioboneflexdriver_t
{
	int BoneIndex;			// Bone to drive flex controller
	int ControlCount;		// Number of flex controllers being driven
	int ControlIndex;		// Index into data where controllers are (relative to this)

	inline mstudioboneflexdrivercontrol_t* GetBoneFlexDriverControl(int i) const
	{
		return (mstudioboneflexdrivercontrol_t*)(((unsigned char*)this) + ControlIndex) + i;
	}

	int unused[3];

	mstudioboneflexdriver_t() {}
private:
	// No copy constructors allowed
	mstudioboneflexdriver_t(const mstudioboneflexdriver_t& vOther);
};


#define BONE_CALCULATE_MASK			0x1F
#define BONE_PHYSICALLY_SIMULATED	0x01	// bone is physically simulated when physics are active
#define BONE_PHYSICS_PROCEDURAL		0x02	// procedural when physics is active
#define BONE_ALWAYS_PROCEDURAL		0x04	// bone is always procedurally animated
#define BONE_SCREEN_ALIGN_SPHERE	0x08	// bone aligns to the screen, not constrained in motion.
#define BONE_SCREEN_ALIGN_CYLINDER	0x10	// bone aligns to the screen, constrained by it's own axis.

#define BONE_USED_MASK				0x0007FF00
#define BONE_USED_BY_ANYTHING		0x0007FF00
#define BONE_USED_BY_HITBOX			0x00000100	// bone (or child) is used by a hit box
#define BONE_USED_BY_ATTACHMENT		0x00000200	// bone (or child) is used by an attachment point
#define BONE_USED_BY_VERTEX_MASK	0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0	0x00000400	// bone (or child) is used by the toplevel model via skinned vertex
#define BONE_USED_BY_VERTEX_LOD1	0x00000800	
#define BONE_USED_BY_VERTEX_LOD2	0x00001000  
#define BONE_USED_BY_VERTEX_LOD3	0x00002000
#define BONE_USED_BY_VERTEX_LOD4	0x00004000
#define BONE_USED_BY_VERTEX_LOD5	0x00008000
#define BONE_USED_BY_VERTEX_LOD6	0x00010000
#define BONE_USED_BY_VERTEX_LOD7	0x00020000
#define BONE_USED_BY_BONE_MERGE		0x00040000	// bone is available for bone merge to occur against it

#define BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )

#define MAX_NUM_LODS 8

#define BONE_TYPE_MASK				0x00F00000
#define BONE_FIXED_ALIGNMENT		0x00100000	// bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

#define BONE_HAS_SAVEFRAME_POS		0x00200000	// Vector48
#define BONE_HAS_SAVEFRAME_ROT		0x00400000	// Quaternion64

// bone controllers
struct mstudiobonecontroller_t
{
	int					Bone;	// -1 == 0
	int					Type;	// X, Y, Z, XR, YR, ZR, M
	float				Start;
	float				End;
	int					Rest;	// unsigned char index value at rest
	int					InputField;	// 0-3 user set controller, 4 mouth
	int					unused[8];
};

// intersection boxes
struct mstudiobbox_t
{
	int					Bone;
	int					Group;				// intersection group
	Vector				Min;				// bounding box
	Vector				Max;
	int					HitboxNameIndex;	// offset to the name of the hitbox.
	int					unused[8];

	const char*		GetHitboxName()
	{
		if (HitboxNameIndex == 0)
			return "";

		return ((const char*)this) + HitboxNameIndex;
	}

	mstudiobbox_t() {}

private:
	// No copy constructors allowed
	mstudiobbox_t(const mstudiobbox_t& vOther);
};

// demand loaded sequence groups
struct mstudiomodelgroup_t
{
	int					LabelIndex;	// textual name
	inline char* const	GetLabel() const { return ((char*)this) + LabelIndex; }
	int					NameIndex;	// file name
	inline char* const	GetName() const { return ((char*)this) + NameIndex; }
};

struct mstudiomodelgrouplookup_t
{
	int					ModelGroup;
	int					IndexWithinGroup;
};

// events
struct mstudioevent_t
{
	float				Cycle;
	int					Event;
	int					Type;
	inline const char*	GetOptions() const { return Options; }
	char				Options[64];

	int					EventIndex;
	inline char* const	GetEventName() const { return ((char*)this) + EventIndex; }
};

#define	ATTACHMENT_FLAG_WORLD_ALIGN 0x10000

// attachment
struct mstudioattachment_t
{
	int					NameIndex;
	inline char* const	GetName() const { return ((char*)this) + NameIndex; }
	unsigned int		Flags;
	int					LocalBone;
	Matrix3x4			Local; // attachment point
	int					unused[8];
};

#define IK_SELF 1
#define IK_WORLD 2
#define IK_GROUND 3
#define IK_RELEASE 4
#define IK_ATTACHMENT 5
#define IK_UNLATCH 6

struct mstudioikerror_t
{
	Vector		Pos;
	Quaternion	Quat;

	mstudioikerror_t() = default;
	mstudioikerror_t(const mstudioikerror_t&) = delete;
};

union mstudioanimvalue_t;

struct mstudiocompressedikerror_t
{
	float	Scale[6];
	short	Offset[6];
	inline mstudioanimvalue_t* GetAnimvalue(int i) const { if (Offset[i] > 0) return  (mstudioanimvalue_t*)(((unsigned char*)this) + Offset[i]); else return NULL; };
	mstudiocompressedikerror_t() = default;
	mstudiocompressedikerror_t(const mstudioikerror_t&) = delete;
};

struct mstudioikrule_t
{
	int			Index;

	int			Type;
	int			Chain;

	int			Bone;

	int			Slot;	// iktarget slot.  Usually same as chain.
	float		Height;
	float		Radius;
	float		Floor;
	Vector		Pos;
	Quaternion	Quat;

	int			CompressedikErrorIndex;
	inline mstudiocompressedikerror_t* GetCompressedError() const { return (mstudiocompressedikerror_t*)(((unsigned char*)this) + CompressedikErrorIndex); };
	int			unused2;

	int			iStart;
	int			kErrorindex;
	inline mstudioikerror_t*GetpError(int i) const { return  (kErrorindex) ? (mstudioikerror_t*)(((unsigned char*)this) + kErrorindex) + (i - iStart) : NULL; };

	float		Start;	// beginning of influence
	float		Peak;	// start of full influence
	float		Tail;	// end of full influence
	float		End;	// end of all influence

	float		unused3;	// 
	float		Contact;	// frame footstep makes ground concact
	float		Drop;		// how far down the foot should drop when reaching for IK
	float		Top;		// top of the foot box

	int			unused6;
	int			unused7;
	int			unused8;

	int			AttachmentIndex;		// name of world attachment
	inline char* const GetAttachment() const { return ((char*)this) + AttachmentIndex; }

	int			unused[7];

	mstudioikrule_t() = default;
	mstudioikrule_t(const mstudioikerror_t&) = delete;
};


struct mstudioiklock_t
{
	int			Chain;
	float		PosWeight;
	float		LocalQWeight;
	int			Flags;

	int			unused[4];
};


struct mstudiolocalhierarchy_t
{
	int			Bone;			// bone being adjusted
	int			NewParent;		// the bones new parent

	float		fStart;			// beginning of influence
	float		Peak;			// start of full influence
	float		Tail;			// end of full influence
	float		End;			// end of all influence

	int			iStart;			// first frame 

	int			LocalAnimIndex;
	inline mstudiocompressedikerror_t* GetLocalAnim() const { return (mstudiocompressedikerror_t*)(((unsigned char*)this) + LocalAnimIndex); };

	int			unused[4];
};



// animation frames
union mstudioanimvalue_t
{
	struct
	{
		unsigned char	valid;
		unsigned char	total;
	} num;
	short		value;
};

struct mstudioanim_valueptr_t
{
	short	Offset[3];
	inline mstudioanimvalue_t* GetAnimvalue(int i) const { if (Offset[i] > 0) return  (mstudioanimvalue_t*)(((unsigned char*)this) + Offset[i]); else return NULL; };
};

#define STUDIO_ANIM_RAWPOS	0x01 // Vector48
#define STUDIO_ANIM_RAWROT	0x02 // Quaternion48
#define STUDIO_ANIM_ANIMPOS	0x04 // mstudioanim_valueptr_t
#define STUDIO_ANIM_ANIMROT	0x08 // mstudioanim_valueptr_t
#define STUDIO_ANIM_DELTA	0x10
#define STUDIO_ANIM_RAWROT2	0x20 // Quaternion64


// per bone per animation DOF and weight pointers
struct mstudioanim_t
{
	unsigned char				Bone;
	unsigned char				Flags;		// weighing options

	// valid for animating data only
	inline unsigned char* GetData() const { return (((unsigned char*)this) + sizeof(*this)); };
	inline mstudioanim_valueptr_t* GetRotV() const { return (mstudioanim_valueptr_t*)(GetData()); };
	inline mstudioanim_valueptr_t* GetPosV() const { return (mstudioanim_valueptr_t*)(GetData()) + ((Flags & STUDIO_ANIM_ANIMROT) != 0); };

	// valid if animation unvaring over timeline
	inline Quaternion48* GetQuat48() const { return (Quaternion48*)(GetData()); };
	inline Quaternion64* GetQuat64() const { return (Quaternion64*)(GetData()); };
	inline void*	 GetPos() const { return (void*)(GetData() + ((Flags & STUDIO_ANIM_RAWROT) != 0) * sizeof(Quaternion48) + ((Flags & STUDIO_ANIM_RAWROT2) != 0) * sizeof(Quaternion48)); };

	short				NextOffset;
	inline mstudioanim_t* GetNext() const { if (NextOffset != 0) return  (mstudioanim_t*)(((unsigned char*)this) + NextOffset); else return NULL; };
};

struct mstudiomovement_t
{
	int					EndFrame;
	int					MotionFlags;
	float				V0;			// velocity at start of block
	float				V1;			// velocity at end of block
	float				Angle;		// YAW rotation at end of this blocks movement
	Vector				RelMovPos;		// movement vector relative to this blocks initial angle
	Vector				RelStartPos;	// relative to start of animation???

	mstudiomovement_t() = default;
	mstudiomovement_t(const mstudiomovement_t&) = delete;
};

struct studiohdr_t;

// used for piecewise loading of animation data
struct mstudioanimblock_t
{
	int					DataStart;
	int					DataEnd;
};

struct mstudioanimsections_t
{
	int					AnimBlock;
	int					AnimIndex;
};

struct mstudioanimdesc_t
{
	int					BasePtr;
	inline studiohdr_t* GetStudioHDR() const { return (studiohdr_t*)(((unsigned char*)this) + BasePtr); }

	int					NameIndex;
	inline char* const GetName() const { return ((char*)this) + NameIndex; }

	float				FPS;		// frames per second	
	int					Flags;		// looping/non-looping flags

	int					NumFrames;

	// piecewise movement
	int					NumMovements;
	int					MovementIndex;
	inline mstudiomovement_t* const GetMovement(int i) const { return (mstudiomovement_t*)(((unsigned char*)this) + MovementIndex) + i; };

	int					unused1[6];			// remove as appropriate (and zero if loading older versions)	

	int					AnimBlock;
	int					AnimIndex;	 // non-zero when anim data isn't in sections
	mstudioanim_t*		GetAnimBlock(int block, int index) const; // returns pointer to a specific anim block (local or external)

	int					NumIKRules;
	int					IKRuleIndex;	// non-zero when IK data is stored in the mdl
	int					AnimBlockikruleIndex; // non-zero when IK data is stored in animblock file

	int					NumLocalHierarchy;
	int					LocalHierarchyIndex;

	int					SectionIndex;
	int					SectionFrames; // number of frames used in each fast lookup section, zero if not used
	inline mstudioanimsections_t* const GetSection(int i) const { return (mstudioanimsections_t*)(((unsigned char*)this) + SectionIndex) + i; }

	short				ZeroFrameSpan;	// frames per span
	short				ZeroFrameCount; // number of spans
	int					ZeroFrameIndex;
	unsigned char*		GetZeroFrameData() const { if (ZeroFrameIndex) return (((unsigned char*)this) + ZeroFrameIndex); else return NULL; };
	mutable float		ZeroFrameStallTime;		// saved during read stalls

	mstudioanimdesc_t() = default;
	mstudioanimdesc_t(const mstudioanimdesc_t&) = delete;
};

struct mstudioikrule_t;

struct mstudioautolayer_t
{
	//private:
	short				Sequence;
	short				Pose;
	//public:
	int					Flags;
	float				Start;	// beginning of influence
	float				Peak;	// start of full influence
	float				Tail;	// end of full influence
	float				End;	// end of all influence
};

struct mstudioactivitymodifier_t
{
	int					NameIndex;
	inline char*		GetName() { return (NameIndex) ? (char*)(((unsigned char*)this) + NameIndex) : NULL; }
};

// sequence descriptions
struct mstudioseqdesc_t
{
	int					BasePtr;
	inline studiohdr_t* GetStudioHDR() const { return (studiohdr_t*)(((unsigned char*)this) + BasePtr); }

	int					LabelIndex;
	inline char* const	GetLabel() const { return ((char*)this) + LabelIndex; }

	int					ActivityNameIndex;
	inline char* const	GetActivityName() const { return ((char*)this) + ActivityNameIndex; }

	int					Flags;		// looping/non-looping flags

	int					Activity;	// initialized at loadtime to game DLL values
	int					ActWeight;

	int					NumEvents;
	int					EventIndex;
	inline mstudioevent_t* GetEvent(int i) const { return (mstudioevent_t*)(((unsigned char*)this) + EventIndex) + i; };

	Vector				Min;		// per sequence bounding box
	Vector				Max;

	int					NumBlends;

	// Index into array of shorts which is groupsize[0] x groupsize[1] in length
	int					AnimIndexIndex;

	inline int			GetAnim(int x, int y) const
	{
		if (x >= GroupSize[0])
		{
			x = GroupSize[0] - 1;
		}

		if (y >= GroupSize[1])
		{
			y = GroupSize[1] - 1;
		}

		int offset = y * GroupSize[0] + x;
		short* blends = (short*)(((unsigned char*)this) + AnimIndexIndex);
		int value = (int)blends[offset];
		return value;
	}

	int					MovementIndex;	// [blend] float array for blended movement
	int					GroupSize[2];
	int					ParamIndex[2];	// X, Y, Z, XR, YR, ZR
	float				ParamStart[2];	// local (0..1) starting value
	float				ParamEnd[2];	// local (0..1) ending value
	int					ParamParent;

	float				FadeInTime;		// ideal cross fate in time (0.2 default)
	float				FadeOutTime;	// ideal cross fade out time (0.2 default)

	int					LocalEntryNode;		// transition node at entry
	int					LocalExitNode;		// transition node at exit
	int					NodeFlags;		// transition rules

	float				EntryPhase;		// used to match entry gait
	float				ExitPhase;		// used to match exit gait

	float				LastFrame;		// frame that should generation EndOfSequence

	int					NextSeq;		// auto advancing sequences
	int					Pose;			// index of delta animation between end and nextseq

	int					NumIKRules;

	int					NumAutoLayers;	//
	int					AutoLayerIndex;
	inline mstudioautolayer_t* GetAutolayer(int i) const {  return (mstudioautolayer_t*)(((unsigned char*)this) + AutoLayerIndex) + i; };

	int					WeightListIndex;
	inline float*		GetBoneweight(int i) const { return ((float*)(((unsigned char*)this) + WeightListIndex) + i); };
	inline float		GetWeight(int i) const { return *(GetBoneweight(i)); };

	// FIXME: make this 2D instead of 2x1D arrays
	int					PoseKeyIndex;
	float*		GetPoseKey(int iParam, int iAnim) const { return (float*)(((unsigned char*)this) + PoseKeyIndex) + iParam * GroupSize[0] + iAnim; }
	float				GetPose(int iParam, int iAnim) const { return *(GetPoseKey(iParam, iAnim)); }

	int					numiklocks;
	int					iklockindex;
	inline mstudioiklock_t* pIKLock(int i) const { return (mstudioiklock_t*)(((unsigned char*)this) + iklockindex) + i; };

	// Key values
	int					KeyValueIndex;
	int					KeyValueSize;
	inline const char*	GetKeyValueText() const { return KeyValueSize != 0 ? ((char*)this) + KeyValueIndex : NULL; }

	int					CyclePoseIndex;		// index of pose parameter to use as cycle index

	int					ActivityModifierIndex;
	int					NumActivityModifiers;
	inline mstudioactivitymodifier_t* pActivityModifier(int i) const { return ActivityModifierIndex != 0 ? (mstudioactivitymodifier_t*)(((unsigned char*)this) + ActivityModifierIndex) + i : NULL; };

	int					unused[5];		// remove/add as appropriate (grow back to 8 ints on version change!)

	mstudioseqdesc_t() = default;
	mstudioseqdesc_t(const mstudioseqdesc_t&) = delete;
};


struct mstudioposeparamdesc_t
{
	int					NameIndex;
	inline char* const	GetName() const { return ((char*)this) + NameIndex; }
	int					Flags;	// ????
	float				Start;	// starting value
	float				End;	// ending value
	float				Loop;	// looping range, 0 for no looping, 360 for rotations, etc.
};

struct mstudioflexdesc_t
{
	int					FACSIndex;
	inline char* const	GetFACS() const { return ((char*)this) + FACSIndex; }
};



struct mstudioflexcontroller_t
{
	int					TypeIndex;
	inline char* const	GetType() const { return ((char*)this) + TypeIndex; }
	int					NameIndex;
	inline char* const	GetName() const { return ((char*)this) + NameIndex; }
	mutable int			LocalToGlobal;	// remapped at load time to master list
	float				Min;
	float				Max;
};


enum class FlexControllerRemapType : unsigned char
{
	PASSTHRU = 0,
	TWOWAY,	// Control 0 -> ramps from 1-0 from 0->0.5. Control 1 -> ramps from 0-1 from 0.5->1
	NWAY,	// StepSize = 1 / (control count-1) Control n -> ramps from 0-1-0 from (n-1)*StepSize to n*StepSize to (n+1)*StepSize. A second control is needed to specify amount to use 
	EYELID
};


struct mstudioflexcontrollerui_t
{
	int					Nameindex;
	inline char* const GetName() const { return ((char*)this) + Nameindex; }

	// These are used like a union to save space
	// Here are the possible configurations for a UI controller
	//
	// SIMPLE NON-STEREO:	0: control	1: unused	2: unused
	// STEREO:				0: left		1: right	2: unused
	// NWAY NON-STEREO:		0: control	1: unused	2: value
	// NWAY STEREO:			0: left		1: right	2: value

	int					Index0;
	int					Index1;
	int					Index2;

	inline const mstudioflexcontroller_t* GetController() const
	{
		return !Stereo ? (mstudioflexcontroller_t*)((char*)this + Index0) : NULL;
	}
	inline char* const	GetControllerName() const { return !Stereo ? GetController()->GetName() : NULL; }

	inline const mstudioflexcontroller_t* GetLeftController() const
	{
		return Stereo ? (mstudioflexcontroller_t*)((char*)this + Index0) : NULL;
	}
	inline char* const	GetLeftName() const { return Stereo ? GetLeftController()->GetName() : NULL; }

	inline const mstudioflexcontroller_t* pRightController() const
	{
		return Stereo ? (mstudioflexcontroller_t*)((char*)this + Index1) : NULL;
	}
	inline char* const	GetRightName() const { return Stereo ? pRightController()->GetName() : NULL; }

	inline const mstudioflexcontroller_t* GetNWayValueController() const
	{
		return remaptype == FlexControllerRemapType::NWAY ? (mstudioflexcontroller_t*)((char*)this + Index2) : NULL;
	}
	inline char* const	GetNWayValueName() const { return remaptype == FlexControllerRemapType::NWAY ? GetNWayValueController()->GetName() : NULL; }

	// Number of controllers this ui description contains, 1, 2 or 3
	inline int			GetCount() const { return (Stereo ? 2 : 1) + (remaptype == FlexControllerRemapType::NWAY ? 1 : 0); }

	FlexControllerRemapType	remaptype;	// See the FlexControllerRemapType_t enum
	bool					Stereo;		// Is this a stereo control?
	unsigned char			unused[2];
};


enum class StudioVertAnimType
{
	Normal = 0,
	Wrinkle,
};


struct mstudioflexop_t
{
	int		Op;
	union
	{
		int		Index;
		float	Value;
	} Val;
};

struct mstudioflexrule_t
{
	int					Flex;
	int					NumOps;
	int					OpIndex;
	inline mstudioflexop_t* GetFlexOp(int i) const { return  (mstudioflexop_t*)(((unsigned char*)this) + OpIndex) + i; };
};

// 16 unsigned chars
struct mstudioboneweight_t
{
	float	Weight[MAX_NUM_BONES_PER_VERT];
	char	Bone[MAX_NUM_BONES_PER_VERT];
	unsigned char	NumBones;

	//	unsigned char	material;
	//	short	firstref;
	//	short	lastref;
};

// NOTE: This is exactly 48 unsigned chars
struct mstudiovertex_t
{
	mstudioboneweight_t	BoneWeights;
	Vector				Position;
	Vector				Normal;
	Vector2D			TexCoord;

	mstudiovertex_t() = default;
	mstudiovertex_t(const mstudiovertex_t&) = delete;
};

// skin info
struct mstudiotexture_t
{
	int						Nameindex;
	inline char* const		GetName() const { return ((char*)this) + Nameindex; }
	int						Flags;
	int						Used;
	int						unused1;
	mutable IMaterial*		Material;  // fixme: this needs to go away . .isn't used by the engine, but is used by studiomdl
	mutable void*			ClientMaterial;	// gary, replace with client material pointer if used

	int						unused[10];
};

// eyeball
struct mstudioeyeball_t
{
	int					Nameindex;
	inline char* const GetName() const { return ((char*)this) + Nameindex; }
	int		Bone;
	Vector	Org;
	float	ZOffset;
	float	Radius;
	Vector	Up;
	Vector	Forward;
	int		Texture;

	int		unused1;
	float	Iris_Scale;
	int		unused2;

	int		UpperFlexDesc[3];	// index of raiser, neutral, and lowerer flexdesc that is set by flex controllers
	int		LowerFlexDesc[3];
	float	UpperTarget[3];		// angle (radians) of raised, neutral, and lowered lid positions
	float	LowerTarget[3];

	int		UpperLidFlexDesc;	// index of flex desc that actual lid flexes look to
	int		LowerLidFlexDesc;
	int		unused[4];			// These were used before, so not guaranteed to be 0
	bool	NonFACS;			// Never used before version 44
	char	unused3[3];
	int		unused4[7];

	mstudioeyeball_t() = default;
	mstudioeyeball_t(const mstudioeyeball_t&) = delete;
};


// ikinfo
struct mstudioiklink_t
{
	int		Bone;
	Vector	KneeDir;	// ideal bending direction (per link, if applicable)
	Vector	unused0;	// unused

	mstudioiklink_t() = default;
	mstudioiklink_t(const mstudioiklink_t&) = delete;
};

struct mstudioikchain_t
{
	int				Nameindex;
	inline char* const GetName() const { return ((char*)this) + Nameindex; }
	int				LinkType;
	int				NumLinks;
	int				LinkIndex;
	inline mstudioiklink_t* GetLink(int i) const { return (mstudioiklink_t*)(((unsigned char*)this) + LinkIndex) + i; };
	// FIXME: add unused entries
};


struct mstudioiface_t
{
	unsigned short a, b, c;		// Indices to vertices
};


struct mstudiomdl_t;

struct mstudio_ModelVertexData_t
{
	Vector* Position(int i) const;
	Vector* Normal(int i) const;
	Vector4D* TangentS(int i) const;
	Vector2D* Texcoord(int i) const;
	mstudioboneweight_t* BoneWeights(int i) const;
	mstudiovertex_t* Vertex(int i) const;
	bool				HasTangentData() const;
	int					GetGlobalVertexIndex(int i) const;
	int					GetGlobalTangentIndex(int i) const;

	// base of external vertex data stores
	const void* VertexData;
	const void* TangentData;
};

struct mstudio_meshVertexData_t
{
	Vector* Position(int i) const;
	Vector* Normal(int i) const;
	Vector4D* TangentS(int i) const;
	Vector2D* Texcoord(int i) const;
	mstudioboneweight_t* BoneWeights(int i) const;
	mstudiovertex_t* Vertex(int i) const;
	bool				HasTangentData() const;
	int					GetModelVertexIndex(int i) const;
	int					GetGlobalVertexIndex(int i) const;

	// indirection to this mesh's model's vertex data
	const mstudio_ModelVertexData_t* ModelVertexData;

	// used for fixup calcs when culling top level lods
	// expected number of mesh verts at desired lod
	int					NumLODVertexes[MAX_NUM_LODS];
};

struct mstudiomesh_t
{
	int					Material;

	int					ModelIndex;
	mstudiomdl_t*		GetModel() const;

	int					NumVertices;		// number of unique vertices/normals/texcoords
	int					VertexOffset;		// vertex mstudiovertex_t

	int					NumFlexes;			// vertex animation
	int					FlexIndex;

	// special codes for material operations
	int					MaterialType;
	int					MaterialParam;

	// a unique ordinal for this mesh
	int					MeshId;

	Vector				Center;

	mstudio_meshVertexData_t VertexData;

	int					unused[8]; // remove as appropriate

	mstudiomesh_t() = default;
	mstudiomesh_t(const mstudiomesh_t&) = delete;
};

// studio models
struct mstudiomdl_t
{
	inline const char*	GetName() const { return Name; }
	char				Name[64];

	int					Type;

	float				BoundingRadius;

	int					NumMeshes;
	int					MeshIndex;
	inline mstudiomesh_t* GetMesh(int i) const { return (mstudiomesh_t*)(((unsigned char*)this) + MeshIndex) + i; };

	// cache purposes
	int					NumVertices;		// number of unique vertices/normals/texcoords
	int					VertexIndex;		// vertex Vector
	int					TangentsIndex;		// tangents Vector

	int					NumattAchments;
	int					AttachmentIndex;

	int					NumEyeBalls;
	int					EyeballIndex;
	inline  mstudioeyeball_t* pEyeball(int i) { return (mstudioeyeball_t*)(((unsigned char*)this) + EyeballIndex) + i; };

	mstudio_ModelVertexData_t VertexData;

	int					unused[8];		// remove as appropriate
};

inline bool mstudio_ModelVertexData_t::HasTangentData() const
{
	return (TangentData != NULL);
}

inline int mstudio_ModelVertexData_t::GetGlobalVertexIndex(int i) const
{
	mstudiomdl_t* modelptr = (mstudiomdl_t*)((unsigned char*)this - offsetof(mstudiomdl_t, VertexData));
	return (i + (modelptr->VertexIndex / sizeof(mstudiovertex_t)));
}

inline int mstudio_ModelVertexData_t::GetGlobalTangentIndex(int i) const
{
	mstudiomdl_t* modelptr = (mstudiomdl_t*)((unsigned char*)this - offsetof(mstudiomdl_t, VertexData));
	return (i + (modelptr->TangentsIndex / sizeof(Vector4D)));
}

inline mstudiovertex_t* mstudio_ModelVertexData_t::Vertex(int i) const
{
	return (mstudiovertex_t*)VertexData + GetGlobalVertexIndex(i);
}

inline Vector* mstudio_ModelVertexData_t::Position(int i) const
{
	return &Vertex(i)->Position;
}

inline Vector* mstudio_ModelVertexData_t::Normal(int i) const
{
	return &Vertex(i)->Normal;
}

inline Vector4D* mstudio_ModelVertexData_t::TangentS(int i) const
{
	// NOTE: The tangents vector is 16-unsigned chars in a separate array
	// because it only exists on the high end, and if I leave it out
	// of the mstudiovertex_t, the vertex is 64-unsigned chars (good for low end)
	return (Vector4D*)VertexData + GetGlobalTangentIndex(i);
}

inline Vector2D* mstudio_ModelVertexData_t::Texcoord(int i) const
{
	return &Vertex(i)->TexCoord;
}

inline mstudioboneweight_t* mstudio_ModelVertexData_t::BoneWeights(int i) const
{
	return &Vertex(i)->BoneWeights;
}

inline mstudiomdl_t* mstudiomesh_t::GetModel() const
{
	return (mstudiomdl_t*)(((unsigned char*)this) + ModelIndex);
}

inline bool mstudio_meshVertexData_t::HasTangentData() const
{
	return ModelVertexData->HasTangentData();
}

inline int mstudio_meshVertexData_t::GetModelVertexIndex(int i) const
{
	mstudiomesh_t* meshptr = (mstudiomesh_t*)((unsigned char*)this - offsetof(mstudiomesh_t, VertexData));
	return meshptr->VertexOffset + i;
}

inline int mstudio_meshVertexData_t::GetGlobalVertexIndex(int i) const
{
	return ModelVertexData->GetGlobalVertexIndex(GetModelVertexIndex(i));
}

inline Vector* mstudio_meshVertexData_t::Position(int i) const
{
	return ModelVertexData->Position(GetModelVertexIndex(i));
};

inline Vector* mstudio_meshVertexData_t::Normal(int i) const
{
	return ModelVertexData->Normal(GetModelVertexIndex(i));
};

inline Vector4D* mstudio_meshVertexData_t::TangentS(int i) const
{
	return ModelVertexData->TangentS(GetModelVertexIndex(i));
}

inline Vector2D* mstudio_meshVertexData_t::Texcoord(int i) const
{
	return ModelVertexData->Texcoord(GetModelVertexIndex(i));
};

inline mstudioboneweight_t* mstudio_meshVertexData_t::BoneWeights(int i) const
{
	return ModelVertexData->BoneWeights(GetModelVertexIndex(i));
};

inline mstudiovertex_t* mstudio_meshVertexData_t::Vertex(int i) const
{
	return ModelVertexData->Vertex(GetModelVertexIndex(i));
}

// a group of studio model data
enum class studiomeshgroupflags
{
	IS_FLEXED = 0x1,
	IS_HWSKINNED = 0x2,
	IS_DELTA_FLEXED = 0x4
};


// ----------------------------------------------------------
// runtime stuff
// ----------------------------------------------------------

struct studiomeshgroup_t
{
	IMesh*			Mesh;
	int				NumStrips;
	int				Flags;		// see studiomeshgroupflags_t
	OptimizedModel::StripHeader_t* StripData;
	unsigned short* GroupIndexToMeshIndex;
	int				NumVertices;
	int*			UniqueTris;	// for performance measurements
	unsigned short* Indices;
	bool			MeshNeedsRestore;
	short			ColorMeshID;
	IMorph*			Morph;

	inline unsigned short MeshIndex(int i) const { return GroupIndexToMeshIndex[Indices[i]]; }
};


// studio model data
struct studiomeshdata_t
{
	int					NumGroup;
	studiomeshgroup_t*	MeshGroup;
};

struct studioloddata_t
{
	// not needed - this is really the same as studiohwdata_t.m_NumStudioMeshes
	//int					m_NumMeshes; 
	studiomeshdata_t*	MeshData; // there are studiohwdata_t.m_NumStudioMeshes of these.
	float				SwitchPoint;
	// one of these for each lod since we can switch to simpler materials on lower lods.
	int					numMaterials;
	IMaterial**			ppMaterials; /* will have studiohdr_t.numtextures elements allocated */
	// hack - this needs to go away.
	int*				MaterialFlags; /* will have studiohdr_t.numtextures elements allocated */

	// For decals on hardware morphing, we must actually do hardware skinning
	// For this to work, we have to hope that the total # of bones used by
	// hw flexed verts is < than the max possible for the dx level we're running under
	int*				HWMorphDecalBoneRemap;
	int					DecalBoneCount;
};

struct studiohwdata_t
{
	int					RootLOD;	// calced and clamped, nonzero for lod culling
	int					NumLODs;
	studioloddata_t*	LODs;
	int					NumStudioMeshes;

	inline float		LODMetric(float unitSphereSize) const { return (unitSphereSize != 0.0f) ? (100.0f / unitSphereSize) : 0.0f; }
	inline int			GetLODForMetric(float lodMetric) const
	{
		if (!NumLODs)
			return 0;

		// shadow lod is specified on the last lod with a negative switch
		// never consider shadow lod as viable candidate
		int numLODs = (LODs[NumLODs - 1].SwitchPoint < 0.0f) ? NumLODs - 1 : NumLODs;

		for (int i = RootLOD; i < numLODs - 1; i++)
		{
			if (LODs[i + 1].SwitchPoint > lodMetric)
				return i;
		}

		return numLODs - 1;
	}
};

// ----------------------------------------------------------
// ----------------------------------------------------------

// body part index
struct mstudiobodyparts_t
{
	int					Nameindex;
	inline char* const	GetName() const { return ((char*)this) + Nameindex; }
	int					NumModels;
	int					Base;
	int					ModelIndex; // index into models array
	inline mstudiomdl_t* GetModel(int i) const { return (mstudiomdl_t*)(((unsigned char*)this) + ModelIndex) + i; };
};


struct mstudiomouth_t
{
	int					Bone;
	Vector				Forward;
	int					FlexDesc;

	mstudiomouth_t() = default;
	mstudiomouth_t(const mstudiomouth_t&) = delete;
};

struct mstudiohitboxset_t
{
	int					Nameindex;
	inline char* const	GetName() const { return ((char*)this) + Nameindex; }
	int					NumHitboxes;
	int					HitboxIndex;
	inline mstudiobbox_t* GetHitbox(int i) const { return (mstudiobbox_t*)(((unsigned char*)this) + HitboxIndex) + i; };
};


//-----------------------------------------------------------------------------
// Src bone transforms are transformations that will convert .dmx or .smd-based animations into .mdl-based animations
// NOTE: The operation you should apply is: pretransform * bone transform * posttransform
//-----------------------------------------------------------------------------
struct mstudiosrcbonetransform_t
{
	int			Nameindex;
	inline const char* GetName() const { return ((char*)this) + Nameindex; }
	Matrix3x4	PreTransform;
	Matrix3x4	PostTransform;
};


// ----------------------------------------------------------
// Purpose: Load time results on model compositing
// ----------------------------------------------------------

class virtualgroup_t
{
public:
	virtualgroup_t() { cache = NULL; };
	// tool dependant.  In engine this is a ModelInfo, in tool it's a direct pointer
	void* cache;

	ValveUtlVector<int> BoneMap;				// maps global bone to local bone
	ValveUtlVector<int> MasterBone;			// maps local bone to global bone
	ValveUtlVector<int> MasterSeq;			// maps local sequence to master sequence
	ValveUtlVector<int> MasterAnim;			// maps local animation to master animation
	ValveUtlVector<int> MasterAttachment;	// maps local attachment to global
	ValveUtlVector<int> MasterPose;			// maps local pose parameter to global
	ValveUtlVector<int> MasterNode;			// maps local transition nodes to global
};

struct virtualsequence_t
{
	int	Flags;
	int Activity;
	int Group;
	int Index;
};

struct virtualgeneric_t
{
	int Group;
	int Index;
};


struct virtualmdl_t
{
	virtualgroup_t* GetAnimGroup(int animation) { return &Group[Anim[animation].Group]; } // Note: user must manage mutex for this
	virtualgroup_t* GetSeqGroup(int sequence)
	{
		return &Group[Seq[sequence].Group];
	} // Note: user must manage mutex for this

	struct
	{
		volatile uint32_t OwnerId;
		int Depth;
	} Lock;

	ValveUtlVector<virtualsequence_t>Seq;
	ValveUtlVector<virtualgeneric_t> Anim;
	ValveUtlVector<virtualgeneric_t> Attachment;
	ValveUtlVector<virtualgeneric_t> Pose;
	ValveUtlVector<virtualgroup_t>	Group;
	ValveUtlVector<virtualgeneric_t> Node;
	ValveUtlVector<virtualgeneric_t> IKLock;
	ValveUtlVector<unsigned short>	AutoplaySequences;
};

// apply sequentially to lod sorted vertex and tangent pools to re-establish mesh order
struct vertexFileFixup_t
{
	int		LOD;				// used to skip culled root lod
	int		SourceVertexID;		// absolute index from start of vertex/tangent blocks
	int		NumVertexes;
};

// This flag is set if no hitbox information was specified
#define STUDIOHDR_FLAGS_AUTOGENERATED_HITBOX				0x00000001

// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
// models when we change materials.
#define STUDIOHDR_FLAGS_USES_ENV_CUBEMAP					0x00000002

// Use this when there are translucent parts to the model but we're not going to sort it 
#define STUDIOHDR_FLAGS_FORCE_OPAQUE						0x00000004

// Use this when we want to render the opaque parts during the opaque pass
// and the translucent parts during the translucent pass
#define STUDIOHDR_FLAGS_TRANSLUCENT_TWOPASS					0x00000008

// This is set any time the .qc files has $staticprop in it
// Means there's no bones and no transforms
#define STUDIOHDR_FLAGS_STATIC_PROP							0x00000010

// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
// models when we change materials.
#define STUDIOHDR_FLAGS_USES_FB_TEXTURE						0x00000020

// This flag is set by studiomdl.exe if a separate "$shadowlod" entry was present
//  for the .mdl (the shadow lod is the last entry in the lod list if present)
#define STUDIOHDR_FLAGS_HASSHADOWLOD						0x00000040

// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
// models when we change materials.
#define STUDIOHDR_FLAGS_USES_BUMPMAPPING					0x00000080

// NOTE:  This flag is set when we should use the actual materials on the shadow LOD
// instead of overriding them with the default one (necessary for translucent shadows)
#define STUDIOHDR_FLAGS_USE_SHADOWLOD_MATERIALS				0x00000100

// NOTE:  This flag is set when we should use the actual materials on the shadow LOD
// instead of overriding them with the default one (necessary for translucent shadows)
#define STUDIOHDR_FLAGS_OBSOLETE							0x00000200

#define STUDIOHDR_FLAGS_UNUSED								0x00000400

// NOTE:  This flag is set at mdl build time
#define STUDIOHDR_FLAGS_NO_FORCED_FADE						0x00000800

// NOTE:  The npc will lengthen the viseme check to always include two phonemes
#define STUDIOHDR_FLAGS_FORCE_PHONEME_CROSSFADE				0x00001000

// This flag is set when the .qc has $constantdirectionallight in it
// If set, we use constantdirectionallightdot to calculate light intensity
// rather than the normal directional dot product
// only valid if STUDIOHDR_FLAGS_STATIC_PROP is also set
#define STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT		0x00002000

// Flag to mark delta flexes as already converted from disk format to memory format
#define STUDIOHDR_FLAGS_FLEXES_CONVERTED					0x00004000

// Indicates the studiomdl was built in preview mode
#define STUDIOHDR_FLAGS_BUILT_IN_PREVIEW_MODE				0x00008000

// Ambient boost (runtime flag)
#define STUDIOHDR_FLAGS_AMBIENT_BOOST						0x00010000

// Don't cast shadows from this model (useful on first-person models)
#define STUDIOHDR_FLAGS_DO_NOT_CAST_SHADOWS					0x00020000

// alpha textures should cast shadows in vrad on this model (ONLY prop_static!)
#define STUDIOHDR_FLAGS_CAST_TEXTURE_SHADOWS				0x00040000


// flagged on load to indicate no animation events on this model
#define STUDIOHDR_FLAGS_VERT_ANIM_FIXED_POINT_SCALE			0x00200000

// NOTE! Next time we up the .mdl file format, remove studiohdr2_t
// and insert all fields in this structure into studiohdr_t.
struct studiohdr2_t
{
	// NOTE: For forward compat, make sure any methods in this struct
	// are also available in studiohdr_t so no leaf code ever directly references
	// a studiohdr2_t structure
	int NumSrcBoneTransform;
	int SrcBoneTransformIndex;

	int	IllumPositionAttachmentIndex;
	inline int			GetIllumPositionAttachmentIndex() const { return IllumPositionAttachmentIndex; }

	float MaxEyeDeflection;
	inline float		GetMaxEyeDeflection() const { return MaxEyeDeflection != 0.0f ? MaxEyeDeflection : 0.866f; } // default to cos(30) if not set

	int LinearBoneIndex;
	inline mstudiolinearbone_t* GetLinearBones() const { return (LinearBoneIndex) ? (mstudiolinearbone_t*)(((unsigned char*)this) + LinearBoneIndex) : NULL; }

	int NameIndex;
	inline char* GetName() { return (NameIndex) ? (char*)(((unsigned char*)this) + NameIndex) : NULL; }

	int BoneFlexDriverCount;
	int BoneFlexDriverIndex;
	inline mstudioboneflexdriver_t* GetBoneFlexDriver(int i) const { return (mstudioboneflexdriver_t*)(((unsigned char*)this) + BoneFlexDriverIndex) + i; }

	int reserved[56];
};

struct studiohdr_t
{
	int					Id;
	int					Version;

	int					Checksum;		// this has to be the same in the phy and vtx files to load!

	inline const char*	GetName() const { if (StudioHDR2index && GetStudioHdr2()->GetName()) return GetStudioHdr2()->GetName(); else return Name; }
	char				Name[64];
	int					Length;


	Vector				EyePosition;	// ideal eye position

	Vector				IllumPosition;	// illumination center

	Vector				Hull_Min;		// ideal movement hull size
	Vector				Hull_Max;

	Vector				View_BMin;		// clipping bounding box
	Vector				View_BMax;

	int					Flags;

	int					NumBones;			// bones
	int					BoneIndex;
	inline mstudiobone_t* GetBone(int i) const { return (mstudiobone_t*)(((unsigned char*)this) + BoneIndex) + i; };

	int					NumBoneControllers;		// bone controllers
	int					BoneControllerIndex;
	inline mstudiobonecontroller_t* pBonecontroller(int i) const { return (mstudiobonecontroller_t*)(((unsigned char*)this) + BoneControllerIndex) + i; };

	int					NumHitboxSets;
	int					HitboxSetIndex;

	// Look up hitbox set by index
	mstudiohitboxset_t* GetHitboxSet(int i) const
	{
		return (mstudiohitboxset_t*)(((unsigned char*)this) + HitboxSetIndex) + i;
	};

	// Calls through to hitbox to determine size of specified set
	inline mstudiobbox_t* GetHitbox(int i, int set) const
	{
		mstudiohitboxset_t const* s = GetHitboxSet(set);
		if (!s)
			return NULL;

		return s->GetHitbox(i);
	};

	// Calls through to set to get hitbox count for set
	inline int			GetHitboxCount(int set) const
	{
		mstudiohitboxset_t const* s = GetHitboxSet(set);
		if (!s)
			return 0;

		return s->NumHitboxes;
	};

	// file local animations? and sequences
//private:
	int					NumLocalAnim;			// animations/poses
	int					LocalAnimIndex;		// animation descriptions
	inline mstudioanimdesc_t* GetLocalAnimdesc(int i) const { return (mstudioanimdesc_t*)(((unsigned char*)this) + LocalAnimIndex) + i; };

	int					NumLocalSeq;				// sequences
	int					LocalSeqIndex;
	inline mstudioseqdesc_t* GetLocalSeqdesc(int i) const { return (mstudioseqdesc_t*)(((unsigned char*)this) + LocalSeqIndex) + i; };

	//public:
//private:
	mutable int			ActivityListVersion;	// initialization flag - have the sequences been indexed?
	mutable int			EventsIndexed;
	
	// raw textures
	int					NumTextures;
	int					TextureIndex;
	inline mstudiotexture_t* GetTexture(int i) const { return (mstudiotexture_t*)(((unsigned char*)this) + TextureIndex) + i; };


	// raw textures search paths
	int					NumCdTextures;
	int					CdTextureIndex;
	inline char*		GetCdtexture(int i) const { return (((char*)this) + *((int*)(((unsigned char*)this) + CdTextureIndex) + i)); };

	// replaceable textures tables
	int					NumSkinRef;
	int					NumSkinFamilies;
	int					SkinIndex;
	inline short*		GetSkinref(int i) const { return (short*)(((unsigned char*)this) + SkinIndex) + i; };

	int					NumBodyparts;
	int					BodyPartIndex;
	inline mstudiobodyparts_t* GetBodypart(int i) const { return (mstudiobodyparts_t*)(((unsigned char*)this) + BodyPartIndex) + i; };

	// queryable attachable points
//private:
	int					NumLocalAttachments;
	int					LocalAttachmentIndex;
	inline mstudioattachment_t* pLocalAttachment(int i) const { return (mstudioattachment_t*)(((unsigned char*)this) + LocalAttachmentIndex) + i; };
	//public:

	// animation node to animation node transition graph
//private:
	int					NumLocalnodes;
	int					LocalNodeIndex;
	int					LocalNodeNameIndex;
	inline char*		GetLocalNodeName(int iNode) const { return (((char*)this) + *((int*)(((unsigned char*)this) + LocalNodeNameIndex) + iNode)); }
	inline unsigned char* GetLocalTransition(int i) const { return (unsigned char*)(((unsigned char*)this) + LocalNodeIndex) + i; };


	int					NumFlexDesc;
	int					FlexDescIndex;
	inline mstudioflexdesc_t* GetFlexdesc(int i) const { return (mstudioflexdesc_t*)(((unsigned char*)this) + FlexDescIndex) + i; };

	int					NumFlexcontrollers;
	int					FlexControllerIndex;
	inline mstudioflexcontroller_t* GetFlexcontroller(int i) const { return (mstudioflexcontroller_t*)(((unsigned char*)this) + FlexControllerIndex) + i; };

	int					NumFlexRules;
	int					FlexRuleIndex;
	inline mstudioflexrule_t* GetFlexRule(int i) const { return (mstudioflexrule_t*)(((unsigned char*)this) + FlexRuleIndex) + i; };

	int					NumikChains;
	int					IKChainIndex;
	inline mstudioikchain_t* GetIKChain(int i) const { return (mstudioikchain_t*)(((unsigned char*)this) + IKChainIndex) + i; };

	int					NumMouths;
	int					MouthIndex;
	inline mstudiomouth_t* GetMouth(int i) const { return (mstudiomouth_t*)(((unsigned char*)this) + MouthIndex) + i; };

	//private:
	int					NumLocalPoseParameters;
	int					LocalPoseParamIndex;
	inline mstudioposeparamdesc_t* GetLocalPoseParameter(int i) const { return (mstudioposeparamdesc_t*)(((unsigned char*)this) + LocalPoseParamIndex) + i; };
	//public:

	int					SurfacePropIndex;
	inline char* const	GetSurfaceProp() const { return ((char*)this) + SurfacePropIndex; }

	// Key values
	int					KeyValueIndex;
	int					KeyValueSize;
	inline const char*	GetKeyValueText() const { return KeyValueSize != 0 ? ((char*)this) + KeyValueIndex : NULL; }

	int					NumLocalIKAutoplayLocks;
	int					LocalIKAutoplayLockIndex;
	inline mstudioiklock_t* GetLocalIKAutoplayLock(int i) const {  return (mstudioiklock_t*)(((unsigned char*)this) + LocalIKAutoplayLockIndex) + i; };

	// The collision model mass that jay wanted
	float				Mass;
	int					Contents;

	// external animations, models, etc.
	int					NumIncludeModels;
	int					IncludeModelIndex;
	inline mstudiomodelgroup_t* GetModelGroup(int i) const { return (mstudiomodelgroup_t*)(((unsigned char*)this) + IncludeModelIndex) + i; };

	// implementation specific back pointer to virtual data
	mutable void*	VirtualModel;

	// for demand loaded animation blocks
	int					AnimBlockNameIndex;
	inline char* const	GetAnimBlockName() const { return ((char*)this) + AnimBlockNameIndex; }
	int					NumAnimBlocks;
	int					AnimBlockIndex;
	inline mstudioanimblock_t* GetStudioAnimBlock(int i) const { return (mstudioanimblock_t*)(((unsigned char*)this) + AnimBlockIndex) + i; };
	mutable void*		AnimBlockModel;

	int					BoneTableBynameIndex;
	inline const unsigned char* GetBoneTableSortedByName() const { return (unsigned char*)this + BoneTableBynameIndex; }

	// used by tools only that don't cache, but persist mdl's peer data
	// engine uses virtualModel to back link to cache pointers
	void* VertexBase;
	void* IndexBase;

	// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
	// this value is used to calculate directional components of lighting 
	// on static props
	unsigned char				ConstDirectionalLightDot;

	// set during load of mdl data to track *desired* lod configuration (not actual)
	// the *actual* clamped root lod is found in studiohwdata
	// this is stored here as a global store to ensure the staged loading matches the rendering
	unsigned char				RootLOD;

	// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
	// to be set as root LOD:
	//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
	//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
	unsigned char				NumAllowedRootLODs;

	unsigned char				unused[1];

	int					unused4; // zero out if version < 47

	int					NumFlexControllerUI;
	int					FlexControllerUIIndex;
	mstudioflexcontrollerui_t* pFlexControllerUI(int i) const { return (mstudioflexcontrollerui_t*)(((unsigned char*)this) + FlexControllerUIIndex) + i; }

	float				VertAnimFixedPointScale;
	inline float		GetVertAnimFixedPointScale() const { return (Flags & STUDIOHDR_FLAGS_VERT_ANIM_FIXED_POINT_SCALE) ? VertAnimFixedPointScale : 1.0f / 4096.0f; }

	int					unused3[1];

	// FIXME: Remove when we up the model version. Move all fields of studiohdr2_t into studiohdr_t.
	int					StudioHDR2index;
	studiohdr2_t*		GetStudioHdr2() const { return (studiohdr2_t*)(((unsigned char*)this) + StudioHDR2index); }

	// Src bone transforms are transformations that will convert .dmx or .smd-based animations into .mdl-based animations
	int					NumSrcBoneTransforms() const { return StudioHDR2index ? GetStudioHdr2()->NumSrcBoneTransform : 0; }
	const mstudiosrcbonetransform_t* SrcBoneTransform(int i) const { return (mstudiosrcbonetransform_t*)(((unsigned char*)this) + GetStudioHdr2()->SrcBoneTransformIndex) + i; }

	inline int			IllumPositionAttachmentIndex() const { return StudioHDR2index ? GetStudioHdr2()->GetIllumPositionAttachmentIndex() : 0; }

	inline float		MaxEyeDeflection() const { return StudioHDR2index ? GetStudioHdr2()->GetMaxEyeDeflection() : 0.866f; } // default to cos(30) if not set

	inline mstudiolinearbone_t* pLinearBones() const { return StudioHDR2index ? GetStudioHdr2()->GetLinearBones() : NULL; }

	inline int			BoneFlexDriverCount() const { return StudioHDR2index ? GetStudioHdr2()->BoneFlexDriverCount : 0; }
	inline const mstudioboneflexdriver_t* BoneFlexDriver(int i) const { return StudioHDR2index ? GetStudioHdr2()->GetBoneFlexDriver(i) : NULL; }

	// NOTE: No room to add stuff? Up the .mdl file format version 
	// [and move all fields in studiohdr2_t into studiohdr_t and kill studiohdr2_t],
	// or add your stuff to studiohdr2_t. See NumSrcBoneTransforms/SrcBoneTransform for the pattern to use.
	int					unused2[1];

	studiohdr_t() = default;
	studiohdr_t(const studiohdr_t&) = delete;

	friend struct virtualmdl_t;
};

class IDataCache;
class IMDLCache;


#define STUDIO_CONST	1	// get float
#define STUDIO_FETCH1	2	// get Flexcontroller value
#define STUDIO_FETCH2	3	// get flex weight
#define STUDIO_ADD		4
#define STUDIO_SUB		5
#define STUDIO_MUL		6
#define STUDIO_DIV		7
#define STUDIO_NEG		8	// not implemented
#define STUDIO_EXP		9	// not implemented
#define STUDIO_OPEN		10	// only used in token parsing
#define STUDIO_CLOSE	11
#define STUDIO_COMMA	12	// only used in token parsing
#define STUDIO_MAX		13
#define STUDIO_MIN		14
#define STUDIO_2WAY_0	15	// Fetch a value from a 2 Way slider for the 1st value RemapVal( 0.0, 0.5, 0.0, 1.0 )
#define STUDIO_2WAY_1	16	// Fetch a value from a 2 Way slider for the 2nd value RemapVal( 0.5, 1.0, 0.0, 1.0 )
#define STUDIO_NWAY		17	// Fetch a value from a 2 Way slider for the 2nd value RemapVal( 0.5, 1.0, 0.0, 1.0 )
#define STUDIO_COMBO	18	// Perform a combo operation (essentially multiply the last N values on the stack)
#define STUDIO_DOMINATE	19	// Performs a combination domination operation
#define STUDIO_DME_LOWER_EYELID 20	// 
#define STUDIO_DME_UPPER_EYELID 21	// 

// motion flags
#define STUDIO_X		0x00000001
#define STUDIO_Y		0x00000002	
#define STUDIO_Z		0x00000004
#define STUDIO_XR		0x00000008
#define STUDIO_YR		0x00000010
#define STUDIO_ZR		0x00000020

#define STUDIO_LX		0x00000040
#define STUDIO_LY		0x00000080
#define STUDIO_LZ		0x00000100
#define STUDIO_LXR		0x00000200
#define STUDIO_LYR		0x00000400
#define STUDIO_LZR		0x00000800

#define STUDIO_LINEAR	0x00001000

#define STUDIO_TYPES	0x0003FFFF
#define STUDIO_RLOOP	0x00040000	// controller that wraps shortest distance

// sequence and autolayer flags
#define STUDIO_LOOPING	0x0001		// ending frame should be the same as the starting frame
#define STUDIO_SNAP		0x0002		// do not interpolate between previous animation and this one
#define STUDIO_DELTA	0x0004		// this sequence "adds" to the base sequences, not slerp blends
#define STUDIO_AUTOPLAY	0x0008		// temporary flag that forces the sequence to always play
#define STUDIO_POST		0x0010		// 
#define STUDIO_ALLZEROS	0x0020		// this animation/sequence has no real animation data
//						0x0040
#define STUDIO_CYCLEPOSE 0x0080		// cycle index is taken from a pose parameter index
#define STUDIO_REALTIME	0x0100		// cycle index is taken from a real-time clock, not the animations cycle index
#define STUDIO_LOCAL	0x0200		// sequence has a local context sequence
#define STUDIO_HIDDEN	0x0400		// don't show in default selection views
#define STUDIO_OVERRIDE	0x0800		// a forward declared sequence (empty)
#define STUDIO_ACTIVITY	0x1000		// Has been updated at runtime to activity index
#define STUDIO_EVENT	0x2000		// Has been updated at runtime to event index
#define STUDIO_WORLD	0x4000		// sequence blends in worldspace
// autolayer flags
//							0x0001
//							0x0002
//							0x0004
//							0x0008
#define STUDIO_AL_POST		0x0010		// 
//							0x0020
#define STUDIO_AL_SPLINE	0x0040		// convert layer ramp in/out curve is a spline instead of linear
#define STUDIO_AL_XFADE		0x0080		// pre-bias the ramp curve to compense for a non-1 weight, assuming a second layer is also going to accumulate
//							0x0100
#define STUDIO_AL_NOBLEND	0x0200		// animation always blends at 1.0 (ignores weight)
//							0x0400
//							0x0800
#define STUDIO_AL_LOCAL		0x1000		// layer is a local context sequence
//							0x2000
#define STUDIO_AL_POSE		0x4000		// layer blends using a pose parameter instead of parent cycle
