#include "../Interfaces/HatCommand.h"
#include "Inventory.h"

#include "../Helpers/Timer.h"
#include "../Helpers/json.h"
#include "../Helpers/Commons.h"

#include <fstream>

#include "../Helpers/VTable.h"

constexpr size_t flWeaponRefresh = 1200;

InventoryHack invmanager;
ItemManager* g_ItemManager = nullptr;

const char* m_szActualModels[] = {
	"models/player/scout.mdl",
	"models/player/scout.mdl",
	"models/player/sniper.mdl",
	"models/player/soldier.mdl",
	"models/player/demo.mdl",
	"models/player/medic.mdl",
	"models/player/heavy.mdl",
	"models/player/pyro.mdl",
	"models/player/spy.mdl",
	"models/player/engineer.mdl"
};

class _PlayerClassData
{
	using GetClassDataFn = uintptr_t*(*)(uint32_t);
	GetClassDataFn pFn;

public:
	_PlayerClassData()
	{
		pFn = reinterpret_cast<GetClassDataFn>(Library::clientlib.FindPattern("GetPlayerClassData"));
	}

	~_PlayerClassData()
	{
		Shutdown();
	}

	void SetDataModel(uint8_t cls, const char* newMdl);

	void Reset(uint8_t i)
	{
		SetDataModel(i, NULL);
	}

	void Shutdown()
	{
		for (uint8_t i = 1; i <= 9; i++)
			this->Reset(i);
	}
} static* player_data_model;

void _PlayerClassData::SetDataModel(uint8_t cls, const char* newMdl)
{
	uintptr_t* data = pFn(cls);
	
	char* szModelName = reinterpret_cast<char*>(&data[Offsets::ClientDLL::PlayerData_t::OffsetToModel]);

	if (!newMdl)
		newMdl = m_szActualModels[cls];

	strncpy(szModelName, newMdl, Offsets::ClientDLL::PlayerData_t::SizeOfModel);
}


void ForceFullUpdate();

inline void PrecacheDataModel()
{
	if (!nstcontainer)
		return;
	auto modelprecache = nstcontainer->FindTable("modelprecache");
	if (!modelprecache)
		return;

	constexpr size_t count = invmanager.Models.size();
	for (uint8_t i = 0; i < count; i++)
	{
		auto& data = invmanager.Models[i].models;
		for (auto& entry : data)
		{
			if (modelprecache->AddString(false, entry.second.c_str()) == INVALID_STRING_INDEX)
				MIKUDebug::LogCritical(Format("[Model] Failed to Precache model: \"", entry.first, "\":\"", entry.second, "\""));
		}
	}
}

static void InitModels()
{
	const std::vector<const char*> classes = {
		"Scout", "Sniper", "Soldier",
		"Demoman", "Medic", "Heavy",
		"Pyro", "Spy", "Engineer"
	};

	Json::Value data;
	std::ifstream cfg(".\\Miku\\Models.json", std::ifstream::binary);
	cfg >> data;

	for (size_t i = 0; i < classes.size(); i++)
	{
		const char* cls = classes[i];
		Json::Value& curClass = data[cls];

		if (!curClass.size())
			continue;

		auto& dataMDL = invmanager.Models[i].models;
		auto members = curClass.getMemberNames();

		for (std::string& mdl : members)
		{
			if (!mdl.size())
				continue;

			std::string path = curClass[mdl].asString();

			dataMDL[mdl] = path;
		}
	}
}

