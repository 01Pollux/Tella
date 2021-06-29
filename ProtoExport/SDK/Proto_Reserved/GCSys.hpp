#pragma once

#include <Steam/Commons.hpp>
#include <google/protobuf/message.h>

#include "steammessages.pb.h"
#include "Library/Lib.hpp"

#include "SDK/ProtoExport.hpp"

namespace gcsdk
{
	template<typename _Proto, class _Dtor = std::default_delete<_Proto>> using unique_proto = std::unique_ptr<_Proto, _Dtor>;

	using ProtoBufMsgType = uint32_t;

	enum class MsgFormatType
	{
		Struct = 0,
		ClientStruct = 1,
		ClientStructDeprecated = 2,
		ProtocolBuffer = 3
	};

	struct GCMsgHdrEx
	{
		ProtoBufMsgType	eMsg;					// The message type
		SteamID			SteamID;				// User's SteamID
		uint16_t		HdrVersion;
		uint64_t		JobIDTarget;
		uint64_t		JobIDSource;
	};

#pragma pack(push, 1)
	struct ProtoBufMsgHeader
	{
		int32_t			MsgFlagged{ };
		uint32_t		cubProtoBufExtHdr{ };

		ProtoBufMsgHeader() = default;
		ProtoBufMsgHeader(MsgFormatType eMsg, uint32_t cubProtoBufExtHdr) : MsgFlagged(static_cast<int32_t>(eMsg) | 0x80000000), cubProtoBufExtHdr(cubProtoBufExtHdr) {}
		MsgFormatType GetEMsg() const { return static_cast<MsgFormatType>(MsgFlagged & (~0x80000000)); }
	};
#pragma pack(pop)


	class IRefCount
	{
	public:
		int AddRef() { return ++Ref; }
		int Release()
		{
			if (0 == --Ref)
				DestroyThis();
			return Ref;
		}
	protected:
		virtual ~IRefCount() = default;

		virtual void DestroyThis() { delete this; }

		std::atomic_int Ref{ 1 };
	};

	class INetPacket
	{
	public:
		//called to allocate a buffer for the net packet of the specified size. This takes an optional pointer
		//of which it will copy into the data if appropriate
		void Init(uint32_t cubData, const void* pCopyData = nullptr)
		{
			pubData = new uint8_t[cubData];
			cubData = cubData;

			if (pCopyData)
				memcpy(pubData, pCopyData, cubData);

			AddRef();
		}

		//called when working with a net packet that you want to reference a separate buffer
		// data
		uint8_t* PubData() const { return pubData; }
		uint32_t CubData() const { return cubData; }

		// ownership
		void AddRef() { ++Ref; }
		void Release()
		{
			if (!--Ref)
				if (pubData)
					delete[] pubData;
		}

	private:
		int		 Ref{ };
		uint32_t cubData{ };
		uint8_t* pubData{ };
	};

	class IProtoBufNetPacket : public IRefCount
	{
	public:
		IProtoBufNetPacket(INetPacket* pNetPacket, GCProtoBufMsgSrc eReplyType, const SteamID& steamID, uint32_t nGCDirIndex, ProtoBufMsgType msgType);

		virtual MsgFormatType GetEMsgFormatType() const { return MsgFormatType::ProtocolBuffer; }
		virtual INetPacket* GetCNetPacket() const { return NetPacket; }
		virtual uint8_t* PubData() const { return NetPacket->PubData(); }
		virtual uint32_t CubData() const { return NetPacket->CubData(); }

		virtual ProtoBufMsgType GetEMsg() const { return MsgType; }
		virtual uint64_t GetSourceJobID() const { return Header->job_id_source(); }
		virtual uint64_t GetTargetJobID() const { return Header->job_id_target(); }
		virtual void SetTargetJobID(uint64_t ulJobID) { Header->set_job_id_target(ulJobID); }

		virtual SteamID GetSteamID() const { return steamID; }
		virtual void SetSteamID(SteamID steamID) { steamID = steamID; }

		virtual SteamAppId GetSourceAppID() const { return Header->source_app_id(); };
		virtual void		SetSourceAppID(SteamAppId appId) { Header->set_source_app_id(appId); }

		virtual bool		has_job_name() const { return Header->has_target_job_name(); }
		virtual const char*	job_name() const { return Header->target_job_name().c_str(); }

		bool IsValid() const { return bIsValid; }
		ProtoBufMsgHeader&	fixed_header() const { return *reinterpret_cast<ProtoBufMsgHeader*>(PubData()); }
		CMsgProtoBufHeader* proto_header() const { return Header.get(); }

	protected:
		virtual ~IProtoBufNetPacket() = default;

