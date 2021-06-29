#include "Backtrack.hpp"

#include "Hooks/LevelChange.hpp"

namespace BackTrack
{
	static const BackTrack::MainHack* Handler = nullptr;

	DETOUR_CREATE_MEMBER(int, NetChannel_SendDatagram, bf_write* OutData)
	{
		if (!Handler->enabled())
			return DETOUR_MEMBER_CALL(NetChannel_SendDatagram, OutData);

		INetChannel* pNet = DETOUR_GET_MEMBER(INetChannel);
		int out_seq = pNet->OutSequenceNr;
		int in_state = pNet->InReliableState;

		Handler->OnSendDatagram(pNet);

		int ret = DETOUR_MEMBER_CALL(NetChannel_SendDatagram, OutData);
		pNet->OutSequenceNr = out_seq;
		pNet->InReliableState = in_state;

		return ret;
	}


	struct DatagramSeq
	{
		static inline int LastOutSeq = 0;
		static constexpr size_t RestoreCapacity = 2048U;

		int OutSeq;
		int InState;
		float Time;
	};
	
	static std::vector<DatagramSeq> restore_datagrams;
}


void BackTrack::MainHack::OnSendDatagram(INetChannel* pNet) const
{
	double latency = this->latency();
	for (auto iter = restore_datagrams.crbegin(); iter != restore_datagrams.crend(); iter++)
	{
		if (static_cast<double>(Interfaces::GlobalVars->CurTime - iter->Time) >= latency)
		{
			pNet->OutSequenceNr = iter->OutSeq;
			pNet->InReliableState = iter->InState;
			break;
		}
	}
}


void BackTrack::MainHack::UpdateDatagram()
{
	if (INetChannel* pNet = Interfaces::EngineClient->GetNetChannelInfo())
	{
		if (restore_datagrams.size() >= DatagramSeq::RestoreCapacity)
			restore_datagrams.pop_back();

		if (DatagramSeq::LastOutSeq < pNet->OutSequenceNr)
		{
			DatagramSeq::LastOutSeq = pNet->OutSequenceNr;
			restore_datagrams.emplace_back(DatagramSeq::LastOutSeq, pNet->InReliableState, Interfaces::GlobalVars->CurTime);
		}
	}
}


BackTrack::MainHack::MainHack()
{
	M0EventManager::AddListener(
		EVENT_KEY_LOAD_DLL,
		[this](M0EventData*)
		{
			Handler = this;
			restore_datagrams.reserve(DatagramSeq::RestoreCapacity);

			void* ptr = M0Library{ M0ENGINE_DLL }.FindPattern("NetChan::SendDatagram");
			if (ptr)
				DETOUR_LINK_TO_MEMBER(NetChannel_SendDatagram, ptr);

			M0HookManager::Policy::CreateMove createmove(true);
			{
				createmove->AddPreHook(HookCall::VeryEarly, std::bind(&MainHack::PreCreateMove, this, std::placeholders::_2));
				createmove->AddPostHook(HookCall::VeryLate, std::bind(&MainHack::PostCreateMove, this, std::placeholders::_2));
			}
			
			M0HookManager::Policy::LevelShutdown levelshutdown(true);
			{
				levelshutdown->AddPreHook(
					HookCall::Any, 
					[this]() -> MHookRes
					{ 
						DatagramSeq::LastOutSeq = 0;
						restore_datagrams.clear();
						this->clear();

						return { };
					}
				);
			}
		},
		EVENT_NULL_NAME
	);

	M0EventManager::AddListener(
		EVENT_KEY_UNLOAD_DLL,
		[](M0EventData*)
		{
			DETOUR_UNLINK_FROM_MEMBER(NetChannel_SendDatagram);
		},
		EVENT_NULL_NAME
	);
}