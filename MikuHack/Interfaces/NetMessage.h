#pragma once

#include <inetmessage.h>
#include <inetmsghandler.h>

#include <bitbuf.h>
#include <utlvector.h>
#include <inetchannel.h>
#include <inetmessage.h>
#include <inetmsghandler.h>
#include <KeyValues.h>
#include <checksum_crc.h>


enum UserMsg_t
{
	Geiger,
	Train,
	HudText,
	SayText,
	SayText2,
	TextMsg,
	ResetHUD,
	GameTitle,
	ItemPickup,
	ShowMenu,
	Shake,
	Fade,
	VGUIMenu,
	Rumble,
	CloseCaption,
	SendAudio,
	VoiceMask,
	RequestState,
	Damage,
	HintText,
	KeyHintText,
	HudMsg,
	AmmoDenied,
	AchievementEvent,
	UpdateRadar,
	VoiceSubtitle,
	HudNotify,
	HudNotifyCustom,
	PlayerStatsUpdate,
	MapStatsUpdate,
	PlayerIgnited,
	PlayerIgnitedInv,
	HudArenaNotify,
	UpdateAchievement,
	TrainingMsg,
	TrainingObjective,
	DamageDodged,
	PlayerJarated,
	PlayerExtinguished,
	PlayerJaratedFade,
	PlayerShieldBlocked,
	BreakModel,
	CheapBreakModel,
	BreakModel_Pumpkin,
	BreakModelRocketDud,
	CallVoteFailed,
	VoteStart,
	VotePass,
	VoteFailed,
	VoteSetup,
	PlayerBonusPoints,
	RDTeamPointsChanged,
	SpawnFlyingBird,
	PlayerGodRayEffect,
	PlayerTeleportHomeEffect,
	MVMStatsReset,
	MVMPlayerEvent,
	MVMResetPlayerStats,
	MVMWaveFailed,
	MVMAnnouncement,
	MVMPlayerUpgradedEvent,
	MVMVictory,
	MVMWaveChange,
	MVMLocalPlayerUpgradesClear,
	MVMLocalPlayerUpgradesValue,
	MVMResetPlayerWaveSpendingStats,
	MVMLocalPlayerWaveSpendingValue,
	MVMResetPlayerUpgradeSpending,
	MVMServerKickTimeUpdate,
	PlayerLoadoutUpdated,
	PlayerTauntSoundLoopStart,
	PlayerTauntSoundLoopEnd,
	ForcePlayerViewAngles,
	BonusDucks,
	EOTLDuckEvent,
	PlayerPickupWeapon,
	QuestObjectiveCompleted,
	SPHapWeapEvent,
	HapDmg,
	HapPunch,
	HapSetDrag,
	HapSetConst,
	HapMeleeContact
};

typedef enum
{
	NA_NULL = 0,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
} netadrtype_t;

struct netadr_s
{
	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned short	port;
};

typedef struct netpacket_s
{
	netadr_t		from;		// sender IP
	int				source;		// received source 
	double			received;	// received time
	unsigned char* data;		// pointer to raw packet data
	bf_read			message;	// easy bitbuf data access
	int				size;		// size in bytes
	int				wiresize;   // size in bytes before decompression
	bool			stream;		// was send as stream
	struct netpacket_s* pNext;	// for internal use, should be NULL in public
} netpacket_t;

#define MAX_CUSTOM_FILES		4
#define MAX_PLAYER_NAME_LENGTH		32
#define	MAX_OSPATH		260

#define	net_NOP 		0			// nop command used for padding
#define net_Disconnect	1			// disconnect, last message in connection
#define net_File		2			// file transmission message request/deny

#define net_Tick		3			// send last world tick
#define net_StringCmd	4			// a string command
#define net_SetConVar	5			// sends one/multiple convar settings
#define	net_SignonState	6			// signals current signon state

//
// server to client
//

#define	svc_Print			7		// print text to console
#define	svc_ServerInfo		8		// first message from server about game, map etc
#define svc_SendTable		9		// sends a sendtable description for a game class
#define svc_ClassInfo		10		// Info about classes (first byte is a CLASSINFO_ define).							
#define	svc_SetPause		11		// tells client if server paused or unpaused


