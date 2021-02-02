#include "../Helpers/DrawTools.h"
#include "Visual.h"
#include "../Helpers/Timer.h"


VisualMenu VMenu;

void VisualMenu::OnRender()
{
	if (ImGui::BeginTabItem("Visual"))
	{
		ImGui::Checkbox("No scope", bHideScope.get());
		ImGui::SameLine(); DrawTools::DrawHelp("Hide sniper zoom overlay");

		ImGui::Checkbox("Clean SS", bCleanScreenshot.get());
		ImGui::SameLine(); DrawTools::DrawHelp("Hide cheat from screenshots");

		ImGui::Checkbox("Spectators", bWhosWatchingMe.get());
		ImGui::SameLine(); DrawTools::DrawHelp("Expose who is spectating you");

		ImGui::SliderInt2("Text Adjustor", iWatchOffset.data(), 0, 1980);

		ImGui::EndTabItem();
	}
}


HookRes VisualMenu::OnPaintTraverse()
{
	if (BAD_LOCAL())
		return HookRes::Continue;

	if (!bWhosWatchingMe)
		return HookRes::Continue;

	ITFPlayer* pEnt;
	ITFPlayer* pSpectated;
	static player_info_t info;

	int draw_offset = DrawTools::m_ScreenSize.second - iWatchOffset[1];
	int iLocalplayer = iLocalPlayer;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		if (i == iLocalplayer)
			continue;
		
		pEnt = reinterpret_cast<ITFPlayer*>(clientlist->GetClientEntity(i));
		if (BAD_PLAYER(pEnt))
			continue;

		if (pEnt->GetLifeState() == LIFE_STATE::ALIVE)
			continue;

		pSpectated = reinterpret_cast<ITFPlayer*>(clientlist->GetClientEntityFromHandle(*pEnt->GetEntProp<IBaseHandle>("m_hObserverTarget")));
		if (pSpectated != pLocalPlayer)
			continue;
		
		static const char* mode = "";
		switch (*pEnt->GetEntProp<int>("m_iObserverMode"))
		{
		case 4:
			mode = " >FirstPerson";
			break;

		case 5:
			mode = " >ThirdPerson";
			break;

		case 7:
			mode = " >FreeCam";
			break;

		default: 
			mode = "";
			break;
		}

		if (engineclient->GetPlayerInfo(i, &info))
		{
			using namespace DrawTools;

			std::string str = std::string(info.name) + mode;
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