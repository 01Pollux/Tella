#include "CreateMove.hpp"
#include "GlobalHook/listener.hpp"
#include "Helper/Timer.hpp"


DECL_VHOOK_HANDLER(CreateMove, M0PROFILER_GROUP::HOOK_CREATE_MOVE, bool, float, UserCmd*);
EXPOSE_VHOOK(CreateMove, "CreateMove", M0Libraries::Client->FindPattern("ClientModePointer"), Offsets::ClientDLL::VTIdx_CreateMove);



M0_INTERFACE;
bool* pSendPacket = nullptr;
M0_END;

class CreateMove_Mgr
{
public:
	CreateMove_Mgr()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[this](M0EventData*)
			{
				CreateMove.init();
				CreateMove->AddPreHook(HookCall::ReservedFirst, std::bind(&CreateMove_Mgr::OnCreateMove, this, std::placeholders::_2));
				CreateMove->AddPostHook(HookCall::ReservedFirst, [](float, UserCmd* cmd) { return !cmd ? HookRes::BreakImmediate : HookRes::Continue; });
			},
			EVENT_NULL_NAME
		);

		M0EventManager::AddListener(
			EVENT_KEY_UNLOAD_DLL_LATE,
			[this](M0EventData*)
			{
				CreateMove.shutdown();
			},
			EVENT_NULL_NAME
		);
	}

	HookRes OnCreateMove(const UserCmd* cmd)
	{
		if (!cmd)
			return HookRes::BreakImmediate;

		_asm
		{
			mov eax, ebp
			mov eax, [eax]
			mov eax, [eax]
			dec eax
			mov Interfaces::pSendPacket, eax
		};

		ITimerSys::ExecuteFrame();

		return HookRes::Continue;
	}

private:
	CreateMove_Hook CreateMove;
} static createmove_hook;