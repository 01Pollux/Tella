#include "CreateMove.hpp"
#include "GlobalHooks/event_listener.hpp"
#include "GlobalHooks/th_vtable.hpp"
#include "Helper/Timer.hpp"
#include "Library/Lib.hpp"
#include "Helper/Offsets.hpp"

TH_DECL_HANDLER_MFP(CreateMove, M0PROFILER_GROUP::HOOK_CREATE_MOVE, bool, float, UserCmd*);

namespace Interfaces { bool* pSendPacket = nullptr; }

class CreateMove_Mgr
{
public:
	CreateMove_Mgr()
	{
		using namespace tella;
		event_listener::insert(
			event_listener::names::LoadDLL_Early,
			[this](event_listener::data*)
			{
				hook::vtable cmvtable(M0Library{ M0CLIENT_DLL }.FindPattern("ClientModePointer"), Offsets::ClientDLL::VTIdx_CreateMove);
				TH_ALLOC_HANDLER_ONCE(CreateMove, "CreateMove", *cmvtable);

				CreateMoveHook.find(TH_REFERENCE_NAME(CreateMoveHook));
				CreateMoveHook->AddPreHook(
					hook_order::reserved_first, 
					std::bind(&CreateMove_Mgr::OnCreateMove, this, hook::arg::_3)
				);

				CreateMoveHook->AddPostHook(
					hook_order::reserved_first, 
					[](void*, float, const UserCmd* cmd)
					{
						return !cmd ? tella::to_bitmask(tella::hook_results_::break_loop, tella::hook_results_::skip_post) : tella::hook_results{ };
					}
				);
			},
			event_listener::names::Null
		);
	}

	tella::hook_results OnCreateMove(const UserCmd* cmd)
	{
		if (!cmd)
			return bitmask::to_mask(tella::hook_results_::break_loop, tella::hook_results_::skip_post);

		_asm
		{
			mov eax, ebp
			mov eax, [eax]
			mov eax, [eax]
			dec eax
			mov Interfaces::pSendPacket, eax
		};

		tella::timer::_execute_frame();

		return { };
	}

private:
	TH_REFERENCE(CreateMove) CreateMoveHook;
} static createmove_hook;