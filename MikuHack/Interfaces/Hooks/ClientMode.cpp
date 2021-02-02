
#include "../Helpers/VTable.h"

#include "../IClientMode.h"
#include "../Helpers/sdk.h"
#include <icvar.h>

#include "../NetMessage.h"

#include "Debug.h"
#include "../Helpers/Timer.h"

#include "../Detour/detours.h"

#include "../Helpers/Offsets.h"


DECL_VHOOK_CLASS(void, LevelShutdown)
{
	using LvlHook = IGlobalEvent::LevelShutdown::Hook;

	VHOOK_EXECUTE(LevelShutdown);

	LvlHook::RunAllHooks();
	
	Timer::RunOnLevelShutdown();
}

DECL_VHOOK_CLASS(void, LevelInit, const char* new_map)
{
	using LvlHook = IGlobalEvent::LevelInit::Hook;

	VHOOK_EXECUTE(LevelInit, new_map);

	LvlHook::RunAllHooks();
}

DETOUR_CREATE_MEMBER(bool, SendNetMsg, INetMessage& msg, bool bForceReliable, bool bVoice)
{
	LINKED_HOOK_PROCESS(SendNetMsg, DETOUR_MEMBER_CALL(SendNetMsg, msg, bForceReliable, bVoice), msg);
}


DECL_VHOOK_CLASS(bool, CreateMove, float flInputSampleTime, CUserCmd* cmd)
{
	bool res = VHOOK_EXECUTE(CreateMove, flInputSampleTime, cmd);

	if (BAD_LOCAL() || !cmd)
		return res;

	_asm {
		mov eax, ebp
		mov eax, [eax]
		mov eax, [eax]
		mov eax, [eax]
		dec eax
		mov Globals::bSendPacket, eax
	};

	Globals::m_pUserCmd = cmd;
	IGlobalEvent::CreateMove::Hook::RunAllHooks(res);

	Timer::ExecuteFrame();

	return res;
}


class ClientModeHook 
{
	IUniqueVHook lvl_init;
	IUniqueVHook lvl_shutdown;
	IUniqueVHook create_move;

public:
	ClientModeHook()
	{
		using namespace IGlobalEvent;
		LoadDLL::Hook::Register(std::bind(&ClientModeHook::OnDLLAttach, this));
		UnloadDLL::Hook::Register([]() -> HookRes { DETOUR_UNLINK_FROM_MEMBER(SendNetMsg); return HookRes::Continue; });
	};

	HookRes OnDLLAttach()
	{
		uintptr_t pClientMode = Library::clientlib.FindPattern("pClientModePtr");

		lvl_init = std::make_unique<ISingleVHook>(pClientMode, VHOOK_GET_CALLBACK(LevelInit), IGlobalEvent::LevelInit::Offset);
		VHOOK_LINK(lvl_init, LevelInit);

		lvl_shutdown = std::make_unique<ISingleVHook>(pClientMode, VHOOK_GET_CALLBACK(LevelShutdown), IGlobalEvent::LevelShutdown::Offset);
		VHOOK_LINK(lvl_shutdown, LevelShutdown);
		
		create_move = std::make_unique<ISingleVHook>(pClientMode, VHOOK_GET_CALLBACK(LevelShutdown), IGlobalEvent::CreateMove::Offset);
		VHOOK_LINK(create_move, CreateMove);

		{
			uintptr_t ptr = Library::enginelib.FindPattern("NetChan::SendNetMsg");
			DETOUR_LINK_TO_MEMBER(SendNetMsg, ptr);

			if (error)
				MIKUDebug::LogCritical(Format("Failed to detour \"CNetChan::SendNetMsg\"", error));
		}

		return HookRes::Continue;
	}

} static dummy_clientmode;


#include "../IVEClientTrace.h"
IEngineTrace* clienttrace;
IVDebugOverlay* debugoverlay;


#include "../HatCommand.h"

HAT_COMMAND(string_cmd, "")
{
	INetChannel* pNet = reinterpret_cast<INetChannel*>(engineclient->GetNetChannelInfo());
	if (!pNet)
		REPLY_TO_TARGET(return, "Invalid NetChannel\n");

	const char* cmd = args.ArgS();

	NET_StringCmd str_cmd(cmd);
	pNet->SendNetMsg(str_cmd);
}