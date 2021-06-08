#pragma once

#include "NetChannel.hpp"
#include "BitBuffers.hpp"
#include "Const.hpp"
#include "CheckSum.hpp"
#include "UtlVector.hpp"


enum class UserMsg
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



struct NetPacket
{
	NetAddress		From;		// sender IP
	int				Source;		// received source 
	double			Received;	// received time
	unsigned char*	Data;		// pointer to raw packet data
	bf_read			Message;	// easy bitbuf data access
	int				Size;		// size in bytes
	int				WireSize;   // size in bytes before decompression
	bool			Stream;		// was send as stream
	NetPacket*		NextPacket;	// for internal use, should be NULL in public
};


enum class SignonStateType
{
	None,		// no state yet, about to connect
	Challenge,	// client challenging server, all OOB packets
	Connected,	// client is connected to server, netchans ready
	New,		// just got serverinfo and string tables
	PreSpawn,	// received signon buffers
	Spawn,		// ready to receive entity packets
	Full,		// we are fully connected, first non-delta packet received
	ChangeLevel	// server is changing level, please wait
};


enum class NetMsgType
{
	Nop,
	Disconnect,
	File,

	Tick,
	StringCmd,
	SetConVar,
	SignonState,

	ClientInfo = 8,
	Move,
	VoiceData,
	BaseLineAck,
	ListenEvents,
	QueryCVarValue,
	FileCRCCheck,
	SaveReplay,
	CmdKeyValues,
	FileMD5Check,

	LastMsg = FileMD5Check,
	Count
};


enum class QueryCvarValueStatus
{
	ValueIntact = 0,	// It got the value fine.
	CvarNotFound = 1,
	NotACvar = 2,		// There's a ConCommand, but it's not a ConVar.
	CvarProtected = 3	// The cvar was marked with FCVAR_SERVER_CAN_NOT_QUERY, so the server is not allowed to have its value.
};

#define MAX_CUSTOM_FILES		4


class SendTable;
class KeyValues;
class INetMessageHandler;

using QueryCVarCookie = int;