[[inline]] void InventoryHack::ProcessWeaponHack()
{
	/// We don't want to be constantly looping through Weapon List (m_hMyWeapons)
	{
		static Timer timer;
		if (!timer.has_elapsed(flWeaponRefresh))
			return;
		timer.update();
	}

	IBaseHandle hndl;
	IBaseObject* weapon;
	IAttributeList* pList;
	WeaponInfo* infos;

	ITFPlayer* pLocalplayer = pLocalPlayer;
	TFClass cls = pLocalplayer->GetClass();
	IBaseHandle* list = pLocalplayer->GetWeaponList();

	for (uint i = 0; i < 4; i++)
	{
		hndl = list[i];
		if (!hndl.IsValid())
			return;

		weapon = reinterpret_cast<IBaseObject*>(clientlist->GetClientEntityFromHandle(hndl));
		if (!weapon)
			return;

		if (!weapon->IsBaseCombatWeapon())
			continue;

		int& id = weapon->GetItemDefinitionIndex();
		infos = g_ItemManager->GetWeaponInfo(cls, id);
		if (!infos || infos->m_bDisabled)
			continue;

		pList = weapon->GetAttributeList();

		for (auto& attr : infos->m_Attributes)
			pList->SetAttribute(attr.first, attr.second);

		if (infos->m_iNewIndex != -1)
			id = infos->m_iNewIndex;
	}
}

HookRes InventoryHack::OnFrameStageNotify(ClientFrameStage_t stage)
{
	if (BAD_LOCAL() || !g_ItemManager)
		return HookRes::Continue;

	if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && invmanager.bWeaponEnabled)
		invmanager.ProcessWeaponHack();

	return HookRes::Continue;
}


void ItemManager::InitFromFile()
{
	const std::vector<const char*> classes = {
		"Scout", "Sniper", "Soldier",
		"Demoman", "Medic", "Heavy",
		"Pyro", "Spy", "Engineer"
	};

	Json::Value data;

	WeaponInfo info;
	AttributeMap map;
	int id;

	std::ifstream cfg(m_szPath, std::ifstream::binary);
	cfg >> data;

	const char* cursub = nullptr;
	const char* attr = nullptr;

	for (size_t i = 0; i < classes.size(); i++)
	{
		const char* cls = classes[i];
		Json::Value& curClass = data[cls];
		ItemsList& list = m_Classes[i];

		if (!curClass.size())
			continue;

		auto members = curClass.getMemberNames();
		for (std::string& idx : members)
		{
			if (!idx.size())
				continue;

			id = std::stoi(idx);

			Json::Value& curID = curClass[idx];

			info.m_iNewIndex = curID.isMember("New ID") ? curID["New ID"].asInt() : id;
			info.m_szDisplayName = curID.isMember("Name") ? curID["Name"].asString() : "Unknown";
			info.m_bDisabled = curID.isMember("Disabled") ? curID["Disabled"].asBool() : false;

			Json::Value& Attributes = curID["Attributes"];

			map.clear();
			auto members = Attributes.getMemberNames();
			for (std::string& key : members)
			{
				if (map.size() >= 16)
					break;

				attr = key.c_str();
				int i = atoi(attr);

				map[i] = Attributes[attr].asFloat();
			}
			info.m_Attributes = map;

			list.insert(std::make_pair(id, info));
		}
	}
}

void ItemManager::SaveToFile()
{
	const std::vector<const char*> classes = {
		"Scout", "Sniper", "Soldier",
		"Demoman", "Medic", "Heavy",
		"Pyro", "Spy", "Engineer"
	};

	Json::Value data;
	Json::StyledWriter writer;

	WeaponInfo* infos;
	int id;

	std::ifstream cfg(m_szPath, std::ifstream::binary);
	cfg >> data;

	char sub[6], attr[6];

	for (size_t i = 0; i < classes.size(); i++)
	{
		const char* cur = classes[i];
		Msg("Class: %s\n", cur);

		Json::Value& curClass = data[cur];
		ItemsList& pList = m_Classes[i];

		for (auto& entry : pList)
		{
			id = entry.first;
			sprintf_s<6>(sub, "%i", id);
			Msg(">%s\n", sub);

			Json::Value& curID = curClass[sub];
			
			infos = &entry.second;
			curID["New ID"] = infos->m_iNewIndex;
			curID["Name"] = infos->m_szDisplayName;
			curID["Disabled"] = infos->m_bDisabled;

			Json::Value& Attributes = curID["Attributes"];

			for (auto& inner : infos->m_Attributes)
			{
				sprintf_s<6>(attr, "%i", inner.first);
				Msg(">%s : %.1f\n", attr, inner.second);
				Attributes[attr] = inner.second;
			}
		}
	}

	std::ofstream(m_szPath) << writer.write(data);
}

