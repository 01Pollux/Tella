#include "LevelChange.hpp"
#include "GlobalHook/listener.hpp"
#include "Helper/Timer.hpp"
#include "ResourceEntity.hpp"


DECL_VHOOK_HANDLER(LevelInit,		M0PROFILER_GROUP::GLOBAL_HOOK_ANY, void, const char*);
DECL_VHOOK_HANDLER(LevelShutdown,	M0PROFILER_GROUP::GLOBAL_HOOK_ANY, void);

EXPOSE_VHOOK(LevelInit,		"LevelInit",	M0Library{M0CLIENT_DLL}.FindPattern("ClientModePointer"), Offsets::ClientDLL::VTIdx_LevelInit);
EXPOSE_VHOOK(LevelShutdown, "LevelShutdown",M0Library{M0CLIENT_DLL}.FindPattern("ClientModePointer"), Offsets::ClientDLL::VTIdx_LevelShutdown);



class LevelChange_Mgr
{
public:
	LevelChange_Mgr()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[this](M0EventData*)
			{
				LevelInit.init();
				LevelShutdown.init();

				LevelShutdown->AddPostHook(
					HookCall::ReservedFirst,
					[]() -> MHookRes
					{
						ITimerSys::RunOnLevelShutdown();
						TFResourceEntity::Player = nullptr;
						TFResourceEntity::Monster = nullptr;

						return { };
					}
				);

			},
			EVENT_NULL_NAME
		);

		M0EventManager::AddListener(
			EVENT_KEY_UNLOAD_DLL_LATE,
			[this](M0EventData*)
			{
				LevelInit.shutdown();
				LevelShutdown.shutdown();
			},
			EVENT_NULL_NAME
		);
	}


private:
	LevelInit_Hook LevelInit;
	LevelShutdown_Hook LevelShutdown;
} static levelchange_hook;