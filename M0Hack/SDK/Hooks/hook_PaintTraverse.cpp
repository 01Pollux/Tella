#include "PaintTraverse.hpp"
#include "GlobalHook/listener.hpp"


DECL_VHOOK_HANDLER(PaintTraverse, M0PROFILER_GROUP::HOOK_PAINT_TRAVERSE, void, unsigned int, bool, bool);
EXPOSE_VHOOK(PaintTraverse, "PaintTraverse", Interfaces::VGUIPanel, Offsets::VGUI::VTIdx_PaintTraverse);


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