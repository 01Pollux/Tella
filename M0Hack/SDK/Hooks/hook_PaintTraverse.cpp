#include "PaintTraverse.hpp"
#include "GlobalHook/listener.hpp"


//DECL_VHOOK_HANDLER(PaintTraverse, M0PROFILER_GROUP::HOOK_PAINT_TRAVERSE, void, unsigned int, bool, bool);
//EXPOSE_VHOOK(PaintTraverse, "PaintTraverse", Interfaces::VGUIPanel, Offsets::VGUI::VTIdx_PaintTraverse);
using GHPaintTraverse_Type = IGlobalVHook<void, unsigned int, bool, bool>; 
namespace
{
	template<typename Return, typename... Args>
	class GHPaintTraverse_Handler
	{
	public:
		static inline GHPaintTraverse_Type* Handler = nullptr;

		Return Func(Args... args)
		{
			PROFILE_UFUNCSIG(M0PROFILER_GROUP::HOOK_PAINT_TRAVERSE);
			if constexpr (std::is_void_v<Return>)
			{
				if (Handler->ProcessPreCallback(std::forward<Args>(args)...).test(HookRes::DontCall))
					return;

				Handler->ForceCall(this, args...);

				Handler->ProcessPostCallback(std::forward<Args>(args)...);
			}
			else
			{
				const MHookRes hr = Handler->ProcessPreCallback(std::forward<Args>(args)...);
				Return tmp = Handler->GetReturnInfo();
				if (hr.test(HookRes::DontCall))
					return tmp;

				const bool overwritten = hr.test(HookRes::ChangeReturnValue);

				Return ret = Handler->ForceCall(this, args...);

				const Return& actual = overwritten ? tmp : ret;
				Handler->SetReturnInfo(actual);

				if (Handler->ProcessPostCallback(std::forward<Args>(args)...) && overwritten)
					Handler->SetReturnInfo(actual);

				return Handler->GetReturnInfo();
			}
		}
	};
}
using GHPaintTraverseUH = ::GHPaintTraverse_Handler<void, unsigned int, bool, bool>;

//#define EXPOSE_VHOOK(NAME, CUSTOM_NAME, THIS_PTR, OFFSET) 

static_assert(M0HookManager::Policy::PaintTraverse::Name == "PaintTraverse", "Hook Name mismatch");
class PaintTraverse_Hook
{
	GHPaintTraverse_Type* GHPaintTraverse{ };

public:
	void init()
	{
		GHPaintTraverse = M0HookManager::AddHook<GHPaintTraverse_Type>("PaintTraverse");
		if (!GHPaintTraverse->IsActive())
		{
			GHPaintTraverseUH::Handler = GHPaintTraverse;
			GHPaintTraverse->Alloc(Interfaces::VGUIPanel, GetAddressOfFunction(&GHPaintTraverseUH::Func), Offsets::VGUI::VTIdx_PaintTraverse);
		}
	};
	void shutdown()
	{
		M0HookManager::RemoveHook("PaintTraverse");
	}
	auto operator->() noexcept { return GHPaintTraverse; }
};


namespace VGUI
{
	GAMEPROP_IMPL_OFFSET(Panel, VPanel);
	GAMEPROP_IMPL_OFFSET(Panel, Name);

	GAMEPROP_IMPL_OFFSET(Panel, FGColor);
	GAMEPROP_IMPL_OFFSET(Panel, BGColor);

	VPANEL FocusOverlayPanel = 0;
}

class PaintTraverse_Mgr
{
public:
	PaintTraverse_Mgr()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[this](M0EventData*)
			{
				PaintTraverse.init();
				VGUI::FocusOverlayPanel = FindPanelByName("FocusOverlayPanel");
			},
			EVENT_NULL_NAME
		);
		
		M0EventManager::AddListener(
			EVENT_KEY_UNLOAD_DLL_LATE,
			[this](M0EventData*)
			{
				PaintTraverse.shutdown();
			},
			EVENT_NULL_NAME
		);
	}

private:
	PaintTraverse_Hook PaintTraverse;
} static painttraverse_hook;