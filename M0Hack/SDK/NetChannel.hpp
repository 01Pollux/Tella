#pragma once

#include "UtlVector.hpp"
#include "BitBuffers.hpp"

class INetChannel;

enum class NetMsgFlowType
{
	Outgoing,
	Incoming,
};

enum class NetMsgGroup
{
	Generic = 0,	// must be first and is default group
	LocalPlayer,	// bytes for local player entity update
	OtherPlayers,	// bytes for other players update
	Entities,		// all other entity bytes
	Sounds,			// game sounds
	Events,			// event messages
	UserMsgs,		// user messages
	EntMsgs,		// entity messages
	Voice,			// voice data
	StringTable,	// a stringtable update
	Move,			// client move cmds
	StringCmd,		// string command
	Signon,			// various signondata

	Count,			// must be last and is not a real group
};

enum NetAddressType
{
	Null = 0,
	LoopBack,
	Broadcast,
	IP,
};

struct NetAddress
{
	NetAddressType	Type;
	unsigned char	IP[4];
	unsigned short	Port;
};

class INetChannel_Internal
{
public:
	virtual const char* GetName() const abstract;	// get channel name
	virtual const char* GetAddress() const abstract; // get channel IP address as string
	virtual float		GetTime() const abstract;	// current net time
	virtual float		GetTimeConnected() const abstract;	// get connection time in seconds
	virtual int			GetBufferSize() const abstract;	// netchannel packet history size
	virtual int			GetDataRate() const abstract; // send data rate in byte/sec

	virtual bool		IsLoopback() const abstract;	// true if loopback channel
	virtual bool		IsTimingOut() const abstract;	// true if timing out
	virtual bool		IsPlayback() const abstract;	// true if demo playback

	virtual float		GetLatency(NetMsgFlowType flow) const abstract;	 // current latency (RTT), more accurate but jittering
	virtual float		GetAvgLatency(NetMsgFlowType flow) const abstract; // average packet latency in seconds
	virtual float		GetAvgLoss(NetMsgFlowType flow) const abstract;	 // avg packet loss[0..1]
	virtual float		GetAvgChoke(NetMsgFlowType flow) const abstract;	 // avg packet choke[0..1]
	virtual float		GetAvgData(NetMsgFlowType flow) const abstract;	 // data flow in bytes/sec
	virtual float		GetAvgPackets(NetMsgFlowType flow) const abstract; // avg packets/sec
	virtual int			GetTotalData(NetMsgFlowType flow) const abstract;	 // total flow in/out in bytes
	virtual int			GetSequenceNr(NetMsgFlowType flow) const abstract;	// last send seq number
	virtual bool		IsValidPacket(NetMsgFlowType flow, int frame_number) const abstract; // true if packet was not lost/dropped/chocked/flushed
	virtual float		GetPacketTime(NetMsgFlowType flow, int frame_number) const abstract; // time when packet was send
	virtual int			GetPacketBytes(NetMsgFlowType flow, int frame_number, int group) const abstract; // group size of this packet
	virtual bool		GetStreamProgress(NetMsgFlowType flow, int* received, int* total) const abstract;  // TCP progress if transmitting
	virtual float		GetTimeSinceLastReceived() const abstract;	// get time since last recieved packet in seconds
	virtual	float		GetCommandInterpolationAmount(NetMsgFlowType flow, int frame_number) const abstract;
	virtual void		GetPacketResponseLatency(NetMsgFlowType flow, int frame_number, int* pnLatencyMsecs, int* pnChoke) const abstract;
	virtual void		GetRemoteFramerate(float* pflFrameTime, float* pflFrameTimeStdDeviation) const abstract;

	virtual float		GetTimeoutSeconds() const abstract;
};

class IDemoRecorder;
class INetMessage;
class INetChannelHandler;
class BfWrite;

class INetChannel : public INetChannel_Internal
{
public:
	virtual	~INetChannel() = default;

	virtual void	SetDataRate(float rate) abstract;
	virtual bool	RegisterMessage(INetMessage* msg) abstract;
	virtual bool	StartStreaming(unsigned int challengeNr) abstract;
	virtual void	ResetStreaming() abstract;
	virtual void	SetTimeout(float seconds) abstract;
	virtual void	SetDemoRecorder(IDemoRecorder* recorder) abstract;
	virtual void	SetChallengeNr(unsigned int chnr) abstract;

	virtual void	Reset() abstract;
	virtual void	Clear() abstract;
	virtual void	Shutdown(const char* reason) abstract;

	virtual void	ProcessPlayback() abstract;
	virtual bool	ProcessStream() abstract;
	virtual void	ProcessPacket(struct netpacket_s* packet, bool bHasHeader) abstract;

