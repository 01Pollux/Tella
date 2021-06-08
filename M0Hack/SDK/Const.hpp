#pragma once

#include "Helper/EnumClass.hpp"
#include "GlobalVars.hpp"


// the command line param that tells the engine to use steam
#define STEAM_PARM					"-steam"
// the command line param to tell dedicated server to restart 
// if they are out of date
#define AUTO_RESTART "-autoupdate"


// the message a server sends when a clients steam login is expired
#define INVALID_STEAM_TICKET "Invalid STEAM UserID Ticket\n"
#define INVALID_STEAM_LOGON "No Steam logon\n"
#define INVALID_STEAM_VACBANSTATE "VAC banned from secure server\n"
#define INVALID_STEAM_LOGGED_IN_ELSEWHERE "This Steam account is being used in another location\n"


// This is the default, see shareddefs.h for mod-specific value, which can override this
#define DEFAULT_TICK_INTERVAL	(0.015)				// 15 msec is the default
#define MINIMUM_TICK_INTERVAL   (0.001)
#define MAXIMUM_TICK_INTERVAL	(0.1)


// This is the max # of players the engine can handle
#define ABSOLUTE_PLAYER_LIMIT 255  // not 256, so we can send the limit as a byte 
#define ABSOLUTE_PLAYER_LIMIT_DW	( (ABSOLUTE_PLAYER_LIMIT/32) + 1 )


// a player name may have 31 chars + 0 on the PC.
// the 360 only allows 15 char + 0, but stick with the larger PC size for cross-platform communication
#define MAX_PLAYER_NAME_LENGTH		32

#define MAX_PLAYERS 34

#define MAX_WEAPON_SLOTS		6
#define MAX_WEAPON_POSITIONS	20
#define MAX_ITEM_TYPES			6
#define MAX_WEAPONS				48

#define MAX_PLAYERS_PER_CLIENT		1	// One player per PC


#define MAX_MAP_NAME				32	
#define	MAX_NETWORKID_LENGTH		64  // num chars for a network (i.e steam) ID


// BUGBUG: Reconcile with or derive this from the engine's internal definition!
// FIXME: I added an extra bit because I needed to make it signed
#define SP_MODEL_INDEX_BITS			11


// How many bits to use to encode an edict.
#define	MAX_EDICT_BITS				11			// # of bits needed to represent max edicts
// Max # of edicts in a level
#define	MAX_EDICTS					(1<<MAX_EDICT_BITS)


// How many bits to use to encode an server class index
#define MAX_SERVER_CLASS_BITS		9
// Max # of networkable server classes
#define MAX_SERVER_CLASSES			(1<<MAX_SERVER_CLASS_BITS)


#define SIGNED_GUID_LEN 32 // Hashed CD Key (32 hex alphabetic chars + 0 terminator )


// Used for networking ehandles.
#define NUM_ENT_ENTRY_BITS		(MAX_EDICT_BITS + 1)
#define NUM_ENT_ENTRIES			(1 << NUM_ENT_ENTRY_BITS)
#define ENT_ENTRY_MASK			(NUM_ENT_ENTRIES - 1)
#define INVALID_EHANDLE_INDEX	0xFFFFFFFF


#define NUM_SERIAL_NUM_BITS		(32 - NUM_ENT_ENTRY_BITS)


// Networked ehandles use less bits to encode the serial number.
#define NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS	10
#define NUM_NETWORKED_EHANDLE_BITS					(MAX_EDICT_BITS + NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS)
#define INVALID_NETWORKED_EHANDLE_VALUE				((1 << NUM_NETWORKED_EHANDLE_BITS) - 1)


// This is the maximum amount of data a PackedEntity can have. Having a limit allows us
// to use static arrays sometimes instead of allocating memory all over the place.
#define MAX_PACKEDENTITY_DATA	(16384)


// This is the maximum number of properties that can be delta'd. Must be evenly divisible by 8.
#define MAX_PACKEDENTITY_PROPS	(4096)


// a client can have up to 4 customization files (logo, sounds, models, txt).
#define MAX_CUSTOM_FILES		4		// max 4 files
#define MAX_CUSTOM_FILE_SIZE	131072	


