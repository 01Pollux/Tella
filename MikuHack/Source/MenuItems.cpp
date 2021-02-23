
#include "Main.h"
#include "Debug.h"

#include "../Helpers/Config.h"
#include "../Helpers/DrawTools.h"
#include "../Helpers/Commons.h"

#include "../Interfaces/IVEngineClient.h"
#include "../Profiler/mprofiler.h"

#include <fstream>
#include <iomanip>

MainMenu Mmain;


static void OpenStyleManager(bool* open);
static void OpenProfileManager(bool* open);
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
			for (auto& entry : IAutoList<MenuPanel>::List())
				entry->OnRender();

			if (ImGui::BeginTabItem("Extra##EXTRA"))
			{
				for (auto& entry : IAutoList<MenuPanel>::List())
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
		static bool debug_is_open = false;
		static bool profiler_is_open = false;
		static bool begin_shutdown = false;

		if (style_is_open)
			OpenStyleManager(&style_is_open);

		if (profiler_is_open)
			OpenProfileManager(&profiler_is_open);

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
					MikuConfig::SaveSettings(".\\Miku\\Config.json");
					ImGui::SaveIniSettingsToDisk(".\\Miku\\IMGui.ini");

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

			ImGui::MenuItem("Profiler", NULL, &profiler_is_open);

			ImGui::MenuItem("Debug Control", NULL, &debug_is_open);

			if (ImGui::MenuItem("Update Screen"))
			{
				engineclient->GetScreenSize(DrawTools::m_ScreenSize.first, DrawTools::m_ScreenSize.second);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Config"))
		{
			if (ImGui::MenuItem("Load"))
			{
				MikuConfig::LoadSettings(".\\Miku\\Config.json");
				ImGui::LoadIniSettingsFromDisk(".\\Miku\\IMGui.ini");
			}

			if (ImGui::MenuItem("Save"))
			{
				MikuConfig::SaveSettings(".\\Miku\\Config.json");
				ImGui::SaveIniSettingsToDisk(".\\Miku\\IMGui.ini");
				ImGui::GetIO().WantSaveIniSettings = false;
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

void OpenDebugControl(bool* open)
{
	ImGui::Begin("Debug Control", open);
	ImGui::SetWindowSize(ImVec2{ 750, 426 });

	AutoBool Debugging("MIKUDebug.m_bDebugging");
	ImGui::Checkbox("Debugging", &Debugging);

	AutoBool UserMsgDebug("MIKUDebug.UserMsgDebug");
	ImGui::Checkbox("User Message", &UserMsgDebug);

	ImGui::End();
}

void OpenProfileManager(bool* open)
{
	ImGui::Begin("Profiler Manager", open, ImGuiWindowFlags_NoResize);

	ImGui::SetWindowSize(ImVec2{ 870, 640 });

	static bool should_record[SizeOfArray(M0PROFILE_NAMES)]{ };

	for (size_t i = 0; i < SizeOfArray(M0PROFILE_NAMES); i++)
	{
		if (ImGui::TreeNode(M0PROFILE_NAMES[i]))
		{
			const M0PROFILER_GROUP group =static_cast<M0PROFILER_GROUP>(i);

			if (ImGui::Checkbox("Enable", &should_record[i]))
			{
				if (should_record[i])	M0Profiler::Start(group);
				else					M0Profiler::Stop(group);
			}
			
			ImGui::SameLine();
			static  bool output_all = false;
			ImGui::Checkbox("Output All", &output_all);

			ImGui::SameLine();
			if (ImGui::Button("Export"))
			{
				M0Profiler::Stop(group);
				should_record[i] = false;

				time_t this_time; time(&this_time);
				tm* time_info = localtime(&this_time);

				std::string path(Format(M0PROFILER_OUT_STREAM, M0PROFILE_NAMES[i], std::put_time(time_info, "__%h_%d_%H_%M_%S"), ".txt"));
				std::ofstream output(path, std::ios::app | std::ios::out);

				M0Profiler::OutputToFile(group, output, output_all ? M0PROFILER_FLAGS::RESULTS_ONLY : M0PROFILER_FLAGS::EMPTY);

				M0Profiler::Reset(group);
			}
		}
	}

	ImGui::End();
}

//