		INetPacket*							NetPacket;
		unique_proto<CMsgProtoBufHeader>	Header;
		SteamID								steamID;
		ProtoBufMsgType						MsgType;
		bool								bIsValid{ };
	};


	class M0PROTO ProtoMsgBase
	{
	public:
		virtual ~ProtoMsgBase()
		{
			if (Packet)
			{
				Packet->Release();
				Packet = nullptr;
			}
		}
		virtual google::protobuf::Message* gbody() const { return Message.get(); }

		template<typename _GCMsg>	_GCMsg& body() noexcept { return *reinterpret_cast<_GCMsg*>(Message); }
		template<typename _GCMsg>	const _GCMsg& body() const noexcept { return *reinterpret_cast<const _GCMsg*>(Message); }

		CMsgProtoBufHeader& hdr()		noexcept { return *Header; }
		const CMsgProtoBufHeader& hdr() const noexcept { return *Header; }

		ProtoBufMsgType msg()			const noexcept { return MsgType & 0x80000000; }
		const SteamID& steam_id()		const noexcept { return Header->client_steam_id(); }

		ProtoMsgBase() = default;
		ProtoMsgBase(ProtoBufMsgType msg) : MsgType(msg), Header(std::make_unique<CMsgProtoBufHeader>()) { }

		static IMemberFuncThunk<bool, const ProtoMsgBase&> get_call();
	private:

		IProtoBufNetPacket* Packet;
		unique_proto<CMsgProtoBufHeader> Header;

		ProtoBufMsgType MsgType{ };

		std::unique_ptr<google::protobuf::Message> Message;
	};


	template <typename _Ty>
	class M0PROTO GCMsgBase_Internal
	{
	public:
		// Send constructor
		GCMsgBase_Internal(size_t cubStruct, size_t cubReserve = 64U) :
			cubStruct(cubStruct), cubMsgHdr(sizeof(_Ty)),
			cubPkt(cubStruct + sizeof(_Ty))
		{
			pubPkt = std::make_unique<uint8_t[]>(cubPkt + cubReserve);
			pubBody = pubPkt.get() + cubMsgHdr;
		}

		virtual ~GCMsgBase_Internal() = default;

		uint8_t*		pubvar()	   noexcept { return (pubPkt.get() + cubMsgHdr + cubStruct); }
		const uint8_t*	pubvar() const noexcept { return (pubPkt.get() + cubMsgHdr + cubStruct); }
		uint32_t		curvar() const noexcept
		{
			return (cubPkt >= (cubMsgHdr + cubStruct)) ? cubPkt - cubMsgHdr - cubStruct : 0;
		}

		uint8_t*		pubpkt()		  noexcept { return pubPkt.get(); }
		const uint8_t*	pubpkt()	const noexcept { return pubPkt.get(); }
		uint32_t		cubpkt()	const noexcept { return cubPkt; }
		_Ty&			hdr()			  noexcept { return *reinterpret_cast<_Ty*>(pubpkt()); }
		const _Ty&		hdr()		const noexcept { return *reinterpret_cast<const _Ty*>(pubpkt()); }
		uint32_t		cubhdr()	const noexcept { return cubMsgHdr; }

		uint8_t*		pubbody() noexcept		 { return pubBody; }
		const uint8_t*	pubbody() const noexcept { return pubBody; }
		const uint32_t	cubbody() const noexcept { return cubpkt() - cubhdr(); }

	protected:
		unique_proto<uint8_t[]> pubPkt;		// Raw packet data
		uint8_t*				pubBody;	// pointer to body; always equal to m_pubPkt + m_cubMsgHdr
		size_t					cubPkt;		// Raw packet size
		const size_t			cubMsgHdr;	// Size of our message header
		size_t					cubStruct;	// Size of our message-specific struct

	private:
		GCMsgBase_Internal(const GCMsgBase_Internal&) = delete;	GCMsgBase_Internal& operator=(const GCMsgBase_Internal&) = delete;
		GCMsgBase_Internal(GCMsgBase_Internal&&) = default;		GCMsgBase_Internal& operator=(GCMsgBase_Internal&&) = default;

		bool		IsAlloced{ };				// Did we allocate this buffer or does someone else own it

		// Receive only
		uint8_t*	pubVarRead{ };			// Our current read pointer in the variable-length data
	};

	class M0PROTO GCMsgBase : public GCMsgBase_Internal<GCMsgHdrEx>
	{
	public:
		GCMsgBase(size_t cubStruct, size_t cubReserve = 64U) :
			GCMsgBase_Internal(cubStruct, cubReserve) { }

		static IMemberFuncThunk<bool, const GCMsgBase&> get_call();
	};
}