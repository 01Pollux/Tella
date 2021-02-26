#include "../Interfaces/HatCommand.h"
#include "Inventory.h"

#include "../Helpers/Timer.h"
#include "../Helpers/json.h"
#include "../Helpers/Commons.h"

#include <fstream>


constexpr std::chrono::milliseconds msWeaponRefresh{ 1200 };

InventoryHack invmanager;
IItemManager* g_ItemManager = nullptr;

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
	_PlayerClassData() noexcept
	{
		pFn = reinterpret_cast<GetClassDataFn>(Library::clientlib.FindPattern("GetPlayerClassData"));
	}

	~_PlayerClassData()
	{
		Shutdown();
	}

	void SetDataModel(uint8_t cls, const char* newMdl) noexcept;

	void Reset(uint8_t i) noexcept
	{
		SetDataModel(i, NULL);
	}

	void Shutdown() noexcept
	{
		for (uint8_t i = 1; i <= 9; i++)
			this->Reset(i);
	}

public:
	_PlayerClassData(const _PlayerClassData&)			 = delete;
	_PlayerClassData& operator=(const _PlayerClassData&) = delete;
	_PlayerClassData(_PlayerClassData&&)				 = delete;
	_PlayerClassData& operator=(_PlayerClassData&&)		 = delete;
} static* player_data_model;

void _PlayerClassData::SetDataModel(uint8_t cls, const char* newMdl) noexcept
{
	uintptr_t* data = pFn(cls);
	
	char* szModelName = reinterpret_cast<char*>(&data[Offsets::ClientDLL::PlayerData_t::OffsetToModel]);

	if (!newMdl)
		newMdl = m_szActualModels[cls];

	strncpy(szModelName, newMdl, Offsets::ClientDLL::PlayerData_t::SizeOfModel);
}


void PrecacheDataModel()
{
	if (!nstcontainer)
		return;

	auto modelprecache = nstcontainer->FindTable("modelprecache");
	if (!modelprecache)
		return;

	constexpr size_t count = invmanager.Models.size();
	for (size_t i = 0; i < count; i++)
	{
		auto& data = invmanager.Models[i].models;
		for (auto&& [name, path] : data)
		{
			if (modelprecache->AddString(false, path.c_str()) == INVALID_STRING_INDEX)
				MIKUDebug::LogCritical(fmt::format("[Model] Failed to Precache model: \"{}\":\"{}\"", name, path));
		}
	}
}

void ForceFullUpdate();

static void InitModels()
{
	constexpr const char* classes[] = {	//	TODO Maybe use m_szTFClasses?
		"Scout", "Sniper", "Soldier",
		"Demoman", "Medic", "Heavy",
		"Pyro", "Spy", "Engineer"
	};

	Json::Value data;
	std::ifstream cfg(".\\Miku\\Models.json", std::ifstream::binary);
	cfg >> data;

	for (size_t i = 0; i < SizeOfArray(classes); i++)
	{
		const char* cls = classes[i];
		Json::Value& curClass = data[cls];

		if (!curClass.size())
			continue;

		auto& dataMDL = invmanager.Models[i].models;
		const auto& members = curClass.getMemberNames();

		for (const std::string& mdl : members)
		{
			if (mdl.size())
				dataMDL[mdl] = curClass[mdl].asString();
		}
	}
}

void InventoryHack::ProcessWeaponHack()
{
	/// We don't want to be constantly looping through Weapon List (m_hMyWeapons)
	{
		static Timer timer;
		if (!timer.has_elapsed(msWeaponRefresh))
			return;
		timer.update();
	}

	ITFPlayer* pLocalplayer = ILocalPtr();
	TFClass cls = pLocalplayer->GetClass();
	IBaseHandle* list = pLocalplayer->GetWeaponList();

	for (uint i = 0; i < 4; i++)
	{
		IBaseHandle& hndl = list[i];
		if (!hndl.IsValid())
			return;

		IBaseObject* weapon = ::GetIBaseObject(hndl);
		if (!weapon)
			return;

		if (!weapon->IsBaseCombatWeapon())
			continue;

		int& id = weapon->GetItemDefinitionIndex();
		WeaponInfo* infos = g_ItemManager->GetWeaponInfo(cls, id);
		if (!infos || infos->m_bDisabled)
			continue;

		IAttributeList* pList = weapon->GetAttributeList();

		for (auto&& [attribute, value] : infos->m_Attributes)
			pList->SetAttribute(attribute, value);

		if (infos->m_iNewIndex != -1)
			id = infos->m_iNewIndex;
	}
}


void IItemManager::InitFromFile()
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

	std::ifstream cfg(IItemManager::GetPath(), std::ifstream::binary);
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

void IItemManager::SaveToFile()
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

	std::ifstream cfg(IItemManager::GetPath(), std::ifstream::binary);
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

	std::ofstream(IItemManager::GetPath()) << writer.write(data);
}

void IItemManager::Register(TFClass cls, int index, int new_index, std::string display_name, AttributeMap& map)
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

