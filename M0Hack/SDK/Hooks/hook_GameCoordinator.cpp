#include "FrameStageNotify.hpp"
#include "GlobalHook/listener.hpp"


DECL_VHOOK_HANDLER(FrameStageNotify, M0PROFILER_GROUP::HOOK_FRAME_STAGE_NOTIFY, void, ClientFrameStage);
EXPOSE_VHOOK(FrameStageNotify, "FrameStageNotify", Interfaces::ClientDLL, Offsets::ClientDLL::VTIdx_FrameStageNotify);


class FrameStageNotify_Mgr
{
public:
	FrameStageNotify_Mgr()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[this](M0EventData*)
			{
				FrameStageNotify.init();
			},
			EVENT_NULL_NAME
		);

		M0EventManager::AddListener(
			EVENT_KEY_UNLOAD_DLL_LATE,
			[this](M0EventData*)
			{
				FrameStageNotify.shutdown();
			},
			EVENT_NULL_NAME
		);
	}

private:
	FrameStageNotify_Hook FrameStageNotify;
} static fsn_hook;