
#include "../Helpers/AutoItem.h"
#include "../Helpers/Timer.h"
#include "../Helpers/Commons.h"

#include "../Source/Main.h"

#include "vhook.h"
#include "load_routine.h"
#include "../Interfaces/VGUIS.h"

class CUserCmd;
class ILegacyHooks: public IMainRoutine
{
	IAutoVar<bool*> bSendPacket{ "bSendPacket", nullptr };

public:
	HookRes CreateMove(CUserCmd* cmd)
	{
		if (!cmd)
			return HookRes::BreakImmediate;

		bool*& send_packet = bSendPacket;

		__asm
		{
			mov eax, ebp
			mov eax, [eax]
			mov eax, [eax]
			mov eax, [eax]
			dec eax
			mov send_packet, eax
		};

		Timer::ExecuteFrame();
		
		return HookRes::Continue;
	}

	HookRes PaintTraverse(unsigned int panelID)
	{
		if (!PID_FocusOverlay)
		{
			const char* name = panel->GetName(panelID);
			if (name[0] == 'F' && name[5] == 'O' && name[12] == 'P')
				PID_FocusOverlay = panelID;
		}
		else if (!PID_HudScope && !strcmp(panel->GetName(panelID), "HudScope"))
			PID_HudScope = panelID;

		return (panelID == PID_HudScope || panelID == PID_FocusOverlay) ? HookRes::Continue : HookRes::BreakImmediate;
	}

	void OnLoadDLL() override
	{
		auto createmove = IGlobalVHook<bool, float, CUserCmd*>::QueryHook("CreateMove");
		createmove->AddPreHook(HookCall::ReservedFirst, std::bind(&ILegacyHooks::CreateMove, this, std::placeholders::_2));
		createmove->AddPostHook(HookCall::ReservedFirst, [](float, CUserCmd* cmd) { return !cmd ? HookRes::BreakImmediate : HookRes::Continue; } );

		auto levelshutdown = IGlobalVHook<void>::QueryHook("LevelShutdown");
		levelshutdown->AddPostHook(HookCall::ReservedFirst, []() { Timer::RunOnLevelShutdown(); return HookRes::Continue; });

		auto paint_traverse = IGlobalVHook<void, uint, bool, bool>::QueryHook("PaintTraverse");
		paint_traverse->AddPreHook(HookCall::ReservedFirst, std::bind(&ILegacyHooks::PaintTraverse, this, std::placeholders::_1));
	}

} static legacy_hooks;