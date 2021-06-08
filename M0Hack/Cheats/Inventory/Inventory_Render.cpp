#pragma once

#include "Inventory_Models.hpp"
#include "Inventory_Skins.hpp"

namespace Inventory
{
	class RenderHelper
	{
	public:
		RenderHelper();
	};
}

static Inventory::RenderHelper render_helper;

Inventory::RenderHelper::RenderHelper()
{
	M0EventManager::AddListener(
		EVENT_KEY_RENDER_MENU,
		[](M0EventData*)
		{
			if (ImGui::BeginTabItem("Inv Manager"))
			{
				ModelManager::OnRender();
				SkinManager::OnRender();
				ImGui::EndTabItem();
			}
		},
		"Inventory::OnRender"
	);
}