#define MAX_COORD_INTEGER			(16384)
#define MIN_COORD_INTEGER			(-MAX_COORD_INTEGER)
#define MAX_COORD_FRACTION			(1.0-(1.0/16.0))
#define MIN_COORD_FRACTION			(-1.0+(1.0/16.0))

#define MAX_COORD_FLOAT				(16384.0f)
#define MIN_COORD_FLOAT				(-MAX_COORD_FLOAT)

// Width of the coord system, which is TOO BIG to send as a client/server coordinate value
#define COORD_EXTENT				(2*MAX_COORD_INTEGER)

// Maximum traceable distance ( assumes cubic world and trace from one corner to opposite )
// COORD_EXTENT * sqrt(3)
#define MAX_TRACE_LENGTH			( 1.732050807569 * COORD_EXTENT )		

// This value is the LONGEST possible range (limited by max valid coordinate number, not 2x)
#define MAX_COORD_RANGE				(MAX_COORD_INTEGER)


//
// Constants shared by the engine and dlls
// This header file included by engine files and DLL files.
// Most came from server.h

// CBaseEntity::m_fFlags
// PLAYER SPECIFIC FLAGS FIRST BECAUSE WE USE ONLY A FEW BITS OF NETWORK PRECISION
#define	FL_ONGROUND				(1<<0)	// At rest / on the ground
#define FL_DUCKING				(1<<1)	// Player flag -- Player is fully crouched
#define	FL_WATERJUMP			(1<<3)	// player jumping out of water
#define FL_ONTRAIN				(1<<4) // Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
#define FL_INRAIN				(1<<5)	// Indicates the entity is standing in rain
#define FL_FROZEN				(1<<6) // Player is frozen for 3rd person camera
#define FL_ATCONTROLS			(1<<7) // Player can't move, but keeps key inputs for controlling another entity
#define	FL_CLIENT				(1<<8)	// Is a player
#define FL_FAKECLIENT			(1<<9)	// Fake client, simulated server side; don't send network messages to them
#define	FL_INWATER				(1<<10)	// In water
// NOTE if you move things up, make sure to change this value
#define PLAYER_FLAG_BITS		11

// NON-PLAYER SPECIFIC (i.e., not used by GameMovement or the client .dll ) -- Can still be applied to players, though
#define	FL_FLY					(1<<11)	// Changes the SV_Movestep() behavior to not need to be on ground
#define	FL_SWIM					(1<<12)	// Changes the SV_Movestep() behavior to not need to be on ground (but stay in water)
#define	FL_CONVEYOR				(1<<13)
#define	FL_NPC					(1<<14)
#define	FL_GODMODE				(1<<15)
#define	FL_NOTARGET				(1<<16)
#define	FL_AIMTARGET			(1<<17)	// set if the crosshair needs to aim onto the entity
#define	FL_PARTIALGROUND		(1<<18)	// not all corners are valid
#define FL_STATICPROP			(1<<19)	// Eetsa static prop!		
#define FL_GRAPHED				(1<<20) // worldgraph has this ent listed as something that blocks a connection
#define FL_GRENADE				(1<<21)
#define FL_STEPMOVEMENT			(1<<22)	// Changes the SV_Movestep() behavior to not do any processing
#define FL_DONTTOUCH			(1<<23)	// Doesn't generate touch functions, generates Untouch() for anything it was touching when this flag was set
#define FL_BASEVELOCITY			(1<<24)	// Base velocity has been applied this frame (used to convert base velocity into momentum)
#define FL_WORLDBRUSH			(1<<25)	// Not moveable/removeable brush entity (really part of the world, but represented as an entity for transparency or something)
#define FL_OBJECT				(1<<26) // Terrible name. This is an object that NPCs should see. Missiles, for example.
#define FL_KILLME				(1<<27)	// This entity is marked for death -- will be freed by game DLL
#define FL_ONFIRE				(1<<28)	// You know...
#define FL_DISSOLVING			(1<<29) // We're dissolving!
#define FL_TRANSRAGDOLL			(1<<30) // In the process of turning into a client side ragdoll.
#define FL_UNBLOCKABLE_BY_PLAYER (1<<31) // pusher that can't be blocked by the player