	virtual bool	SendNetMsg(INetMessage& msg, bool bForceReliable = false, bool bVoice = false) abstract;
	virtual bool	SendData(BfWrite& msg, bool bReliable = true) abstract;
	virtual bool	SendFile(const char* filename, unsigned int transferID) abstract;
	virtual void	DenyFile(const char* filename, unsigned int transferID) abstract;
	virtual void	RequestFile_OLD(const char* filename, unsigned int transferID) abstract;	// get rid of this function when we version the 
	virtual void	SetChoked() abstract;
	virtual int		SendDatagram(BfWrite* data) abstract;
	virtual bool	Transmit(bool onlyReliable = false) abstract;

	virtual const NetAddress& GetRemoteAddress() const abstract;
	virtual INetChannelHandler* GetMsgHandler() const abstract;
	virtual int				GetDropNumber() const abstract;
	virtual int				GetSocket() const abstract;
	virtual unsigned int	GetChallengeNr() const abstract;
	virtual void			GetSequenceData(int& nOutSequenceNr, int& nInSequenceNr, int& nOutSequenceNrAck) abstract;
	virtual void			SetSequenceData(int nOutSequenceNr, int nInSequenceNr, int nOutSequenceNrAck) abstract;

	virtual void	UpdateMessageStats(int msggroup, int bits) abstract;
	virtual bool	CanPacket() const abstract;
	virtual bool	IsOverflowed() const abstract;
	virtual bool	IsTimedOut() const abstract;
	virtual bool	HasPendingReliableData() abstract;

	virtual void	SetFileTransmissionMode(bool bBackgroundMode) abstract;
	virtual void	SetCompressionMode(bool bUseCompression) abstract;
	virtual unsigned int RequestFile(const char* filename) abstract;
	virtual float	GetTimeSinceLastReceived() const abstract;	// get time since last received packet in seconds

	virtual void	SetMaxBufferSize(bool bReliable, int nBytes, bool bVoice = false) abstract;

	virtual bool	IsNull() const abstract;
	virtual int		GetNumBitsWritten(bool bReliable) abstract;
	virtual void	SetInterpolationAmount(float flInterpolationAmount) abstract;
	virtual void	SetRemoteFramerate(float flFrameTime, float flFrameTimeStdDeviation) abstract;

	// Max # of payload bytes before we must split/fragment the packet
	virtual void	SetMaxRoutablePayloadSize(int nSplitSize) abstract;
	virtual int		GetMaxRoutablePayloadSize() abstract;

	virtual int		GetProtocolVersion() abstract;

public:
	struct DataFragmenets
	{
		void*			File;			// open file handle
		char			FileName[_MAX_PATH]; // filename
		char*			Buffer;			// if NULL it's a file
		unsigned int	Bytes;			// size in bytes
		unsigned int	Bits;			// size in bits
		unsigned int	TransferID;		// only for files
		bool			IsCompressed;	// true if data is bzip compressed
		unsigned int	UncompressedSize; // full size in bytes
		bool			asTCP;			// send as TCP stream
		int				NumFragments;	// number of total fragments
		int				AckedFragments; // number of fragments send & acknowledged
		int				PendingFragments; // number of fragments send, but not acknowledged yet
	};

	struct SubChannel
	{
		int	StartFraggment[2];
		int	NumFragments[2];
		int	SendSeqNr;
		int	State; // 0 = free, 1 = scheduled to send, 2 = send & waiting, 3 = dirty
		int	Index; // index in m_SubChannels[]
	};

	struct NetFrame
	{
		// Data received from server
		float			Time;			// net_time received/send
		int				Size;			// total size in bytes
		float			Latency;		// raw ping for this packet, not cleaned. set when acknowledged otherwise -1.
		float			AvgLatency;	// averaged ping for this packet
		bool			Valid;			// false if dropped, lost, flushed
		int				Choked;			// number of previously chocked packets
		int				Dropped;
		float			InterpolationAmount;
		unsigned short	MsgGroups[static_cast<size_t>(NetMsgGroup::Count)];	// received bytes for each message group
	};

	struct NetFlow
	{
		float		NextCompute;	// Time when we should recompute k/sec data
		float		AvgBytespersec;	// average bytes/sec
		float		AvgPacketspersec;// average packets/sec
		float		AvgLoss;		// average packet loss [0..1]
		float		AvgChoke;		// average packet choke [0..1]
		float		AvgLatency;		// average ping, not cleaned
		float		Latency;		// current ping, more accurate also more jittering
		int			TotalPackets;	// total processed packets
		int			TotalBytes;		// total processed bytes
		int			CurrentIndex;	// current frame index
		NetFrame	Frames[64];		// frame history
		NetFrame*	CurrentFrame;	// current frame
	} ;