#define DECLARE_NET_MESSAGE(TYPE, GROUP, NAME) \
	public: \
		INetMessageHandler* MsgHandler; \
		INetChannel* GetNetChannel()				const final { return NetChnl; } \
		void SetNetChannel(INetChannel* netchan)		  final { NetChnl = netchan; } \
		void SetReliable(bool state)					  final { Reliable = state; } \
		bool IsReliable()							const final { return Reliable; }  \
		bool			ReadFromBuffer( bf_read &buffer ) final; \
		bool			WriteToBuffer( bf_write &buffer ) final; \
		const char*		ToString()					const final { return ""; } \
		NetMsgType		GetType()					const final { return NetMsgType::TYPE; } \
		const char*		GetName()					const final { return NAME; } \
		NetMsgGroup		GetGroup()					const final { return GROUP; } \
		bool			Process()						  final { return MsgHandler->Process##TYPE(this); }



class INetMessage
{
public:
	virtual	~INetMessage() = default;

	// Use these to setup who can hear whose voice.
	// Pass in client indices (which are their ent indices - 1).

	virtual void	SetNetChannel(INetChannel* netchan) abstract; // netchannel this message is from/for
	virtual void	SetReliable(bool state)			abstract;	// set to true if it's a reliable message

	virtual bool	Process()						abstract; // calles the recently set handler to process this message

	virtual	bool	ReadFromBuffer(bf_read& buffer) abstract; // returns true if parsing was OK
	virtual	bool	WriteToBuffer(bf_write& buffer) abstract;	// returns true if writing was OK

	virtual bool	IsReliable()			const abstract;  // true, if message needs reliable handling

	virtual NetMsgType		GetType()		const abstract; // returns module specific header tag eg svc_serverinfo
	virtual NetMsgGroup		GetGroup()		const abstract;	// returns net message group of this message
	virtual const char*		GetName()		const abstract;	// returns network message name, eg "svc_serverinfo"
	virtual INetChannel*	GetNetChannel() const abstract;
	virtual const char*		ToString()		const abstract; // returns a human readable string about message content

	INetMessage() = default;
	INetMessage(const INetMessage&) = default;	INetMessage& operator=(const INetMessage&) = default;
	INetMessage(INetMessage&&) = default;		INetMessage& operator=(INetMessage&&) = default;


	bool			Reliable{ true };
	INetChannel*	NetChnl{ };
};


class NET_SetConVar: public INetMessage
{
	DECLARE_NET_MESSAGE(SetConVar, NetMsgGroup::StringCmd, "net_SetConVar");

	NET_SetConVar() = default;
	NET_SetConVar(const char* name, const char* value)
	{
		insert(name, value);
	}

	void insert(const char* name, const char* value)
	{
		ConVars.AddToTail({ name, value });
	}

public:
	struct cvar_t
	{
		char	Name[_MAX_PATH];
		char	Value[_MAX_PATH];

		cvar_t() = default;
		cvar_t(const char* name, const char* value)
		{
			sprintf_s(Name, "%s", name);
			sprintf_s(Value, "%s", value);
		}
	};

	ValveUtlVector<cvar_t> ConVars;
};


class NET_StringCmd : public INetMessage
{
	DECLARE_NET_MESSAGE(StringCmd, NetMsgGroup::StringCmd, "net_StringCmd");

	NET_StringCmd() = default;
	NET_StringCmd(const char* cmd) noexcept : Command(cmd) { };

public:
	const char* Command{ };	// execute this command

private:
	char		CommandBuffer[1024];	// buffer for received messages
};


class NET_Tick : public INetMessage
{
	DECLARE_NET_MESSAGE(Tick, NetMsgGroup::Generic, "net_Tick");

	NET_Tick() noexcept { SetReliable(false); };

	NET_Tick(int tick, float hostFrametime, float hostFrametime_stddeviation) noexcept : Tick(tick), HostFrameTime(hostFrametime), HostFrameTimeStdDeviation(hostFrametime_stddeviation)
	{
		this->Reliable = false;
	};

public:
	int			Tick;
	float		HostFrameTime;
	float		HostFrameTimeStdDeviation;
};


class NET_SignonState : public INetMessage
{
	DECLARE_NET_MESSAGE(SignonState, NetMsgGroup::Signon, "net_SignonState");

	NET_SignonState(SignonStateType state, int spawncount) noexcept : SignonState(state), SpawnCount(spawncount) { };

public:
	SignonStateType	SignonState;
	int				SpawnCount;			// server spawn count (session number)
};



class CLC_ClientInfo : public INetMessage
{
	DECLARE_NET_MESSAGE(ClientInfo, NetMsgGroup::Generic, "clc_ClientInfo");

public:
	CRC32_t		SendTableCRC;
	int			ServerCount;
	bool		IsHLTV;
	bool		IsReplay;
	uint32_t	FriendsID;
	char		FriendsName[MAX_PLAYER_NAME_LENGTH];
	CRC32_t		CustomFiles[MAX_CUSTOM_FILES];
};


class CLC_RespondCvarValue : public INetMessage
{
	int8_t			PAD__[4];
public:
	DECLARE_NET_MESSAGE(QueryCVarValue, NetMsgGroup::Generic, "clc_RespondCvarValue");

	QueryCVarCookie Cookie;

	const char* CvarName;
	const char* CvarValue; // The sender sets this, and it automatically
							   // points it at m_szCvarNameBuffer when
							   // receiving.

	QueryCvarValueStatus StatusCode;

private:
	char m_szCvarNameBuffer[256];
	char m_szCvarValueBuffer[256];
};


class CLC_Move : public INetMessage
{
	DECLARE_NET_MESSAGE(Move, NetMsgGroup::Move, "clc_Move");

	CLC_Move()  noexcept { this->Reliable = false; }

public:
	int				BackupCommands;
	int				NewCommands;
	int				Length;
	bf_read			DataIn;
	bf_write		DataOut;
};


class CLC_VoiceData : public INetMessage
{
	DECLARE_NET_MESSAGE(VoiceData, NetMsgGroup::Voice, "clc_VoiceData");

	CLC_VoiceData() noexcept { this->Reliable = false; }

public:
	int			Length;
	bf_read		DataIn;
	bf_write	DataOut;
	uint64_t	ID;
};


class CLC_BaseLineAck : public INetMessage
{
	DECLARE_NET_MESSAGE(BaseLineAck, NetMsgGroup::Entities, "clc_BaselineAck");

	CLC_BaseLineAck(int tick, int baseline) noexcept : BaselineTick(tick), BaselineNr(baseline) { };

public:
	int		BaselineTick;	// sequence number of baseline
	int		BaselineNr;		// 0 or 1 		
};


class CLC_CmdKeyValues : public INetMessage
{
	DECLARE_NET_MESSAGE(CmdKeyValues, NetMsgGroup::Generic, "clc_CmdKeyValues");
	CLC_CmdKeyValues(KeyValues* pKeyValues = nullptr)  noexcept : KV(pKeyValues) { this->Reliable = false; }

public:
	KeyValues* KV;
};


class CLC_FileCRCCheck : public INetMessage
{
	DECLARE_NET_MESSAGE(FileCRCCheck, NetMsgGroup::Generic, "clc_FileCRCCheck");

public:
	char		PathID[_MAX_PATH];
	char		Filename[_MAX_PATH];
	MD5Value	MD5;
	CRC32_t		CRCIOs;
	int			FileHashType;
	int			FileLen;
	int			PackFileNumber;
	int			PackFileID;
	int			FileFraction;
};


class CLC_FileMD5Check : public INetMessage
{
	DECLARE_NET_MESSAGE(FileMD5Check, NetMsgGroup::Generic, "clc_FileMD5Check");

public:
	char		PathID[_MAX_PATH];
	char		Filename[_MAX_PATH];
	MD5Value	MD5;
};