#define DMG_GENERIC                  0          /**< generic damage was done */
#define DMG_CRUSH                   (1 << 0)    /**< crushed by falling or moving object.
													 NOTE: It's assumed crush damage is occurring as a result of physics collision,
													 so no extra physics force is generated by crush damage.
													 DON'T use DMG_CRUSH when damaging entities unless it's the result of a physics
													 collision. You probably want DMG_CLUB instead. */
#define DMG_BULLET                  (1 << 1)    /**< shot */
#define DMG_SLASH                   (1 << 2)    /**< cut, clawed, stabbed */
#define DMG_BURN                    (1 << 3)    /**< heat burned */
#define DMG_VEHICLE                 (1 << 4)    /**< hit by a vehicle */
#define DMG_FALL                    (1 << 5)    /**< fell too far */
#define DMG_BLAST                   (1 << 6)    /**< explosive blast damage */
#define DMG_CLUB                    (1 << 7)    /**< crowbar, punch, headbutt */
#define DMG_SHOCK                   (1 << 8)    /**< electric shock */
#define DMG_SONIC                   (1 << 9)    /**< sound pulse shockwave */
#define DMG_ENERGYBEAM              (1 << 10)   /**< laser or other high energy beam  */
#define DMG_PREVENT_PHYSICS_FORCE   (1 << 11)   /**< Prevent a physics force  */
#define DMG_NEVERGIB                (1 << 12)   /**< with this bit OR'd in, no damage type will be able to gib victims upon death */
#define DMG_ALWAYSGIB               (1 << 13)   /**< with this bit OR'd in, any damage type can be made to gib victims upon death. */
#define DMG_DROWN                   (1 << 14)   /**< Drowning */
#define DMG_PARALYZE                (1 << 15)   /**< slows affected creature down */
#define DMG_NERVEGAS                (1 << 16)   /**< nerve toxins, very bad */
#define DMG_POISON                  (1 << 17)   /**< blood poisoning - heals over time like drowning damage */
#define DMG_RADIATION               (1 << 18)   /**< radiation exposure */
#define DMG_DROWNRECOVER            (1 << 19)   /**< drowning recovery */
#define DMG_ACID                    (1 << 20)   /**< toxic chemicals or acid burns */
#define DMG_SLOWBURN                (1 << 21)   /**< in an oven */
#define DMG_REMOVENORAGDOLL         (1 << 22)   /**< with this bit OR'd in, no ragdoll will be created, and the target will be quietly removed.
													 use this to kill an entity that you've already got a server-side ragdoll for */
#define DMG_PHYSGUN                 (1 << 23)   /**< Hit by manipulator. Usually doesn't do any damage. */
#define DMG_PLASMA                  (1 << 24)   /**< Shot by Cremator */
#define DMG_AIRBOAT                 (1 << 25)   /**< Hit by the airboat's gun */
#define DMG_DISSOLVE                (1 << 26)   /**< Dissolving! */
#define DMG_BLAST_SURFACE           (1 << 27)   /**< A blast on the surface of water that cannot harm things underwater */
#define DMG_DIRECT                  (1 << 28)
#define DMG_BUCKSHOT                (1 << 29)   /**< not quite a bullet. Little, rounder, different. */

#define DMG_CRIT                DMG_ACID        /**< TF2 crits and minicrits */
#define DMG_RADIUS_MAX          DMG_ENERGYBEAM  /**< No damage falloff */
#define DMG_NOCLOSEDISTANCEMOD  DMG_POISON      /**< Don't do damage falloff too close */
#define DMG_HALF_FALLOFF        DMG_RADIATION   /**< 50% damage falloff */
#define DMG_USEDISTANCEMOD      DMG_SLOWBURN    /**< Do damage falloff */
#define DMG_IGNITE              DMG_PLASMA      /**< Ignite victim */
#define DMG_USE_HITLOCATIONS    DMG_AIRBOAT     /**< Do hit location damage (Like the sniperrifle and ambassador) */


enum class EntBuildingObjType
{
	Dispenser,
	Teleporter,
	Sentry,
	Sapper,

	Count
};

enum class EntBuildingObjMode
{
	TeleEntrace,
	TeleExit,

