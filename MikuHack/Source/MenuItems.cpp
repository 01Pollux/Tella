
#include "../Helpers/DrawTools.h"

#include "Main.h"
#include "ESP.h"
#include "Visual.h"
#include "../Helpers/Config.h"

#include "Debug.h"

MainMenu Mmain;

static void OpenStyleManager(bool* open);
static void OpenColorManager(bool* open);
static void OpenDebugControl(bool* open);

//
void MainMenu::DoRender()
{
	ImGui::SetNextWindowPos({ 650, 20 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 550, 530 }, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Miku Hack", &Mmain.m_bIsActive, ImGuiWindowFlags_NoResize 
													| ImGuiWindowFlags_MenuBar 
													| ImGuiWindowFlags_NoCollapse))
	{
		

		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

		MainMenu::RenderMenuBar();

		if (ImGui::BeginTabBar("##MAIN_TAB", ImGuiTabBarFlags_Reorderable))
		{
			for (auto& entry : AutoList<MenuPanel>::List())
				entry->OnRender();

			if (ImGui::BeginTabItem("Extra##EXTRA"))
			{
				for (auto& entry : AutoList<MenuPanel>::List())
				{
					entry->OnRenderExtra();
					ImGui::Separator();
				}
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();
}
//


//
void MainMenu::RenderMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		static bool style_is_open = false;
		static bool color_is_open = false;
		static bool debug_is_open = false;
		static bool begin_shutdown = false;

		if (style_is_open)
			OpenStyleManager(&style_is_open);

		if (color_is_open)
			OpenColorManager(&color_is_open);

		if (debug_is_open)
			OpenDebugControl(&debug_is_open);

		if (begin_shutdown)
		{
			if (!ImGui::IsPopupOpen("Save To File?"))
				ImGui::OpenPopup("Save To File?");

			if (ImGui::BeginPopup("Save To File?"))
			{
				ImGui::Text("Save 01MIKU?");
				ImGui::Dummy({ 10, 0 });
				if (ImGui::Button("Yes", { 50, 0 }))
				{
					try
					{
						MikuConfig::SaveSettings(".\\Miku\\Config.json");
						ImGui::SaveIniSettingsToDisk(".\\Miku\\IMGui.ini");
					}
					catch (...)
					{
						MessageBox(NULL, "Miku-Miku failed to save your config. It may be outdated.\nClick okay to reset your config", "Error", MB_OKCANCEL);
						if (remove(".\\Miku\\Config.json"))
							MessageBox(NULL, "Failed to delete Miku-Miku's config.", "Fatal!", MB_OKCANCEL);
					}

					begin_shutdown = false;
					DrawTools::MarkForDeletion();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();

				if (ImGui::Button("No", { 50, 0 }))
				{
					begin_shutdown = false;
					DrawTools::MarkForDeletion();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();

				if (ImGui::Button("Cancel", { 50, 0 }))
				{
					begin_shutdown = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem("Shutdown", NULL, &begin_shutdown);

			ImGui::MenuItem(MIKUDebug::m_bDebugging ? "Disable Debugging" : "Enable Debugging", NULL, &MIKUDebug::m_bDebugging);
			ImGui::MenuItem(MIKUDebug::m_bConsolePrint ? "Disable Console" : "Enable Console", NULL, &MIKUDebug::m_bConsolePrint);

			ImGui::MenuItem("Change Style", NULL, &style_is_open);

			ImGui::MenuItem("Change Color", NULL, &color_is_open);

			ImGui::MenuItem("Debug Control", NULL, &debug_is_open);

			if(ImGui::MenuItem("Update Screen"))
			{
				engineclient->GetScreenSize(DrawTools::m_ScreenSize.first, DrawTools::m_ScreenSize.second);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Config"))
		{
			if (ImGui::MenuItem("Load"))
			{
				try
				{
					MikuConfig::LoadSettings(".\\Miku\\Config.json");
					ImGui::LoadIniSettingsFromDisk(".\\Miku\\IMGui.ini");
				}
				catch (...)
				{
					MessageBox(NULL, "Miku-Miku failed to save your config. It may be outdated.\nClick okay to reset your config", "Error", MB_OKCANCEL);
					if (remove(".\\Miku\\Config.json"))
						MessageBox(NULL, "Failed to delete Miku-Miku's config.", "Fatal!", MB_OKCANCEL);
				}
			}

			if (ImGui::MenuItem("Save"))
			{
				try
				{
					MikuConfig::SaveSettings(".\\Miku\\Config.json");
					ImGui::SaveIniSettingsToDisk(".\\Miku\\IMGui.ini");
					ImGui::GetIO().WantSaveIniSettings = false;
				}
				catch (...)
				{
					MessageBox(NULL, "Miku-Miku failed to save your config. It may be outdated.\nClick okay to reset your config", "Error", MB_OKCANCEL);
					if (remove(".\\Miku\\Config.json"))
						MessageBox(NULL, "Failed to delete Miku-Miku's config.", "Fatal!", MB_OKCANCEL);
				}
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}
//


//
void OpenStyleManager(bool* open)
{
	ImGui::Begin("Style Editor", open);
	ImGui::SetWindowSize(ImVec2{ 750, 426 });
	ImGui::ShowStyleEditor();
	ImGui::End();
}

void OpenColorManager(bool* open)
{
	ImGui::Begin("Color Editor", open);

	ImGui::SetWindowSize(ImVec2{ 400, 400 });

	static constexpr ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoSidePreview 
												| ImGuiColorEditFlags_AlphaBar 
												| ImGuiColorEditFlags_PickerHueWheel 
												| ImGuiColorEditFlags_DisplayHex;

	ImGuiStyle& style = ImGui::GetStyle();
	static ImVec4 colors{ style.Colors->x, style.Colors->y, style.Colors->z, style.Colors->w };

	if (ImGui::ColorPicker4("Color Picker", (float*)(&colors), flags))
	{
		style.Colors->x = Mmain.m_flColor[0] = colors.x;
		style.Colors->y = Mmain.m_flColor[1] = colors.y;
		style.Colors->z = Mmain.m_flColor[2] = colors.z;
		style.Colors->w = Mmain.m_flColor[3] = colors.w;
	}

	ImGui::End();
}

void OpenDebugControl(bool* open)
{
	ImGui::Begin("Debug Control", open);
	ImGui::SetWindowSize(ImVec2{ 750, 426 });

	AutoBool Debugging("MIKUDebug.m_bDebugging");
	ImGui::Checkbox("Debugging", Debugging.get());

	AutoBool UserMsgDebug("MIKUDebug.UserMsgDebug");
	ImGui::Checkbox("User Message", UserMsgDebug.get());

	ImGui::End();
}

//