#define	svc_CreateStringTable	12	// inits shared string tables
#define	svc_UpdateStringTable	13	// updates a string table

#define svc_VoiceInit		14		// inits used voice codecs & quality
#define svc_VoiceData		15		// Voicestream data from the server

// #define svc_HLTV			16		// HLTV control messages

#define	svc_Sounds			17		// starts playing sound

#define	svc_SetView			18		// sets entity as point of view
#define	svc_FixAngle		19		// sets/corrects players viewangle
#define	svc_CrosshairAngle	20		// adjusts crosshair in auto aim mode to lock on traget

#define	svc_BSPDecal		21		// add a static decal to the worl BSP

// Message from server side to client side entity
#define svc_UserMessage		23	// a game specific message 
#define svc_EntityMessage	24	// a message for an entity
#define	svc_GameEvent		25	// global game event fired

#define	svc_PacketEntities	26  // non-delta compressed entities

#define	svc_TempEntities	27	// non-reliable event object

#define svc_Prefetch		28	// only sound indices for now

#define svc_Menu			29	// display a menu from a plugin

#define svc_GameEventList	30	// list of known games events and fields

#define svc_GetCvarValue	31	// Server wants to know the value of a cvar on the client

#define svc_CmdKeyValues	32	// Server submits KeyValues command for the client
#define svc_SetPauseTimed	33	// Timed pause - to avoid breaking demos

#define SVC_LASTMSG			33	// last known server messages

//
// client to server
//

#define clc_ClientInfo			8		// client info (table CRC etc)
#define	clc_Move				9		// [CUserCmd]
#define clc_VoiceData			10      // Voicestream data from a client
#define clc_BaselineAck			11		// client acknowledges a new baseline seqnr
#define clc_ListenEvents		12		// client acknowledges a new baseline seqnr
#define clc_RespondCvarValue	13		// client is responding to a svc_GetCvarValue message.
#define clc_FileCRCCheck		14		// client is sending a file's CRC to the server to be verified.
#define clc_SaveReplay			15		// client is sending a save replay request to the server.
#define clc_CmdKeyValues		16
#define clc_FileMD5Check		17		// client is sending a file's MD5 to the server to be verified.

#define CLC_LASTMSG			17		//	last known client message

#define RES_FATALIFMISSING	(1<<0)   // Disconnect if we can't get this file.
#define RES_PRELOAD			(1<<1)  // Load on client rather than just reserving name

#define SIGNONSTATE_NONE		0	// no state yet, about to connect
#define SIGNONSTATE_CHALLENGE	1	// client challenging server, all OOB packets
#define SIGNONSTATE_CONNECTED	2	// client is connected to server, netchans ready
#define SIGNONSTATE_NEW			3	// just got serverinfo and string tables
#define SIGNONSTATE_PRESPAWN	4	// received signon buffers
#define SIGNONSTATE_SPAWN		5	// ready to receive entity packets
#define SIGNONSTATE_FULL		6	// we are fully connected, first non-delta packet received
#define SIGNONSTATE_CHANGELEVEL	7	// server is changing level, please wait

//
// matchmaking
//

#define mm_Heartbeat		16		// send a mm_Heartbeat
#define mm_ClientInfo		17		// information about a player
#define mm_JoinResponse		18		// response to a matchmaking join request
#define mm_RegisterResponse	19		// response to a matchmaking join request
#define mm_Migrate			20		// tell a client to migrate
#define mm_Mutelist			21		// send mutelist info to other clients
#define mm_Checkpoint		22		// game state checkpoints (start, connect, etc)

#define MM_LASTMSG			22		// last known matchmaking message

#define NETMSG_TYPE_BITS 6

class SendTable;
class KeyValue;
class KeyValues;
class INetMessageHandler;
class IServerMessageHandler;
class IClientMessageHandler;

typedef int QueryCvarCookie_t;
typedef enum
{
	eQueryCvarValueStatus_ValueIntact = 0, // It got the value fine.
	eQueryCvarValueStatus_CvarNotFound = 1,
	eQueryCvarValueStatus_NotACvar = 2, // There's a ConCommand, but it's not a ConVar.
	eQueryCvarValueStatus_CvarProtected = 3  // The cvar was marked with FCVAR_SERVER_CAN_NOT_QUERY, so the server
											 // is not allowed to have its value.
} EQueryCvarValueStatus;