	Count
};

enum class TeleporterState
{
	Building,
	Idle,
	Ready,
	Sending,
	Receiving,
	ReceivingRelease,
	Recharging,
	Upgrading,

	Count
};

enum class SentryGunState
{
	Inactive,
	Searching,
	Attacking,

	Count,
};

enum class DispenserState
{
	Idle,
	Upgrading,

	Count

};

enum class EntDamageType
{
	No,
	EventOnly,
	Yes,
	AIm
};

enum class PlayerObserverMode
{
	None = 0,	// not in spectator mode
	DeathCam,	// special mode for death cam animation
	FreezeCam,	// zooms to a target, and freeze-frames on them
	Fixed,		// view from a fixed camera position
	InEye,	// follow a player in first person view
	Chase,		// follow a player in third person view
	PointOfInterest,		// PASSTIME point of interest - game objective, big fight, anything interesting; added in the middle of the enum due to tons of hard-coded "<ROAMING" enum compares
	FreeCam,	// free roaming

	Count,
};

// edict->movetype values
enum class EntMoveType 
{
	None = 0,		// never moves
	Isometric,		// For players -- in TF2 commander view, etc.
	Walk,			// Player only - moving on the ground
	Step,			// gravity, special edge handling -- monsters use this
	Fly,			// No gravity, but still collides with stuff
	FlyGravity,		// flies through the air + is affected by gravity
	VPhysics,		// uses VPHYSICS for simulation
	Push,			// no clip to world, push and crush
	NoClip,			// No gravity, no collisions, still do velocity/avelocity
	Ladder,			// Used by players only when going onto a ladder
	Observer,		// Observer movement, depends on player's observer mode
	Custom,			// Allows the entity to describe its own physics

	Last = Custom,

	MaxBits = 4
};


// edict->movecollide values
enum class EntMoveCollide 
{
	Default = 0,

	// These ones only work for MOVETYPE_FLY + MOVETYPE_FLYGRAVITY
	FlyBounce,	// bounces, reflects, based on elasticity of surface and object - applies friction (adjust velocity)
	FlyCustom,	// Touch() will modify the velocity however it likes
	FlySlide,   // slides along surfaces (no bounce) - applies friciton (adjusts velocity)

	Count,		// Number of different movecollides

	// When adding new movecollide types, make sure this is correct
	MaxBits = 3
};


// edict->solid values
// NOTE: Some movetypes will cause collisions independent of SOLID_NOT/SOLID_TRIGGER when the entity moves
// SOLID only effects OTHER entities colliding with this one when they move - UGH!

// Solid type basically describes how the bounding volume of the object is represented
// NOTE: SOLID_BBOX MUST BE 2, and SOLID_VPHYSICS MUST BE 6
// NOTE: These numerical values are used in the FGD by the prop code (see prop_dynamic)
enum class EntSolidType
{
	None = 0,	// no solid model
	BSP = 1,	// a BSP tree
	BBOX = 2,	// an AABB
	OBB = 3,	// an OBB (not implemented yet)
	OBB_YAW= 4,	// an OBB, constrained so that it can only yaw
	Custom = 5,	// Always call into the entity for tests
	VPhysics = 6,	// solid vphysics object, get vcollide from the model and collide with that
	Count,
};


enum class EntSolidFlags 
{
	CustomRayTest = 0x0001,	// Ignore solid type + always call into the entity for ray tests
	CustomBoxTest = 0x0002,	// Ignore solid type + always call into the entity for swept box tests
	NotSolid = 0x0004,	// Are we currently not solid?
	Trigger = 0x0008,	// This is something may be collideable but fires touch functions
	// even when it's not collideable (when the FSOLID_NOT_SOLID flag is set)
	NotStandable = 0x0010,	// You can't stand on this
	VolumeContents = 0x0020,	// Contains volumetric contents (like water)
	ForceWorldAlign = 0x0040,	// Forces the collision rep to be world-aligned even if it's SOLID_BSP or SOLID_VPHYSICS
	UseTriggerBounds = 0x0080,	// Uses a special trigger bounds separate from the normal OBB
	RootParentsAlign = 0x0100,	// Collisions are defined in root parent's local coordinate space
	TriggerTouchDebris = 0x0200,	// This trigger will touch debris objects