	bool		ProcessingMessages;
	bool		ClearedDuringProcessing;
	bool		ShouldDelete;

	// last send outgoing sequence number
	int			OutSequenceNr;
	// last received incoming sequnec number
	int			InSequenceNr;
	// last received acknowledge outgoing sequnce number
	int			OutSequenceNrAck;

	// state of outgoing reliable data (0/1) flip flop used for loss detection
	int			OutReliableState;
	// state of incoming reliable data
	int			InReliableState;
	//number of choked packets
	int			ChokedPackets;


	// Reliable data buffer, send which each packet (or put in waiting list)
	bf_write	StreamReliable;
	ValveUtlMemory<uint8_t>	ReliableDataBuffer;

	// unreliable message buffer, cleared which each packet
	bf_write	StreamUnreliable;
	ValveUtlMemory<uint8_t> UnreliableDataBuffer;

	bf_write	StreamVoice;
	ValveUtlMemory<uint8_t> VoiceDataBuffer;

	// don't use any vars below this (only in net_ws.cpp)

	int			Socket;   // NS_SERVER or NS_CLIENT index, depending on channel.
	int			StreamSocket;	// TCP socket handle

	unsigned int MaxReliablePayloadSize;	// max size of reliable payload in a single packet	

	// Address this channel is talking to.
	NetAddress	RemoteAddress;

	// For timeouts.  Time last message was received.
	float		LastReceived;
	// Time when channel was connected.
	double      ConnectTime;

	// Bandwidth choke
	// Bytes per second
	int			Rate;
	// If realtime > cleartime, free to send next packet
	double		ClearTime;

	// waiting list for reliable data and file transfer
	ValveUtlVector<DataFragmenets*>	 WaitingList[2];
	// receive buffers for streams
	DataFragmenets					 ReceiveList[2];
	SubChannel						 SubChannels[8];

	unsigned int	m_FileRequestCounter;		// increasing counter with each file request
	bool			m_bFileBackgroundTranmission;// if true, only send 1 fragment per packet
	bool			m_bUseCompression;			// if true, larger reliable data will be bzip compressed

	// TCP stream state maschine:
	bool					StreamActive;		// true if TCP is active
	int						SteamType;			// STREAM_CMD_*
	int						StreamSeqNr;		// each blob send of TCP as an increasing ID
	int						StreamLength;		// total length of current stream blob
	int						StreamReceived;		// length of already received bytes
	char					SteamFile[_MAX_PATH];// if receiving file, this is it's name
	ValveUtlMemory<uint8_t> StreamData;			// Here goes the stream data (if not file). Only allocated if we're going to use it.

	// packet history
	NetFlow			m_DataFlow[2];
	// total bytes for each message group
	int				m_MsgStats[static_cast<size_t>(NetMsgGroup::Count)];


	// packets lost before getting last update (was global net_drop)
	int				m_PacketDrop;

	// channel name
	char			m_Name[32];

	// unique, random challenge number 
	unsigned int	m_ChallengeNr;

	// in seconds 
	float		m_Timeout;

	// who registers and processes messages
	INetChannelHandler*			 MessageHandler;
	// list of registered message
	ValveUtlVector<INetMessage*> NetMessages;
	// if != NULL points to a recording/playback demo object
	IDemoRecorder*				 DemoRecorder;
	int							 QueuedPackets;

	float						InterpolationAmount;
	float						RemoteFrameTime;
	float						RemoteFrameTimeStdDeviation;
	int							MaxRoutablePayloadSize;

	int							SplitPacketSequence;
	// true if PACKET_FLAG_CHALLENGE was set when receiving packets from the sender
	bool						StreamContainsChallenge;

	int							ProtocolVersion;
};


class INetMessageHandler_Internal
{
public:
	virtual ~INetMessageHandler_Internal() = default;

	virtual bool ProcessTick(void*) abstract;
	virtual bool ProcessStringCmd(void*) abstract;
	virtual bool ProcessSetConVar(void*) abstract;
	virtual bool ProcessSignonState(void*) abstract;
};

class INetMessageHandler : public INetMessageHandler_Internal
{
public:
	virtual bool ProcessClientInfo(void*) abstract;
	virtual bool ProcessMove(void*) abstract;
	virtual bool ProcessVoiceData(void*) abstract;
	virtual bool ProcessBaseLineAck(void*) abstract;
	virtual bool ProcessListenEvents(void*) abstract;
	virtual bool ProcessQueryCVarValue(void*) abstract;
	virtual bool ProcessFileCRCCheck(void*) abstract;
	virtual bool ProcessFileMD5Check(void*) abstract;
	virtual bool ProcessSaveReplay(void*) abstract;
	virtual bool ProcessCmdKeyValues(void*) abstract;
};