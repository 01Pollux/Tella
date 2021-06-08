#include "Main.hpp"
#include "ImGui/imgui.h"
#include "Helper/DrawTools.hpp"

#include "Profiler/mprofiler.hpp"
#include "Helper/Config.hpp"

namespace MenuPanel
{
	bool IsMenuActive = false;
	void RenderMenuBar();
}

void MenuPanel::Render()
{
	ImGui::SetNextWindowPos({ 650, 20 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 550, 530 }, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Miku Hack", &MenuPanel::IsMenuActive, 
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

		MenuPanel::RenderMenuBar();

		if (ImGui::BeginTabBar("##MAIN_TAB", ImGuiTabBarFlags_Reorderable))
		{
			M0EventManager::Find(EVENT_KEY_RENDER_MENU)();

			if (ImGui::BeginTabItem("Extra##EXTRA"))
			{
				M0EventManager::Find(EVENT_KEY_RENDER_EXTRA)();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();
}

void MenuPanel::RenderMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		static bool profiler_is_open = false;

		if (profiler_is_open)
			M0Profiler::RenderToImGui(&profiler_is_open);

		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem("Profiler", NULL, &profiler_is_open);

			if (ImGui::MenuItem("Update Screen"))
				DrawTools::Update();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Config"))
		{
			if (ImGui::MenuItem("Load"))
			{
				ImGui::LoadIniSettingsFromDisk(".\\Miku\\IMGui.ini");
				M0CONFIG M0VarStorage::read_var();
			}

			if (ImGui::MenuItem("Save"))
			{
				ImGui::SaveIniSettingsToDisk(".\\Miku\\IMGui.ini");
				ImGui::GetIO().WantSaveIniSettings = false;
				M0CONFIG M0VarStorage::write_var();
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}