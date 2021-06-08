#include "HudElement.h"

#include "Library/Lib.hpp"
#include "Helper/Offsets.hpp"

ITFHudManager::ITFHudManager()
{
	namespace HUD = Offsets::ClientDLL::HUD;

	static IHudElement*** gHUD = reinterpret_cast<IHudElement***>(M0Libraries::Client->FindPattern("gHUD"));

	huds = gHUD[HUD::Manager::This_To_Vec];
	count = reinterpret_cast<int>(gHUD[HUD::Manager::This_To_SizeOfVec]);
}

IHudElement* ITFHudManager::find(const char* name) noexcept
{
	for (auto it = begin(); it != end(); it++)
	{
		if (!strcmp((*it)->GetName(), name))
			return *it;
	}
	return nullptr;
}

const IHudElement* ITFHudManager::find(const char* name) const noexcept
{
	for (auto it = begin(); it != end(); it++)
	{
		if (!strcmp((*it)->GetName(), name))
			return *it;
	}
	return nullptr;
}

VGUI::VPANEL FindPanelByName(const char* name)
{
	int count = Interfaces::VGUISurface->GetPopupCount();
	for (int i = 0; i < count; i++)
	{
		VGUI::VPANEL hPanel = Interfaces::VGUISurface->GetPopup(i);
		const char* cur_name = Interfaces::VGUIPanel->GetName(hPanel);

		if (!strcmp(name, cur_name))
			return hPanel;
		else
		{
			auto& Childrens = Interfaces::VGUIPanel->GetChildren(hPanel);
			for (int i = 0; i < Childrens.Count(); i++)
			{
				VGUI::VPANEL hChild = Childrens[i];
				cur_name = Interfaces::VGUIPanel->GetName(hChild);
				if (!strcmp(name, cur_name))
					return hPanel;
			}
		}
	}

	return NULL;
}