
#include "vhook.h"
#include "detour.h"

#include "../Helpers/Offsets.h"
#include "../Helpers/Library.h"


static M0HOOK_STORAGE* global_hook_storage = new M0HOOK_STORAGE;


M0HOOK_STORAGE* GetStorage() noexcept
{
	return global_hook_storage;
}


#define MAKE_VHOOK(NAME, CUSTOM_NAME, THIS_PTR, OFFSET) \
	static_assert(IGlobalVHookPolicy::NAME::Name == CUSTOM_NAME, "Hook Name mismatch"); \
	class NAME##_Hook \
	{ \
		GH##NAME##_Type* GH##NAME{ }; \
		 \
	public: \
		NAME##_Hook() noexcept \
		{ \
			GH##NAME = GH##NAME##_Type::MakeHook(CUSTOM_NAME); \
			GH##NAME##UH::Handler = GH##NAME; \
			GH##NAME->Alloc(THIS_PTR, GetAddressOfFunction(DummyFn(&GH##NAME##UH::Func)), OFFSET); \
		}; \
		~NAME##_Hook() noexcept(false) \
		{ \
			GH##NAME##_Type::DeleteHook(CUSTOM_NAME); \
		} \
		NAME##_Hook(const NAME##_Hook&)			  = delete; \
		NAME##_Hook& operator=(const NAME##_Hook&)= delete; \
		NAME##_Hook(NAME##_Hook&&)				  = delete; \
		NAME##_Hook& operator=(NAME##_Hook&&)	  = delete; \
	}

#define MAKE_DHOOK(NAME, CUSTOM_NAME, ADDRESS) \
	static_assert(IGlobalDHookPolicy::NAME::Name == CUSTOM_NAME, "Hook Name mismatch"); \
	class NAME##_Hook \
	{ \
		GH##NAME##_Type* GH##NAME{ }; \
		 \
	public: \
		NAME##_Hook() noexcept \
		{ \
			GH##NAME = GH##NAME##_Type::MakeHook(CUSTOM_NAME); \
			GH##NAME##UH::Handler = GH##NAME; \
			GH##NAME->Alloc(reinterpret_cast<void*>(Library::ADDRESS), GetAddressOfFunction(DummyFn(&GH##NAME##UH::Func))); \
		}; \
		~NAME##_Hook() noexcept(false) \
		{ \
			GH##NAME##_Type::DeleteHook(CUSTOM_NAME); \
		} \
		NAME##_Hook(const NAME##_Hook&)			  = delete; \
		NAME##_Hook& operator=(const NAME##_Hook&)= delete; \
		NAME##_Hook(NAME##_Hook&&)				  = delete; \
		NAME##_Hook& operator=(NAME##_Hook&&)	  = delete; \
	}


static inline void* GetClientModePtr()
{
	return reinterpret_cast<void*>(Library::clientlib.FindPattern("pClientModePtr"));
}

#pragma region("Paint Traverse")

#include "../Interfaces/VGUIS.h"
#include "../Interfaces/IVEngineClient.h"

vgui::IPanel* panel;

DECL_VHOOK_HANDLER(PaintTraverse, M0PROFILER_GROUP::HOOK_PAINT_TRAVERSE, void, unsigned int, bool, bool);
MAKE_VHOOK(PaintTraverse, "PaintTraverse", panel, Offsets::ClientDLL::VTIdx_PaintTraverse);

#pragma endregion

#pragma region("Frame Stage Notify")

#include "../Interfaces/IBaseClientDLL.h"

DECL_VHOOK_HANDLER(FrameStageNotify, M0PROFILER_GROUP::HOOK_FRAME_STAGE_NOTIFY, void, ClientFrameStage_t);
MAKE_VHOOK(FrameStageNotify, "FrameStageNotify", clientdll, Offsets::ClientDLL::VTIdx_FrameStageNotify);

#pragma endregion

#pragma region("Create Move")

#include "../Interfaces/IClientMode.h"
#include "../Interfaces/NetMessage.h"

DECL_VHOOK_HANDLER(CreateMove, M0PROFILER_GROUP::HOOK_CREATE_MOVE, bool, float, CUserCmd*);
MAKE_VHOOK(CreateMove, "CreateMove", GetClientModePtr(), Offsets::ClientDLL::VTIdx_CreateMove);

#pragma endregion

#pragma region("Level Init")

DECL_VHOOK_HANDLER(LevelInit, M0PROFILER_GROUP::GLOBAL_VHOOK_ANY, void, const char*);
MAKE_VHOOK(LevelInit, "LevelInit", GetClientModePtr(), Offsets::ClientDLL::VTIdx_LevelInit);

#pragma endregion

#pragma region("Level Shutdown")

DECL_VHOOK_HANDLER(LevelShutdown, M0PROFILER_GROUP::GLOBAL_VHOOK_ANY, void);
MAKE_VHOOK(LevelShutdown, "LevelShutdown", GetClientModePtr(), Offsets::ClientDLL::VTIdx_LevelShutdown);

#pragma endregion

#pragma region("Send Net Msg")

DECL_DHOOK_HANDLER(SendNetMsg, M0PROFILER_GROUP::HOOK_SEND_NETMESSAGE, bool, INetMessage&, bool, bool);
MAKE_DHOOK(SendNetMsg, "SendNetMsg", enginelib.FindPattern("NetChan::SendNetMsg"));

#pragma endregion


#define UNIQUE_HOOK(NAME) static std::unique_ptr<NAME##_Hook>
#define MAKE_UNIQUE(NAME) std::make_unique<NAME##_Hook>()

using namespace std::chrono_literals;

namespace IGlobalVHookManager
{
	std::mutex vhook_init;

	UNIQUE_HOOK(PaintTraverse)		PT;
	UNIQUE_HOOK(FrameStageNotify)	FSN;
	UNIQUE_HOOK(CreateMove)			CM;
	UNIQUE_HOOK(LevelInit)			LI;
	UNIQUE_HOOK(LevelShutdown)		LS;
	UNIQUE_HOOK(SendNetMsg)			NS;

	void Init()
	{
		{
			std::lock_guard lock(vhook_init);

			PT = MAKE_UNIQUE(PaintTraverse);
			FSN = MAKE_UNIQUE(FrameStageNotify);
			CM = MAKE_UNIQUE(CreateMove);
			LI = MAKE_UNIQUE(LevelInit);
			LS = MAKE_UNIQUE(LevelShutdown);
			NS = MAKE_UNIQUE(SendNetMsg);
		}

		std::this_thread::sleep_for(750ms);
	}

	void Shutdown()
	{
		{
			std::lock_guard guard(vhook_init);

			PT = nullptr;
			FSN = nullptr;
			CM = nullptr;
			LI = nullptr;
			LS = nullptr;
			NS = nullptr;
		}
		
		std::this_thread::sleep_for(750ms);
	}
}