	MaxBits = 10
};

ECLASS_BITWISE_OPERATORS(EntSolidFlags);


//-----------------------------------------------------------------------------
// A couple of inline helper methods
//-----------------------------------------------------------------------------
inline bool IsSolid(EntSolidType solidType, EntSolidFlags solidFlags)
{
	return (solidType != EntSolidType::None) && (!(solidFlags & EntSolidFlags::NotSolid));
}


// m_lifeState values
#define	LIFE_ALIVE				0 // alive
#define	LIFE_DYING				1 // playing death animation or still falling off of a ledge waiting to hit ground
#define	LIFE_DEAD				2 // dead. lying still.
#define LIFE_RESPAWNABLE		3
#define LIFE_DISCARDBODY		4

// entity effects
enum EntEffects
{
	BoneMerge = 0x001,	// Performs bone merge on client side
	BrightLight = 0x002,	// DLIGHT centered at entity origin
	DimLight = 0x004,	// player flashlight
	NoInterp = 0x008,	// don't interpolate the next frame
	NoShadow = 0x010,	// Don't cast no shadow
	NoDraw = 0x020,	// don't draw entity
	NoReceiveShadow = 0x040,	// Don't receive no shadow
	BoneMerge_FastCull = 0x080,	// For use with EF_BONEMERGE. If this is set, then it places this ent's origin at its
							
	// parent and uses the parent's bbox + the max extents of the aiment.
	// Otherwise, it sets up the parent's bones every frame to figure out where to place
	// the aiment, which is inefficient because it'll setup the parent's bones even if
	// the parent is not in the PVS.
	ItemBlink = 0x100,	// blink an item so that the user notices it.
	ParentAnimate = 0x200,	// always assume that the parent entity is animating
	MaxBits = 10
};

#define EF_PARITY_BITS	3
#define EF_PARITY_MASK  ((1<<EF_PARITY_BITS)-1)

// How many bits does the muzzle flash parity thing get?
#define EF_MUZZLEFLASH_BITS 2

// plats
#define	PLAT_LOW_TRIGGER	1

// Trains
#define	SF_TRAIN_WAIT_RETRIGGER	1
#define SF_TRAIN_PASSABLE		8		// Train is not solid -- used to make water trains

// view angle update types for CPlayerState::fixangle
#define FIXANGLE_NONE			0
#define FIXANGLE_ABSOLUTE		1
#define FIXANGLE_RELATIVE		2

// Break Model Defines

#define BREAK_GLASS		0x01
#define BREAK_METAL		0x02
#define BREAK_FLESH		0x04
#define BREAK_WOOD		0x08

#define BREAK_SMOKE		0x10
#define BREAK_TRANS		0x20
#define BREAK_CONCRETE	0x40

// If this is set, then we share a lighting origin with the last non-slave breakable sent down to the client
#define BREAK_SLAVE		0x80

// Colliding temp entity sounds

#define BOUNCE_GLASS	BREAK_GLASS
#define	BOUNCE_METAL	BREAK_METAL
#define BOUNCE_FLESH	BREAK_FLESH
#define BOUNCE_WOOD		BREAK_WOOD
#define BOUNCE_SHRAP	0x10
#define BOUNCE_SHELL	0x20
#define	BOUNCE_CONCRETE BREAK_CONCRETE
#define BOUNCE_SHOTSHELL 0x80

// Temp entity bounce sound types
#define TE_BOUNCE_NULL		0
#define TE_BOUNCE_SHELL		1
#define TE_BOUNCE_SHOTSHELL	2

// Rendering constants
// if this is changed, update common/MaterialSystem/Sprite.cpp
enum class RenderMode_t
{
	Normal,			// src
	TransColor,		// c*a+dest*(1-a)
	TransTexture,	// src*a+dest*(1-a)
	Glow,			// src*a+dest -- No Z buffer checks -- Fixed size in screen space
	TransAlpha,		// src*srca+dest*(1-srca)
	TransAdd,		// src*a+dest
	Environmental,	// not drawn, used for environmental effects
	TransAddFrameBlend, // use a fractional frame value to blend between animation frames
	TransAlphaAdd,	// src + dest*(1-a)
	WorldGlow,		// Same as kRenderGlow but not fixed size in screen space
	None,			// Don't render.
};

