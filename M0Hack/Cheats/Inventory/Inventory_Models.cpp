#include "Inventory_Models.hpp"
#include "NetStringTable.hpp"
#include <fstream>

#include "Library/Lib.hpp"
#include "Helper/Debug.hpp"
#include "Hooks/LevelChange.hpp"
#include "ImGui/imgui_helper.h"


static Inventory::ModelManager model_manager;

void Inventory::PlayerDataModel::init()
{
	GetPlayerClassData = M0Libraries::Client->FindPattern("GetPlayerClassData");
}

void Inventory::PlayerDataModel::set(TFClass playercls, std::string_view newmdl) noexcept
{
	M0Pointer* data = GetPlayerClassData(static_cast<uint32_t>(playercls));
	M0Pointer* mdl = &data[Offsets::ClientDLL::PlayerData_t::OffsetToModel];

	if (!newmdl.data())
		newmdl = ActualTFModels[static_cast<size_t>(playercls)];

	strncpy_s(reinterpret_cast<char*>(mdl), Offsets::ClientDLL::PlayerData_t::SizeOfModel, newmdl.data(), newmdl.size());
}

/*
* Structure of "/Miku/Models.json"
{
	"<Class>" {
		"Current" : "XYZ"
		"Models" {
			"ABC": "mdl1.mdl",
			"XYZ": "mdl2.mdl"
		}
	}
}
*/

Inventory::ModelManager::ModelManager()
{
	M0EventManager::AddListener(
		EVENT_KEY_LOAD_DLL,
		[this](M0EventData*)
		{
			DataModel.init();

			Json::Value data;
			std::ifstream cfg("./Miku/Models.json", std::ios::binary);
			cfg >> data;

			for (size_t i = 0; i < SizeOfArray(TFClassNames); i++)
			{
				const char* cls = TFClassNames[i];
				Json::Value& curClass = data[cls];

				if (!curClass.size())
					continue;

				auto& map = Models[i].ModelMap;
				const Json::Value& models = curClass["Models"];

				for (const auto& mdl_names = models.getMemberNames();
					const std::string& mdl : mdl_names)
				{
					if (mdl.size())
						map[mdl] = curClass["Models"][mdl].asString();
				}

				if (Enable)
				{
					if (const Json::Value& selected = curClass["Current"];
						!selected.isNull())
					{
						Models[i].Current = selected.asString();
						DataModel.set(static_cast<TFClass>(i + 1), Models[i].get());
					}
				}
			}

			PrecacheModels();

			M0HookManager::Policy::LevelInit levelinit(true);
			levelinit->AddPostHook(
				HookCall::Any,
				[this](const char*)
				{
					PrecacheModels();
					return HookRes::Continue;
				}
			);
		},
		EVENT_NULL_NAME
	);

	M0EventManager::AddListener(
		EVENT_KEY_UNLOAD_DLL,
		[this](M0EventData*)
		{
			DataModel.reset();
		},
		EVENT_NULL_NAME
	);
}


void Inventory::ModelManager::PrecacheModels()
{
	auto modelprecache = Interfaces::NSTContainer->FindTable("modelprecache");
	if (!modelprecache)
		return;
	
	for (size_t i = 0; i < sizeof_Models; i++)
	{
		auto& data = Models[i].ModelMap;
		for (const auto& [name, path] : data)
		{
			if (modelprecache->AddString(NST_CLIENT_SIDE, path.c_str()) == INVALID_NST_INDEX)
				M0Logger::Err("[Models.json] Failed to Precache model: \"{}\":\"{}\"", name, path);
		}
	}
}


void Inventory::ModelManager::OnRender()
{
	if (ImGui::CollapsingHeader("Models"))
	{
		ImGui::Checkbox("Enabled: ", model_manager.Enable.data());
		ImGui::Separator();

		if (model_manager.Enable)
		{
			static int curClass{ 0 };

			//Force Full Update
			{
				if (ImGui::Button("Force Full Update"))
					Interfaces::ForceClientFullUpdate();
				ImGui::Dummy(ImVec2(0, 10));
			}

			{
				ImGui::Combo("Class: ", &curClass, TFClassNames, SizeOfArray(TFClassNames));
				ImGui::Dummy(ImVec2(0, 10));
			}

			ImGui::Text("Models: ");
			if (ImGui::BeginListBox("##ModelList", ImVec2(450.0f, 0.0f)))
			{
				{
					static AutoCTimer<0.6f> timer_update_model[sizeof_Models];
					PlayerModelInfo& Models = model_manager.Models[curClass];

					{
						ImGui::Text("Current Model: %s", Models.Current.c_str());
						for (auto& entry : Models.ModelMap)
						{
							if (ImGui::Selectable(std::format("[ {} ]", entry.first).c_str()))
							{
								if (timer_update_model[curClass].trigger_if_elapsed())
								{
									Models.set_or_toggle(entry.first);
									model_manager.DataModel.set(static_cast<TFClass>(curClass + 1), Models.get());
								}
							}
						}
					}
				}
				ImGui::EndListBox();
			}

			ImGui::Dummy(ImVec2(0, 10));
		}
	}
}