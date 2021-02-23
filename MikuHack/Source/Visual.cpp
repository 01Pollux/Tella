#include "../GlobalHook/vhook.h"
#include "../GlobalHook/load_routine.h"
#include "../Helpers/Commons.h"
#include "../Helpers/DrawTools.h"
#include "../Interfaces/VGUIS.h"

#include "Main.h"

class VisualMenu : public MenuPanel, public IMainRoutine
{
public:	// VisualMenu
	AutoBool bHideScope{ "VisualMenu::bHideScope", false };
	AutoBool bCleanScreenshot{ "VisualMenu::bCleanScreenshot", true };
	AutoBool bWhosWatchingMe{ "VisualMenu::bWhosWatchingMe", true };
	IAutoArray<int, 2> iWatchOffset{ "VisualMenu::Offset", { 12, 130 } };

public:
	HookRes OnPaintTraverse(uint);

public:	// MenuPanel
	void OnRender() override;
	void JsonCallback(Json::Value& json, bool read) override;


public:	// IMainRoutine
	void OnLoadDLL() override
	{
		using namespace IGlobalVHookPolicy;
		auto paint_traverse = PaintTraverse::Hook::QueryHook(PaintTraverse::Name);
		paint_traverse->AddPreHook(HookCall::Early,
			[this](uint pId, bool, bool)
			{
				return (PID_HudScope == pId && bHideScope) ?
					HookRes::DontCall | HookRes::BreakImmediate : HookRes::Continue;
			});
		paint_traverse->AddPostHook(HookCall::Late, std::bind(&VisualMenu::OnPaintTraverse, this, std::placeholders::_1));
	}
} static visual_menu;


void VisualMenu::OnRender()
{
	if (ImGui::BeginTabItem("Visual"))
	{
		ImGui::Checkbox("No scope", &bHideScope);
		ImGui::SameLine(); ImGui::DrawHelp("Hide sniper zoom overlay");

		ImGui::Checkbox("Clean SS", &bCleanScreenshot);
		ImGui::SameLine(); ImGui::DrawHelp("Hide cheat from screenshots");

		ImGui::Checkbox("Spectators", &bWhosWatchingMe);
		ImGui::SameLine(); ImGui::DrawHelp("Expose who is spectating you");

		ImGui::SliderInt2("Text Adjustor", &iWatchOffset, 0, 1980);

		ImGui::EndTabItem();
	}
}


HookRes VisualMenu::OnPaintTraverse(uint pId)
{
	if (pId != PID_FocusOverlay)
		return HookRes::BreakImmediate;

	if (BadLocal())
		return HookRes::Continue;

	if (!bWhosWatchingMe)
		return HookRes::Continue;

	static player_info_t info;

	int draw_offset = DrawTools::m_ScreenSize.second - iWatchOffset[1];
	int iLocalplayer = ::ILocalIdx();
	ITFPlayer* pMe = ::ILocalPtr();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		ITFPlayer* pEnt = ::GetITFPlayer(i);
		if (BadEntity(pEnt) || pMe == pEnt)
			continue;

		if (pEnt->GetLifeState() == LIFE_STATE::ALIVE)
			continue;

		ITFPlayer* pSpectated = ::GetITFPlayer(*pEnt->GetEntProp<IBaseHandle, PropType::Recv>("m_hObserverTarget"));
		if (pSpectated != pEnt)
			continue;
		
		const char* mode;
		switch (*pEnt->GetEntProp<int, PropType::Recv>("m_iObserverMode"))
		{
		case 4:
			mode = "FirstPerson : ";
			break;

		case 5:
			mode = "ThirdPerson : ";
			break;

		case 7:
			mode = "FreeCam : ";
			break;

		default: 
			mode = "";
			break;
		}

		if (engineclient->GetPlayerInfo(i, &info))
		{
			using namespace DrawTools;

			std::string str = mode + std::string(info.name);
			DrawString(iWatchOffset[0], draw_offset, ColorTools::FromArray(ColorTools::White<char8_t>), str);
			draw_offset -= m_iStringOffset;
		}
	}
	
	return HookRes::Continue;
}


void VisualMenu::JsonCallback(Json::Value& json, bool read)
{
	Json::Value& VCfg = json["Visuals"];
	if (read)
	{
		PROCESS_JSON_READ(VCfg, "NoScope", Bool, bHideScope);
		PROCESS_JSON_READ(VCfg, "Clean Screenshot", Bool, bCleanScreenshot);
		PROCESS_JSON_READ(VCfg, "SpecList", Bool, bWhosWatchingMe);
		PROCESS_JSON_READ(VCfg, "Spec X", Int, iWatchOffset[0]);
		PROCESS_JSON_READ(VCfg, "Spec Y", Int, iWatchOffset[1]);
	}
	else {
		PROCESS_JSON_WRITE(VCfg, "NoScope", *bHideScope);
		PROCESS_JSON_WRITE(VCfg, "Clean Screenshot", *bCleanScreenshot);
		PROCESS_JSON_WRITE(VCfg, "SpecList", *bWhosWatchingMe);
		PROCESS_JSON_WRITE(VCfg, "Spec X", iWatchOffset[0]);
		PROCESS_JSON_WRITE(VCfg, "Spec Y", iWatchOffset[1]);
	}
}