#define DECLARE_BASE_MESSAGE( msgtype )						\
	public:													\
		bool			ReadFromBuffer( bf_read &buffer );	\
		bool			WriteToBuffer( bf_write &buffer );	\
		const char		*ToString() const;					\
		int				GetType() const { return msgtype; } \
		const char		*GetName() const { return #msgtype;}\

#define DECLARE_NET_MESSAGE( name )			\
	DECLARE_BASE_MESSAGE( net_##name );		\
	INetMessageHandler *m_pMessageHandler;	\
	bool Process() { return false; }\

#define DECLARE_SVC_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( svc_##name );	\
	IServerMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

#define DECLARE_CLC_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( clc_##name );	\
	IClientMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

#define DECLARE_MM_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( mm_##name );	\
	IMatchmakingMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

class CNetMessage : public INetMessage
{
public:
	CNetMessage() {
		m_bReliable = true;
		m_NetChannel = NULL;
	}

	INetChannel* GetNetChannel() const { return m_NetChannel; }

	virtual ~CNetMessage() {};

	virtual void SetNetChannel(INetChannel* netchan) { m_NetChannel = netchan; }
	virtual void SetReliable(bool state) { m_bReliable = state; };

	virtual bool Process() { return false; }; // no handler set

	virtual bool ReadFromBuffer(bf_read& buffer) { return false; };
	virtual bool WriteToBuffer(bf_write& buffer) { return false; };

	virtual bool IsReliable() const { return m_bReliable; };

	virtual int GetType(void) const { return 0; };
	virtual int GetGroup() const { return INetChannelInfo::GENERIC; }

	virtual const char* GetName(void) const { return ""; };

protected:
	bool			m_bReliable;	// true if message should be send reliable
	INetChannel*	m_NetChannel;	// netchannel this message is from/for
};

class NET_NOP : public CNetMessage
{
	DECLARE_NET_MESSAGE(NOP);

	int GetGroup() const
	{
		return INetChannelInfo::GENERIC;
	}
	NET_NOP() {};
};



///////////////////////////////////////////////////////////////////////////////////////
// bidirectional net messages:
///////////////////////////////////////////////////////////////////////////////////////

class CLC_RespondCvarValue : public CNetMessage
{
	byte			___pad[4];
public:
	DECLARE_CLC_MESSAGE(RespondCvarValue);

	QueryCvarCookie_t m_iCookie;

	const char* m_szCvarName;
	const char* m_szCvarValue; // The sender sets this, and it automatically
							   // points it at m_szCvarNameBuffer when
							   // receiving.

	EQueryCvarValueStatus m_eStatusCode;

private:
	char m_szCvarNameBuffer[256];
	char m_szCvarValueBuffer[256];
};

class NET_SetConVar : public CNetMessage
{
	DECLARE_NET_MESSAGE(SetConVar);

	int	GetGroup() const { return INetChannelInfo::STRINGCMD; }

	NET_SetConVar() {}
	NET_SetConVar(const char* name, const char* value)
	{
		cvar_t localCvar;
		Q_strncpy(localCvar.name, name, MAX_OSPATH);
		Q_strncpy(localCvar.value, value, MAX_OSPATH);
	}

public:

	typedef struct cvar_s
	{
		char	name[MAX_OSPATH];
		char	value[MAX_OSPATH];
	} cvar_t;
	cvar_t convar;
};

class NET_StringCmd : public CNetMessage
{
	DECLARE_NET_MESSAGE(StringCmd);

	int	GetGroup() const { return INetChannelInfo::STRINGCMD; }

	NET_StringCmd() { m_szCommand = NULL; };
	NET_StringCmd(const char* cmd) { m_szCommand = cmd; };

public:
	const char* m_szCommand;	// execute this command

private:
	char		m_szCommandBuffer[1024];	// buffer for received messages

};

class NET_Tick : public CNetMessage
{
	DECLARE_NET_MESSAGE(Tick);

	NET_Tick()
	{
		m_bReliable = false;
#if PROTOCOL_VERSION > 10
		m_flHostFrameTime = 0;
		m_flHostFrameTimeStdDeviation = 0;
#endif
	};

	NET_Tick(int tick, float hostFrametime, float hostFrametime_stddeviation)
	{
		m_bReliable = false;
		m_nTick = tick;
#if PROTOCOL_VERSION > 10
		m_flHostFrameTime = hostFrametime;
		m_flHostFrameTimeStdDeviation = hostFrametime_stddeviation;
#else
		NOTE_UNUSED(hostFrametime);
		NOTE_UNUSED(hostFrametime_stddeviation);
#endif
	};

public:
	int			m_nTick;
#if PROTOCOL_VERSION > 10
	float		m_flHostFrameTime;
	float		m_flHostFrameTimeStdDeviation;
#endif
};

class NET_SignonState : public CNetMessage
{
	DECLARE_NET_MESSAGE(SignonState);

	int	GetGroup() const { return INetChannelInfo::SIGNON; }

	NET_SignonState() {};
	NET_SignonState(int state, int spawncount) { m_nSignonState = state; m_nSpawnCount = spawncount; };

public:
	int			m_nSignonState;			// See SIGNONSTATE_ defines
	int			m_nSpawnCount;			// server spawn count (session number)
};


///////////////////////////////////////////////////////////////////////////////////////
// Client messages:
///////////////////////////////////////////////////////////////////////////////////////

class CLC_ClientInfo : public CNetMessage
{
	DECLARE_CLC_MESSAGE(ClientInfo);

public:
	CRC32_t			m_nSendTableCRC;
	int				m_nServerCount;
	bool			m_bIsHLTV;
//#if defined( REPLAY_ENABLED )
	bool			m_bIsReplay;
//#endif
	uint32			m_nFriendsID;
	char			m_FriendsName[MAX_PLAYER_NAME_LENGTH];
	CRC32_t			m_nCustomFiles[MAX_CUSTOM_FILES];
};



class CLC_Move : public CNetMessage
{
	DECLARE_CLC_MESSAGE(Move);

	int	GetGroup() const { return INetChannelInfo::MOVE; }

	CLC_Move() { m_bReliable = false; }

public:
	int				m_nBackupCommands;
	int				m_nNewCommands;
	int				m_nLength;
	bf_read			m_DataIn;
	bf_write		m_DataOut;
};

class CLC_VoiceData : public CNetMessage
{
	DECLARE_CLC_MESSAGE(VoiceData);

	int	GetGroup() const { return INetChannelInfo::VOICE; }

	CLC_VoiceData() { m_bReliable = false; };

public:
	int				m_nLength;
	bf_read			m_DataIn;
	bf_write		m_DataOut;
	uint64			m_xuid;
};

class CLC_BaselineAck : public CNetMessage
{
	DECLARE_CLC_MESSAGE(BaselineAck);

	CLC_BaselineAck() {};
	CLC_BaselineAck(int tick, int baseline) { m_nBaselineTick = tick; m_nBaselineNr = baseline; }

	int	GetGroup() const { return INetChannelInfo::ENTITIES; }

public:
	int		m_nBaselineTick;	// sequence number of baseline
	int		m_nBaselineNr;		// 0 or 1 		
};

class CLC_CmdKeyValues : public CNetMessage
{
public:
	DECLARE_CLC_MESSAGE(CmdKeyValues);
	KeyValues* m_pKeyValues;
public:
	explicit CLC_CmdKeyValues(KeyValues* pKeyValues = NULL) { m_bReliable = false; }
};

class SVC_CmdKeyValues : public CNetMessage
{
public:
	DECLARE_SVC_MESSAGE(CmdKeyValues);

	KeyValues* m_pKeyValues;
public:
	explicit SVC_CmdKeyValues(KeyValues* pKeyValues = NULL);	// takes ownership
};

///////////////////////////////////////////////////////////////////////////////////////
// server messages:
///////////////////////////////////////////////////////////////////////////////////////



class SVC_Print : public CNetMessage
{
	DECLARE_SVC_MESSAGE(Print);

	SVC_Print() { m_bReliable = false; m_szText = NULL; };

	SVC_Print(const char* text) { m_bReliable = false; m_szText = text; };

public:
	const char* m_szText;	// show this text

private:
	char		m_szTextBuffer[2048];	// buffer for received messages
};

class SVC_SendTable : public CNetMessage
{
	DECLARE_SVC_MESSAGE(SendTable);

	int	GetGroup() const { return INetChannelInfo::SIGNON; }

public:
	bool			m_bNeedsDecoder;
	int				m_nLength;
	bf_read			m_DataIn;
	bf_write		m_DataOut;
};

class SVC_ClassInfo : public CNetMessage
{
	DECLARE_SVC_MESSAGE(ClassInfo);

	int	GetGroup() const { return INetChannelInfo::SIGNON; }

	SVC_ClassInfo() {};
	SVC_ClassInfo(bool createFromSendTables, int numClasses)
	{
		m_bCreateOnClient = createFromSendTables;
		m_nNumServerClasses = numClasses;
	};

public:

	typedef struct class_s
	{
		int		classID;
		char	datatablename[256];
		char	classname[256];
	} class_t;

	bool					m_bCreateOnClient;	// if true, client creates own SendTables & classinfos from game.dll
	CUtlVector<class_t>		m_Classes;
	int						m_nNumServerClasses;
};


class CNetworkStringTable;

class SVC_CreateStringTable : public CNetMessage
{
	DECLARE_SVC_MESSAGE(CreateStringTable);

	int	GetGroup() const { return INetChannelInfo::SIGNON; }

public:

	SVC_CreateStringTable();

public:

	const char* m_szTableName;
	int			m_nMaxEntries;
	int			m_nNumEntries;
	bool		m_bUserDataFixedSize;
	int			m_nUserDataSize;
	int			m_nUserDataSizeBits;
	bool		m_bIsFilenames;
	int			m_nLength;
	bf_read		m_DataIn;
	bf_write	m_DataOut;
	bool		m_bDataCompressed;

private:
	char		m_szTableNameBuffer[256];
};

class SVC_UpdateStringTable : public CNetMessage
{
	DECLARE_SVC_MESSAGE(UpdateStringTable);

	int	GetGroup() const { return INetChannelInfo::STRINGTABLE; }

public:
	int				m_nTableID;	// table to be updated
	int				m_nChangedEntries; // number of how many entries has changed
	int				m_nLength;	// data length in bits
	bf_read			m_DataIn;
	bf_write		m_DataOut;
};

// SVC_VoiceInit
//   v2 - 2017/02/07
//     - Can detect v2 packets by nLegacyQuality == 255 and presence of additional nSampleRate field.
//     - Added nSampleRate field. Previously, nSampleRate was hard-coded per codec type. ::ReadFromBuffer does a
//       one-time conversion of these old types (which can no longer change)
//     - Marked quality field as deprecated. This was already being ignored. v2 clients send 255
//     - Prior to this the sv_use_steam_voice convar was used to switch to steam voice. With this, we properly set
//       szVoiceCodec to "steam".  See ::ReadFromBuffer for shim to fallback to the convar for old streams.
//     - We no longer pass "svc_voiceinit NULL" as szVoiceCodec if it is not selected, just the empty string.  Nothing
//       used this that I could find.
class SVC_VoiceInit : public CNetMessage
{
	DECLARE_SVC_MESSAGE(VoiceInit);

	int	GetGroup() const { return INetChannelInfo::SIGNON; }

	SVC_VoiceInit()
		: m_nSampleRate(0)
	{
		V_memset(m_szVoiceCodec, 0, sizeof(m_szVoiceCodec));
	}

	SVC_VoiceInit(const char* codec, int nSampleRate)
		: m_nSampleRate(nSampleRate)
	{
		V_strncpy(m_szVoiceCodec, codec ? codec : "", sizeof(m_szVoiceCodec));
	}


public:
	// Used voice codec for voice_init.
	//
	// This used to be a DLL name, then became a whitelisted list of codecs.
	char		m_szVoiceCodec[MAX_OSPATH];

	// DEPRECATED:
	//
	// This field used to be a custom quality setting, but it was not honored for a long time: codecs use their own
	// pre-configured quality settings. We never sent anything besides 5, which was then ignored for some codecs.
	//
	// New clients always set 255 here, old clients probably send 5. This could be re-purposed in the future, but beware
	// that very old demos may have non-5 values. It would take more archaeology to determine how to properly interpret
	// those packets -- they're probably using settings we simply don't support any longer.
	//
	// int m_nQuality;

	// The sample rate we are using
	int			m_nSampleRate;
};

class SVC_VoiceData : public CNetMessage
{
	DECLARE_SVC_MESSAGE(VoiceData);

	int	GetGroup() const { return INetChannelInfo::VOICE; }

	SVC_VoiceData() { m_bReliable = false; }

public:
	int				m_nFromClient;	// client who has spoken
	bool			m_bProximity;
	int				m_nLength;		// data length in bits
	uint64			m_xuid;			// X360 player ID

	bf_read			m_DataIn;
	void* m_DataOut;
};

class SVC_Sounds : public CNetMessage
{
	DECLARE_SVC_MESSAGE(Sounds);

	int	GetGroup() const { return INetChannelInfo::SOUNDS; }

public:

	bool		m_bReliableSound;
	int			m_nNumSounds;
	int			m_nLength;
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_Prefetch : public CNetMessage
{
	DECLARE_SVC_MESSAGE(Prefetch);

	int	GetGroup() const { return INetChannelInfo::SOUNDS; }

	enum
	{
		SOUND = 0,
	};

public:

	unsigned short	m_fType;
	unsigned short	m_nSoundIndex;
};

class SVC_SetView : public CNetMessage
{
	DECLARE_SVC_MESSAGE(SetView);

	SVC_SetView() {}
	SVC_SetView(int entity) { m_nEntityIndex = entity; }

public:
	int				m_nEntityIndex;

};

class SVC_FixAngle : public CNetMessage
{
	DECLARE_SVC_MESSAGE(FixAngle);

	SVC_FixAngle() { m_bReliable = false; };
	SVC_FixAngle(bool bRelative, QAngle angle)
	{
		m_bReliable = false; m_bRelative = bRelative; m_Angle = angle;
	}

public:
	bool			m_bRelative;
	QAngle			m_Angle;
};

class SVC_CrosshairAngle : public CNetMessage
{
	DECLARE_SVC_MESSAGE(CrosshairAngle);

	SVC_CrosshairAngle() {}
	SVC_CrosshairAngle(QAngle angle) { m_Angle = angle; }

public:
	QAngle			m_Angle;
};

class SVC_BSPDecal : public CNetMessage
{
	DECLARE_SVC_MESSAGE(BSPDecal);

public:
	Vector		m_Pos;
	int			m_nDecalTextureIndex;
	int			m_nEntityIndex;
	int			m_nModelIndex;
	bool		m_bLowPriority;
};

class SVC_GameEvent : public CNetMessage
{
	DECLARE_SVC_MESSAGE(GameEvent);

	int	GetGroup() const { return INetChannelInfo::EVENTS; }

public:
	int			m_nLength;	// data length in bits
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_UserMessage : public CNetMessage
{
	DECLARE_SVC_MESSAGE(UserMessage);

	SVC_UserMessage() { m_bReliable = false; }

	int	GetGroup() const { return INetChannelInfo::USERMESSAGES; }

public:
	int			m_nMsgType;
	int			m_nLength;	// data length in bits
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_EntityMessage : public CNetMessage
{
	DECLARE_SVC_MESSAGE(EntityMessage);

	SVC_EntityMessage() { m_bReliable = false; }

	int	GetGroup() const { return INetChannelInfo::ENTMESSAGES; }

public:
	int			m_nEntityIndex;
	int			m_nClassID;
	int			m_nLength;	// data length in bits
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_PacketEntities : public CNetMessage
{
	DECLARE_SVC_MESSAGE(PacketEntities);

	int	GetGroup() const { return INetChannelInfo::ENTITIES; }

public:

	int			m_nMaxEntries;
	int			m_nUpdatedEntries;
	bool		m_bIsDelta;
	bool		m_bUpdateBaseline;
	int			m_nBaseline;
	int			m_nDeltaFrom;
	int			m_nLength;
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_TempEntities : public CNetMessage
{
	DECLARE_SVC_MESSAGE(TempEntities);

	SVC_TempEntities() { m_bReliable = false; }

	int	GetGroup() const { return INetChannelInfo::EVENTS; }

	int			m_nNumEntries;
	int			m_nLength;
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_GameEventList : public CNetMessage
{
public:
	DECLARE_SVC_MESSAGE(GameEventList);

	int			m_nNumEvents;
	int			m_nLength;
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};