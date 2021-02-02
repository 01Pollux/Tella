
#include "../Interfaces/VGUIS.h"
#include "Visual.h"

#include "../Helpers/VTable.h"
#include "../Helpers/Offsets.h"

vgui::IPanel* panel;
IInputSystem* inputsys;


DECL_VHOOK_CLASS(void, PaintTraverse, uint panelID, bool force, bool allowforce)
{
	static uint32_t FocusOverlayPanel = 0;
	static uint32_t HudScope = 0;

	if (!FocusOverlayPanel)
	{
		const char* szName = panel->GetName(panelID);
		if (szName[0] == 'F' && szName[5] == 'O' && szName[12] == 'P')
		{
			FocusOverlayPanel = panelID;
		}
	}
	
	if (panelID == FocusOverlayPanel)
	{
		VHOOK_EXECUTE(PaintTraverse, panelID, force, allowforce);

		if (VMenu.bCleanScreenshot && engineclient->IsTakingScreenshot())
			return;

		IGlobalEvent::PaintTraverse::Hook::RunAllHooks();
		return;
	}

	if (!HudScope && !strcmp(panel->GetName(panelID), "HudScope"))
		HudScope = panelID;

	if (VMenu.bHideScope && HudScope == panelID)
		return;

	VHOOK_EXECUTE(PaintTraverse, panelID, force, allowforce);
}


class PanelHook
{
	IUniqueVHook paint_traverse;

public:
	PanelHook()
	{
		IGlobalEvent::LoadDLL::Hook::Register(
			[this]() -> HookRes
			{
				paint_traverse = std::make_unique<ISingleVHook>(panel, VHOOK_GET_CALLBACK(PaintTraverse), IGlobalEvent::PaintTraverse::Offset);
				VHOOK_LINK(paint_traverse, PaintTraverse);
				return HookRes::Continue;
			});
	};

	HookRes OnDLLAttach()
	{
		

		return HookRes::Continue;
	}
} static panelhook;