void ItemManager::Register(TFClass cls, int index, int new_index, std::string display_name, AttributeMap& map)
{
	ItemsList& curcls = m_Classes[cls - 1];

	auto iter = curcls.find(index);
	if (iter == curcls.end())
	{
		WeaponInfo info;

		info.m_iNewIndex = new_index;
		info.m_szDisplayName = display_name;
		info.m_Attributes = map;

		curcls.insert(iter, std::make_pair(index, info));
	}
	else {
		WeaponInfo& info = iter->second;
		if (new_index != -1)
			info.m_iNewIndex = new_index;

		if (display_name.size())
			info.m_szDisplayName = display_name;

		auto& list = iter->second.m_Attributes;
		for (auto& entry: map)
		{
			if (list.size() >= 16)
				break;
			list[entry.first] = entry.second;
		}
	}
}

void ItemManager::PrintList(TFClass cls)
{
	ItemsList& classes = this->m_Classes[cls - 1];
	for (auto& entry : classes)
	{
		Msg("ID: %i\n", entry.first);
		Msg("Disabled: %i\n", entry.second.m_bDisabled);
		Msg("New ID: %i\n", entry.second.m_iNewIndex);
		Msg("Name: %s\n", entry.second.m_szDisplayName.c_str());
		Msg("Attributes:\n");

		for (auto& attr : entry.second.m_Attributes)
		{
			Msg("			%i = %f\n", attr.first, attr.second);
		}

		Msg("----------------------------------------------------\n");
	}
}

WeaponInfo* ItemManager::GetWeaponInfo(TFClass cls, int index)
{
	if (cls < 0 || cls > 9)
		return nullptr;

	ItemsList& list = m_Classes[cls - 1];
	const auto& entry = list.find(index);

	if (entry == list.end())
		return nullptr;

	return &entry->second;
}



