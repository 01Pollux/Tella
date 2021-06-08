#pragma once

#include <stdint.h>

class IBaseEntityInternal;
struct DataMapInputData;

#define INVALID_TIME (FLT_MAX * -1.0)

enum class DataMapFieldType
{
	FIELD_VOID = 0,			// No type or value
	FIELD_FLOAT,			// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_VECTOR,			// Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION,		// A quaternion
	FIELD_INTEGER,			// Any integer or enum
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_COLOR32,			// 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED,			// an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM,			// special type that contains function pointers to it's read/write/parse functions

	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EDICT,			// edict_t *

	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_TICK,				// an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_INPUT,			// a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)

	FIELD_VMATRIX,			// a vmatrix (output coords are NOT worldspace)

	// NOTE: Use float arrays for local transformations that don't need to be fixed up.
	FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)

	FIELD_INTERVAL,			// a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX,		// a model index
	FIELD_MATERIALINDEX,	// a material index (using the material precache string table)

	FIELD_VECTOR2D,			// 2 floats

	FIELD_TYPECOUNT,		// MUST BE LAST
};


#define FTYPEDESC_GLOBAL			0x0001		// This field is masked for global entity save/restore
#define FTYPEDESC_SAVE				0x0002		// This field is saved to disk
#define FTYPEDESC_KEY				0x0004		// This field can be requested and written to by string name at load time
#define FTYPEDESC_INPUT				0x0008		// This field can be written to by string name at run time, and a function called
#define FTYPEDESC_OUTPUT			0x0010		// This field propogates it's value to all targets whenever it changes
#define FTYPEDESC_FUNCTIONTABLE		0x0020		// This is a table entry for a member function pointer
#define FTYPEDESC_PTR				0x0040		// This field is a pointer, not an embedded object
#define FTYPEDESC_OVERRIDE			0x0080		// The field is an override for one in a base class (only used by prediction system for now)

// Flags used by other systems (e.g., prediction system)
#define FTYPEDESC_INSENDTABLE		0x0100		// This field is present in a network SendTable
#define FTYPEDESC_PRIVATE			0x0200		// The field is local to the client or server only (not referenced by prediction code and not replicated by networking)
#define FTYPEDESC_NOERRORCHECK		0x0400		// The field is part of the prediction typedescription, but doesn't get compared when checking for errors

#define FTYPEDESC_MODELINDEX		0x0800		// The field is a model index (used for debugging output)

#define FTYPEDESC_INDEX				0x1000		// The field is an index into file data, used for byteswapping. 

// These flags apply to C_BasePlayer derived objects only
#define FTYPEDESC_VIEW_OTHER_PLAYER		0x2000		// By default you can only view fields on the local player (yourself), 
													//   but if this is set, then we allow you to see fields on other players
#define FTYPEDESC_VIEW_OWN_TEAM			0x4000		// Only show this data if the player is on the same team as the local player
#define FTYPEDESC_VIEW_NEVER			0x8000		// Never show this field to anyone, even the local player (unusual)

#define TD_MSECTOLERANCE		0.001f		// This is a FIELD_FLOAT and should only be checked to be within 0.001 of the networked info

struct typedescription_t;


class ISaveRestoreOps;

//
// Function prototype for all input handlers.
//
typedef void (IBaseEntityInternal::* DataMapInputFunction)(DataMapInputData&);

struct EntityDataMap;
struct typedescription_t;

enum
{
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,

	// Must be last
	TD_OFFSET_COUNT,
};

struct typedescription_t
{
	DataMapFieldType	Type;
	const char*			FieldName;
	int					Offset[TD_OFFSET_COUNT]; // 0 == normal, 1 == packed offset
	unsigned short		Size;
	short				Flags;
	// the name of the variable in the map/fgd data, or the name of the action
	const char*			ExternalName;
	// pointer to the function set for save/restoring of custom data types
	ISaveRestoreOps*	SaveRestoreOps;
	// for associating function with string names
	DataMapInputFunction InputFunction;
	// For embedding additional datatables inside this one
	EntityDataMap*			DataDesc;

	// Stores the actual member variable size in bytes
	int					SizeInBytes;

	// FTYPEDESC_OVERRIDE point to first baseclass instance if chains_validated has occurred
	struct typedescription_t* OverrideField;

	// Used to track exclusion of baseclass fields
	int					OverrideCcount;

	// Tolerance for field errors for float fields
	float				Tolerance;
};


//-----------------------------------------------------------------------------
// Purpose: stores the list of objects in the hierarchy
//			used to iterate through an object's data descriptions
//-----------------------------------------------------------------------------
struct EntityDataMap
{
	typedescription_t*	DataDesc;
	int					DataNumFields;
	char const*			ClassName;
	EntityDataMap*		BaseMap;

	bool				ChainsValidated;
	// Have the "packed" offsets been computed
	bool				PackedOffsetsComputed;
	int					PackedSize;
};


enum class DataMapType : char8_t
{
	DataMap,
	PredictionMap
};

struct CachedDataMapInfo
{
	typedescription_t*	TypeDesc { };
	uint32_t			Offset{ };
};

