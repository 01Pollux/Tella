#include "GCSys.hpp"

namespace gcsdk
{
	IMemberFuncThunk<bool, const ProtoMsgBase&> ProtoMsgBase::get_call()
	{
		static IMemberFuncThunk<bool, const ProtoMsgBase&> proto_send(M0Libraries::Client->FindPattern("GCClientSystem::SendMsg [proto base]"));
		return proto_send;

	}
	
	IMemberFuncThunk<bool, const GCMsgBase&> GCMsgBase::get_call()
	{
		static IMemberFuncThunk<bool, const GCMsgBase&> msg_send(M0Libraries::Client->FindPattern("GCClientSystem::SendMsg [msg base]"));
		return msg_send;
	}

	IProtoBufNetPacket::IProtoBufNetPacket(INetPacket* pNetPacket, GCProtoBufMsgSrc eReplyType, const SteamID& steamID, uint32_t nGCDirIndex, ProtoBufMsgType msgType)
		: MsgType(msgType), steamID(steamID), NetPacket(pNetPacket)
	{
		pNetPacket->AddRef();

		Header = std::make_unique<CMsgProtoBufHeader>();

		// Pull the length of the header out of the packet, but then validate it's not longer than the packet itself (ie, corrupt packet)
		const uint32_t unLenProtoBufHeader = fixed_header().cubProtoBufExtHdr;
		if (unLenProtoBufHeader + sizeof(ProtoBufMsgHeader) <= pNetPacket->CubData())
		{
			if(Header->ParseFromArray(pNetPacket->PubData() + sizeof(ProtoBufMsgHeader), unLenProtoBufHeader))
			{
				//if this packet doesn't have a source provided, we need to set the source of this message. If one is already provided, we should never stomp that data as that is the ultimate source of the message
				if (Header->gc_msg_src() == GCProtoBufMsgSrc_Unspecified)
				{
					//make sure to set the steam ID to what steam provided so clients can't spoof it
					Header->set_client_steam_id(steamID.to_uint64());
					//and track our original source type and which GC it was first received by
					Header->set_gc_dir_index_source(nGCDirIndex);
					Header->set_gc_msg_src(eReplyType);
				}

				bIsValid = true;
			}
		}
	}

}