enum class RenderFx_t
{
	None = 0,
	PulseSlow,
	PulseFast,
	PulseSlowWide,
	PulseFastWide,
	FadeSlow,
	FadeFast,
	SolidSlow,
	SolidFast,
	StrobeSlow,
	StrobeFast,
	StrobeFaster,
	FlickerSlow,
	FlickerFast,
	NoDissipation,
	Distort,			// Distort/scale/translate flicker
	Hologram,			// kRenderFxDistort + distance fade
	Explode,			// Scale up really big!
	GlowShell,			// Glowing Shell
	ClampMinScale,		// Keep this sprite from getting very small (SPRITES only!)
	EnvRain,			// for environmental rendermode, make rain
	EnvSnow,			//  "        "            "    , make snow
	Spotlight,			// TEST CODE for experimental spotlight
	Ragdoll,			// HACKHACK: TEST CODE for signalling death of a ragdoll character
	PulseFastWider,

	Count
};

enum class EntCollisionGroup
{
	None = 0,
	Debris,			// Collides with nothing but world and static stuff
	DebrisTirgger, // Same as debris, but hits triggers
	InteractiveDebris,	// Collides with everything except other interactive debris or debris
	Interactive,	// Collides with everything except interactive debris or debris
	Player,
	BreakableGlass,
	Vehicle,
	PlayerMovements,  // For HL2, same as Collision_Group_Player, for
	
	// TF2, this filters out other players and CBaseObjects
	NPC,			// Generic NPC group
	InVehicle,		// for any entity inside a vehicle
	Weapon,			// for any weapons that need collision detection
	VehicleClip,	// vehicle clip brush to restrict vehicle movement
	Projectile,		// Projectiles!
	DoorBlocket,	// Blocks entities not permitted to get near moving doors
	PassableDoor,	// Doors that the player shouldn't collide with
	Dissolving,		// Things that are dissolving are in this group
	PushAway,		// Nonsolid on client and server, pushaway in player code
	NPCActor,		// Used so NPCs in scripts ignore the player.
	NPCScripted,	// USed for NPCs in scripts that should not collide with each other
	
	Count
};

#define SOUND_NORMAL_CLIP_DIST	1000.0f

// How many networked area portals do we allow?
#define MAX_AREA_STATE_BYTES		32
#define MAX_AREA_PORTAL_STATE_BYTES 24

// user message max payload size (note, this value is used by the engine, so MODs cannot change it)
#define MAX_USER_MSG_DATA 255
#define MAX_ENTITY_MSG_DATA 255


struct VColor32
{
	bool operator!=(const VColor32& other) const
	{
		return r != other.r || g != other.g || b != other.b || a != other.a;
	}

	char8_t r{}, g{}, b{}, a{};
};


struct VModeS
{
	int			Width;
	int			Height;
	int			BPP;
	int			RefreshRate;
};

enum class SkyboxVisibility
{
	NotVisible,
	Visible3D,
	Visible2D,
};


enum class ClientFrameStage
{
	Unknown = -1,			// (haven't run any frames yet)
	Begin,

	// A network packet is being recieved
	PreNetUpdate,
	// Data has been received and we're going to start calling PostDataUpdate
	PostDataUpdatePre,
	// Data has been received and we've called PostDataUpdate on all data recipients
	PostDataUpdatePost,
	// We've received all packets, we can now do interpolation, prediction, etc..
	PostNetUpdate,

	// We're about to start rendering the scene
	PreRender,
	// We've finished rendering the scene.
	PostRender
};


inline int TimeToTicks(float time)
{
	return static_cast<int>(0.5f + time / Interfaces::GlobalVars->IntervalPerTick);
}

inline float TicksToTime(int tick)
{
	return Interfaces::GlobalVars->IntervalPerTick * static_cast<float>(tick);
}

inline int RoundToTicks(float time)
{
	return static_cast<int>(Interfaces::GlobalVars->IntervalPerTick * TimeToTicks(time));
}