void InventoryHack::OnRender()
{
	if (ImGui::BeginTabItem("Inv Manager"))
	{
		ImGui::PushID("WPN");
		if (ImGui::CollapsingHeader("Weapons"))
		{
			ImGui::Text("Kill-streak Infos");
			ImGui::Checkbox("Enabled: ", bWeaponEnabled.get());

			ImGui::Separator();

			if (bWeaponEnabled && !BAD_LOCAL())
			{
				static int curClass;

				//Force Full Update
				{
					if (ImGui::Button("Force Full Update"))
					{
						ForceFullUpdate();
						if (pLocalPlayer->GetClass())
							pLocalPlayer->GetStreaks(ETFStreak::kTFStreak_Kills) = iStreaks[pLocalPlayer->GetClass() - 1];

					}
					ImGui::Dummy(ImVec2(0, 10));
				}

				//Kill Streak Infos
				{
					ImGui::Text("Kill Streak Infos");

					static bool bSelected[9];

					ImGui::Combo("Class: ", &curClass, m_szTFClasses, SizeOfArray(m_szTFClasses));

					if (ImGui::InputInt("Streaks: ", &(iStreaks[curClass]), 1, 5, ImGuiInputTextFlags_AllowTabInput))
						if (curClass == pLocalPlayer->GetClass() - 1)
							pLocalPlayer->GetStreaks(ETFStreak::kTFStreak_Kills) = iStreaks[curClass];

					ImGui::Dummy(ImVec2(0, 10));
				}

				//ItemsList
				{
					static ItemManager::ItemsList* pItems;
					static Timer item_set;
					if (item_set.trigger_if_elapsed(1200))
						pItems = &g_ItemManager->GetItemsList(static_cast<TFClass>(curClass));

					ImGui::Text("Current Items: ");
					if (ImGui::ListBoxHeader("##ItemList", ImVec2(450.0f, 0.0f)))
					{
						{
							static std::string print;
							static bool reserved = false;
							if (!reserved) { reserved = true; print.reserve(30); }

							static bool selected;

							if (pItems)
							{
								for (auto iter = pItems->begin(); iter != pItems->end(); iter++)
								{
									WeaponInfo& entry = iter->second;

									print = entry.m_bDisabled ? "(Disabled)" : "(Enabled)";
									print += "[ " + entry.m_szDisplayName + " ]";

									selected = false;
									ImGui::Selectable(print.c_str(), &selected);

									if (selected)
										entry.m_bDisabled = !entry.m_bDisabled;
								}
							}
							else ImGui::Selectable("Empty", false);
						}
						ImGui::ListBoxFooter();
					}

					ImGui::Dummy(ImVec2(0, 10));
				}

				//Save Reload
				{
					if (ImGui::Button("Save"))
						g_ItemManager->SaveToFile();
					ImGui::SameLine();
					if (ImGui::Button("Reload"))
						g_ItemManager->InitFromFile();
				}
			}
		}
		ImGui::PopID();

		ImGui::PushID("MDL");
		if (ImGui::CollapsingHeader("Models"))
		{
			ImGui::Checkbox("Enabled: ", bModelEnabled.get());
			ImGui::Separator();

			if (bModelEnabled)
			{
				static int curClass{ 0 };

				//Force Full Update
				{
					if (ImGui::Button("Force Full Update"))
						ForceFullUpdate();
					ImGui::Dummy(ImVec2(0, 10));
				}

				{
					ImGui::Combo("Class: ", &curClass, m_szTFClasses, SizeOfArray(m_szTFClasses));
					ImGui::Dummy(ImVec2(0, 10));
				}

				ImGui::Text("Models: ");
				if (ImGui::ListBoxHeader("##ModelList", ImVec2(450.0f, 0.0f)))
				{
					{
						static std::string print;
						static bool reserved = false;
						if (!reserved) { reserved = true; print.reserve(30); }

						static Timer timer_update_model[9];

						PlayerModelInfo& _Models = Models[curClass];
						{
							ImGui::Text("Current Model: %s", _Models.pos.c_str());
							for (auto& entry: _Models.models)
							{
								print = "[ " + entry.first + " ]";

								if (ImGui::Selectable(print.c_str()))
								{
									if (timer_update_model[curClass].trigger_if_elapsed(600))
									{
										const char* newMdl;

										if (_Models.pos == entry.first)
										{
											_Models.pos = "";
											newMdl = m_szActualModels[curClass + 1];
										}
										else {
											_Models.pos = entry.first;
											newMdl = _Models.GetCurrent();
										}

										player_data_model->SetDataModel(curClass + 1, newMdl);
									}
								}
							}
						}
					}
					ImGui::ListBoxFooter();
				}

				ImGui::Dummy(ImVec2(0, 10));
			}
		}
		ImGui::PopID();

		ImGui::EndTabItem();
	}
}

void InventoryHack::FireGameEvent(IGameEvent* pEvent)
{
	if (pEvent && !BAD_LOCAL())
	{
		ITFPlayer* pMe = pLocalPlayer;

		if (auto cls = pMe->GetClass())
			pMe->GetStreaks(ETFStreak::kTFStreak_Kills) = iStreaks[cls - 1];
	}
}

HookRes InventoryHack::OnLoad()
{
	g_ItemManager = new ItemManager(".\\Miku\\Items.json");
	eventmanager->AddListener(this, "localplayer_respawn", false);

	player_data_model = new _PlayerClassData;

	InitModels();
	PrecacheDataModel();

	return HookRes::Continue;
}

