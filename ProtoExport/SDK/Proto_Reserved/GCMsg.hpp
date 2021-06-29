#pragma once

#include <atomic>
#include <memory>
#include <algorithm>

#include "GCSys.hpp"

namespace gcsdk
{
	extern int M0PROTO Test_;
	class GCClientSystem
	{
	public:
		GCClientSystem()
		{
			static M0Pointer sys = M0Library("client.dll").FindPattern("GCClientSystem").get();
			GCSystem = sys;
		}

		M0Pointer get() noexcept
		{
			return GCSystem;
		}

		M0Pointer handler() noexcept
		{
			return get() + 0x10;
		}

	private:
		M0Pointer GCSystem;
	};

	template<typename _Ty>
	class M0PROTO ProtoMsg final : public ProtoMsgBase
	{
	public:
		ProtoMsg(ProtoBufMsgType msg) : ProtoMsgBase(msg), ProtoBody(std::make_unique<_Ty>()) { };

		_Ty& body()				noexcept { return *ProtoBody; }
		const _Ty& body() const noexcept { return *ProtoBody; }

		/*
		bool send()
		{
			GCClientSystem gc;
			return get_call()(gc.handler(), *this);
		}
		*/

	private:
		unique_proto<_Ty> ProtoBody;
	};
	template<typename _Ty> using protomsg = ProtoMsg<_Ty>;

	template<typename _Ty>
	class M0PROTO GCMsg final : public GCMsgBase
	{
	public:
		GCMsg(ProtoBufMsgType msg, uint32_t cubReserve = 64) :
			GCMsgBase(sizeof(_Ty), cubReserve)
		{
			hdr().eMsg = msg;
			hdr().HdrVersion = 0x1;
			hdr().JobIDSource = 0xffffffffffffffffull;
			hdr().JobIDTarget = 0xffffffffffffffffull;
		}
		
		_Ty& body()				noexcept { return *reinterpret_cast<_Ty*>(pubpkt() + cubhdr()); }
		const _Ty& body() const noexcept { return *reinterpret_cast<const _Ty*>(pubpkt() + cubhdr()); }
		ProtoMsgBase msg() const noexcept { return hdr().eMsg; }

		bool is_expecting_reply() { return hdr().JobIDSource != 0xffffffffffffffffull; }

		/*
		bool send()
		{
			GCClientSystem gc;
			return get_call()(gc.handler(), *this);
		}
		*/
	};
	template<typename _Ty> using gcmsg = GCMsg<_Ty>;
}
