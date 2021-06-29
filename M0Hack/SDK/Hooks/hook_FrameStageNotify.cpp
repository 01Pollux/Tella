#include "FrameStageNotify.hpp"
#include "GlobalHooks/event_listener.hpp"

TH_DECL_HANDLER_MFP(FrameStageNotify, M0PROFILER_GROUP::HOOK_FRAME_STAGE_NOTIFY, void, ClientFrameStage);

//EXPOSE_VHOOK(FrameStageNotify, "FrameStageNotify", Interfaces::ClientDLL, Offsets::ClientDLL::VTIdx_FrameStageNotify);


class FrameStageNotify_Mgr
{
public:
	FrameStageNotify_Mgr()
	{
		using namespace tella;
		event_listener::insert(
			event_listener::names::LoadDLL_Early,
			[this](event_listener::data*)
			{
				FrameStageNotify.find(TH_REFERENCE_NAME(FrameStageNotify));
			},
			event_listener::names::Null
		);
	}

private:
	TH_REFERENCE(FrameStageNotify) FrameStageNotify;
} static fsn_hook;