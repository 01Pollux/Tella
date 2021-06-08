#include "esp.hpp"

namespace ESPHelper
{
	using ESPInst = ESPInfo::Shared*;
	constexpr const char* ESPTeam[] = { ESP_RED_TEAM, ESP_BLU_TEAM };

	void DrawGeneric(ESPInst esp)
	{
		ImGui::Checkbox("Enable", esp->Enable.data());
		ImGui::SameLineHelp(esp->Enable);
		ImGui::Separator();

		ImGui::Text("Draw Color:");
		ImGui::ColorEdit4("##COLOR1", esp->DrawColor.get());
		ImGui::SameLineHelp(esp->DrawColor);

		ImGui::Text("Font Color:");
		ImGui::ColorEdit4("##COLOR2", esp->FontColor.get());
		ImGui::SameLineHelp(esp->FontColor);
		ImGui::Separator();

		ImGui::DragInt("##MAXDIST", esp->MaxDistance.data());
		ImGui::SameLineHelp(esp->MaxDistance);
		ImGui::Separator();

		ImGui::Text("Modes:");

		if (ImGui::RadioButton("None", esp->DrawMode == ESPDrawMode::None))					esp->DrawMode = ESPDrawMode::None;
		if (ImGui::RadioButton("Box", esp->DrawMode == ESPDrawMode::Box))					esp->DrawMode = ESPDrawMode::Box;
		if (ImGui::RadioButton("Outlined Box", esp->DrawMode == ESPDrawMode::BoxOutline))	esp->DrawMode = ESPDrawMode::BoxOutline;
	}
}


void IESPHack::OnRender()
{
	if (ImGui::BeginTabItem("ESP"))
	{
		PROFILE_USECTION("Draw ESP Menu", M0PROFILER_GROUP::DISPATCH_IMGUI);

		if (ImGui::CollapsingHeader(ESP_PLR_MGR))
		{
			ImGui::PushID("PM");
			for (size_t i = 0; i < SizeOfArray(PlayerESPInfo); i++)
			{
				ImGui::PushID(i);

				if (ImGui::TreeNode(ESPHelper::ESPTeam[i]))
				{
					auto cur_esp = &PlayerESPInfo[i];
					ESPHelper::DrawGeneric(cur_esp);

					//what to display
					{
						ImGui::Dummy(ImVec2(0, 10));
						ImGui::Text("Select what to display: ");
						ImGui::Dummy(ImVec2(0, 10));

						ImGui::Checkbox("Draw Name", cur_esp->DrawName.data());				ImGui::SameLineHelp(cur_esp->DrawName);
						ImGui::Checkbox("Draw Distance", cur_esp->DrawDistance.data());		ImGui::SameLineHelp(cur_esp->DrawDistance);

						ImGui::Checkbox("Draw Class", cur_esp->DrawClass.data());			ImGui::SameLineHelp(cur_esp->DrawClass);
						ImGui::Checkbox("Draw Conditions", cur_esp->DrawCond.data());		ImGui::SameLineHelp(cur_esp->DrawCond);
						ImGui::Checkbox("Draw UberCharge", cur_esp->DrawUberPerc.data());	ImGui::SameLineHelp(cur_esp->DrawUberPerc);
						ImGui::Checkbox("Ignore Cloaked", cur_esp->IgnoreCloak.data());		ImGui::SameLineHelp(cur_esp->IgnoreCloak);

						ImGui::Checkbox("Draw Health", cur_esp->DrawHealth.data());			ImGui::SameLineHelp(cur_esp->DrawHealth);
						ImGui::Checkbox("Draw Team", cur_esp->DrawTeam.data());				ImGui::SameLineHelp(cur_esp->DrawTeam);

						ImGui::Dummy(ImVec2(0, 10));

						ImGui::Separator();
					}

					ImGui::TreePop();
				}

				ImGui::PopID();
			}
			ImGui::PopID();
		}
		ImGui::Separator();

		if (ImGui::CollapsingHeader(ESP_BLD_MGR))
		{
			ImGui::PushID("BM");
			for (size_t i = 0; i < SizeOfArray(BuildingESPInfo); i++)
			{
				if (ImGui::TreeNode(ESPHelper::ESPTeam[i]))
				{
					auto cur_esp = &BuildingESPInfo[i];
					ESPHelper::DrawGeneric(cur_esp);

					//what to display
					{
						ImGui::Dummy(ImVec2(0, 10));
						ImGui::Text("Select what to display: ");
						ImGui::Dummy(ImVec2(0, 10));

						ImGui::Checkbox("Draw Name", cur_esp->DrawName.data());				ImGui::SameLineHelp(cur_esp->DrawName);
						ImGui::Checkbox("Draw Distance", cur_esp->DrawDistance.data());		ImGui::SameLineHelp(cur_esp->DrawDistance);

						ImGui::Checkbox("Draw Owner", cur_esp->DrawOwner.data());			ImGui::SameLineHelp(cur_esp->DrawOwner);
						ImGui::Checkbox("Draw Ammo", cur_esp->DrawAmmo.data());				ImGui::SameLineHelp(cur_esp->DrawAmmo);
						ImGui::Checkbox("Draw Level", cur_esp->DrawLevel.data());			ImGui::SameLineHelp(cur_esp->DrawLevel);
						ImGui::Checkbox("Draw State", cur_esp->DrawBState.data());			ImGui::SameLineHelp(cur_esp->DrawBState);

						ImGui::Checkbox("Draw Health", cur_esp->DrawHealth.data());			ImGui::SameLineHelp(cur_esp->DrawHealth);
						ImGui::Checkbox("Draw Team", cur_esp->DrawTeam.data());				ImGui::SameLineHelp(cur_esp->DrawTeam);

						ImGui::Dummy(ImVec2(0, 10));

						ImGui::Separator();
					}

					ImGui::TreePop();
				}
			}
			ImGui::PopID();
		}
		ImGui::Separator();

		if (ImGui::CollapsingHeader(ESP_OBJ_MGR))
		{
			ImGui::PushID("OM");

			auto cur_esp = &ObjectESPInfo;
			ESPHelper::DrawGeneric(cur_esp);

			//what to display
			{
				ImGui::Dummy(ImVec2(0, 10));
				ImGui::Text("Select what to display: ");
				ImGui::Dummy(ImVec2(0, 10));

				ImGui::Checkbox("Draw Name", cur_esp->DrawName.data());					ImGui::SameLineHelp(cur_esp->DrawName);
				ImGui::Checkbox("Draw Distance", cur_esp->DrawDistance.data());			ImGui::SameLineHelp(cur_esp->DrawDistance);

				ImGui::Checkbox("Draw Packs", cur_esp->DrawPacks.data());				ImGui::SameLineHelp(cur_esp->DrawPacks);
				ImGui::Checkbox("Draw Rockets", cur_esp->DrawRockets.data());			ImGui::SameLineHelp(cur_esp->DrawRockets);

				ImGui::Checkbox("Draw Pipes", cur_esp->DrawPipes.data());				ImGui::SameLineHelp(cur_esp->DrawPipes);
				ImGui::Checkbox("Draw Stickies", cur_esp->DrawStickies.data());			ImGui::SameLineHelp(cur_esp->DrawStickies);

				ImGui::Dummy(ImVec2(0, 10));

				ImGui::Separator();
			}

			ImGui::PopID();
		}
		ImGui::Separator();

		ImGui::EndTabItem();
	}
}