WeaponInfo* IItemManager::GetWeaponInfo(TFClass cls, int index) noexcept
{
	if (cls < 0 || cls > 9)
		return nullptr;

	ItemsList& list = m_Classes[cls - 1];
	return &list[index];
}



void InventoryHack::OnRender()
{
	using namespace std::chrono;

	if (ImGui::BeginTabItem("Inv Manager"))
	{
		ImGui::PushID("WPN");
		if (ImGui::CollapsingHeader("Weapons"))
		{
			ImGui::Text("Kill-streak Infos");
			ImGui::Checkbox("Enabled: ", &bWeaponEnabled);

			ImGui::Separator();

			if (bWeaponEnabled && !BadLocal())
			{
				static int curClass;

				//Force Full Update
				{
					if (ImGui::Button("Force Full Update"))
					{
						ForceFullUpdate();
						if (ITFPlayer* pMe = ILocalPtr(); pMe->GetClass())
							pMe->GetStreaks(ETFStreak::kTFStreak_Kills) = iStreaks[pMe->GetClass() - 1];

					}
					ImGui::Dummy(ImVec2(0, 10));
				}

				//Kill Streak Infos
				{
					ImGui::Text("Kill Streak Infos");

					static bool bSelected[9];

					ImGui::Combo("Class: ", &curClass, m_szTFClasses, SizeOfArray(m_szTFClasses));

					if (ImGui::InputInt("Streaks: ", &(iStreaks[curClass]), 1, 5, ImGuiInputTextFlags_AllowTabInput))
						if (ITFPlayer* pMe = ILocalPtr(); curClass == pMe->GetClass() - 1)
							pMe->GetStreaks(ETFStreak::kTFStreak_Kills) = iStreaks[curClass];

					ImGui::Dummy(ImVec2(0, 10));
				}

				//ItemsList
				{
					static IItemManager::ItemsList* pItems;
					static Timer item_set;
					if (item_set.trigger_if_elapsed(1200ms))
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
			ImGui::Checkbox("Enabled: ", &bModelEnabled);
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
									if (timer_update_model[curClass].trigger_if_elapsed(600ms))
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
	if (pEvent && !BadLocal())
	{
		ITFPlayer* pMe = ILocalPtr();

		if (auto cls = pMe->GetClass())
			pMe->GetStreaks(ETFStreak::kTFStreak_Kills) = iStreaks[cls - 1];
	}
}

void InventoryHack::OnLoadDLL()
{
	g_ItemManager = new IItemManager();
	eventmanager->AddListener(this, "localplayer_respawn", false);

	player_data_model = new _PlayerClassData;

	InitModels();
	PrecacheDataModel();

	using namespace IGlobalVHookPolicy;
	auto fsn = FrameStageNotify::Hook::QueryHook(FrameStageNotify::Name);
	fsn->AddPreHook(HookCall::Late,
		[this](ClientFrameStage_t stage)
		{
			if (BadLocal() || !g_ItemManager)
				return HookRes::Continue;

			if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && bWeaponEnabled)
				ProcessWeaponHack();

			return HookRes::Continue;
		});

	auto li = LevelInit::Hook::QueryHook(LevelInit::Name);
	li->AddPostHook(HookCall::Any,
		[](const char*)
		{
			PrecacheDataModel();
			return HookRes::Continue;
		});
}

void InventoryHack::OnUnloadDLL()
{
	eventmanager->RemoveListener(this);

	delete g_ItemManager;
	delete player_data_model;
}


void InventoryHack::JsonCallback(Json::Value& json, bool read)
{
	Json::Value& Models = json["Models"];
	if (read)
	{
		std::string curName;

		for (uint8_t i = 0; i < SizeOfArray(m_szTFClasses); i++)
		{
			PROCESS_JSON_READ(Models, m_szTFClasses[i], String, curName); else curName = "";
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

HAT_COMMAND_NOEXCEPT(print_models, "Print Models")
{
	auto& Models = *invmanager.Models;
	for (int i = 0; i < Models.size(); i++)
	{
		Msg("Class: %i\n", i);
		Msg("Current: %s\n\tModels: \n", Models[i].pos.c_str());
		for (const auto& [name, path] : Models[i].models)
		{
			Msg("\t\t\"%s: %s\"\n", name.c_str(), path.c_str());
		}
	}
}

HAT_COMMAND(custom_models, "")
{
	if (args.ArgC() < 3)
		REPLY_TO_TARGET(return, "Usage: %scustom_models <class>:int <name>:string\n", CMD_TAG);

	int cls = atoi(args[1]);
	auto& Models = invmanager.Models[cls - 1];

	Models.pos = args[2] + std::string(args[3]);

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

	ITFPlayer* pEnt = ILocalPtr();
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
	if (BadLocal())
		return;

	class MyEmptyClass { };
	MyEmptyClass* MyClientState = reinterpret_cast<MyEmptyClass*>(Library::enginelib.FindPattern("pBaseClientStatePtr"));

	union
	{
		void(MyEmptyClass::* fn)();
		uintptr_t ptr;
	} u;
	u.ptr = Library::enginelib.FindPattern("ClientState::ForceFullUpdate");

	(MyClientState->*u.fn)();
}