HookRes InventoryHack::OnUnload()
{
	eventmanager->RemoveListener(this);

	delete g_ItemManager;
	delete player_data_model;
	return HookRes::Continue;
}


HookRes InventoryHack::OnLevelInit()
{
	PrecacheDataModel();
	return HookRes::Continue;
}


void InventoryHack::JsonCallback(Json::Value& json, bool read)
{
	Json::Value& Models = json["Models"];
	if (read)
	{
		std::string curName;

		for (uint8_t i = 0; i < SizeOfArray(m_szTFClasses); i++)
		{
			PROCESS_JSON_READ(Models, m_szTFClasses[i], String, curName);
			this->Models[i].pos = curName;
		}
	}
	else {
		for (uint8_t i = 0; i < SizeOfArray(m_szTFClasses); i++)
			PROCESS_JSON_WRITE(Models, m_szTFClasses[i], this->Models[i].pos);
	}
}


//HAT_COMMAND(save_items, "Saves items")

HAT_COMMAND(force_fullupdate, "CBaseClientState::ForceFullUpdate")
{
	ForceFullUpdate();
}

HAT_COMMAND(print_weapons, "Print Weapons attributes manager")
{
	for(size_t i = 0; i < 9; i++)
		g_ItemManager->PrintList(static_cast<TFClass>(i));
}

HAT_COMMAND(print_models, "Print Models")
{
	auto& Models = invmanager.Models.get();
	for (int i = 0; i < invmanager.Models.size(); i++)
	{
		Msg("Class: %i\n", i);
		Msg("Current: %s\n\tModels: \n", Models[i].pos.c_str());
		for (auto& entry : Models[i].models)
		{
			Msg("\t\t\"%s: %s\"\n", entry.first.c_str(), entry.second.c_str());
		}
	}
}

HAT_COMMAND(custom_models, "")
{
	if (args.ArgC() < 3)
		REPLY_TO_TARGET(return, "Usage: %scustom_models <class>:int <name>:string\n", CMD_TAG);

	int cls = atoi(args[1]);
	auto& Models = invmanager.Models[cls - 1];

	std::string set_cur = args[2] + std::string(args[3]);

	Models.pos = set_cur;

	const char* mdl = Models.GetCurrent();
	player_data_model->SetDataModel(cls, mdl);
}


#include "../../Interfaces/RecipientFilter.h"
#include "../../Interfaces/IVEClientTrace.h"

HAT_COMMAND(client_spray, "call TE_PlayerDecal")
{
	auto fn =	reinterpret_cast<void(*)(IRecipientFilter&, float, const Vector*, int, int)>(
				Library::clientlib.FindPattern("TE_PlayerDecal")
		);

	CRecipientFilter filter;
	filter.AddAllPlayers();

	ITFPlayer* pEnt = pLocalPlayer;
	int idx = pEnt->entindex();

	Vector begin = pEnt->LocalEyePosition();
	Vector fwd; AngleVectors(pEnt->GetAbsAngles(), &fwd);
	Vector end = begin + fwd * 24.0;

	Ray_t ray; ray.Init(begin, end);
	CTraceFilterWorldOnly pworld;
	trace_t res;
	clienttrace->TraceRay(ray, MASK_PLAYERSOLID | CONTENTS_GRATE, &pworld, &res);

	if (res.DidHit())
	{
		Vector pos = res.endpos;
		fn(filter, 2.0, &pos, idx, 0);
	}
}


void ForceFullUpdate() 
{
	if (BAD_LOCAL())
		return;

	void* ClientState = reinterpret_cast<void*>(Library::enginelib.FindPattern("pBaseClientStatePtr"));

	using ForceFullUpdateFn = void(__thiscall*)(void*);
	ForceFullUpdateFn forceFullUpdate = reinterpret_cast<ForceFullUpdateFn>(Library::enginelib.FindPattern("ClientState::ForceFullUpdate"));
	forceFullUpdate(ClientState);
}