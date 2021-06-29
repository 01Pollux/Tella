#include "Inventory_Skins.hpp"
#include "Hooks/FrameStageNotify.hpp"
#include <fstream>

static Inventory::SkinManager skin_mgr;

Inventory::SkinManager::SkinManager()
{
	M0EventManager::AddListener(
		EVENT_KEY_LOAD_DLL,
		[this](M0EventData*)
		{
			ItemMgr.init();

			M0HookManager::Policy::FrameStageNotify fsn_hook(true);
			fsn_hook->AddPreHook(HookCall::Any, std::bind(&SkinManager::FrameStageNotify, this, std::placeholders::_1));

			Interfaces::GameEventMgr->AddListener(this, "localplayer_respawn", GAMEEVENT_CLIENT_SIDE);
		},
		EVENT_NULL_NAME
	);

	M0EventManager::AddListener(
		EVENT_KEY_UNLOAD_DLL,
		[this](M0EventData*)
		{
			Interfaces::GameEventMgr->RemoveListener(this);
		},
		EVENT_NULL_NAME
	);

	Streaks.fill(10);
}

void Inventory::SkinManager::ProcessWeaponHack(ITFPlayer pMe)
{
	{
		static AutoCTimer<2.5f> timer_ignore_update;
		if (!timer_ignore_update.trigger_if_elapsed())
			return;
	}

	TFClass cls = pMe->Class;
	if (cls == TFClass::Unknown)
		return;

	cls = static_cast<TFClass>(static_cast<size_t>(cls) - 1);

	for (size_t i = 0; i <= 4; i++)
	{
		IBaseWeapon weapon(pMe->MyWeapons[i]);

		if (!weapon || !weapon->IsBaseCombatWeapon())
			return;

		int& id = weapon->ItemDefinitionIndex;
		WeaponsInfo* info = ItemMgr.get(cls, id);
		if (!info || info->IsDisabled)
			continue;

		IAttributeList* pAttrList = weapon->AttributeList.data();

		for (const auto [attribute, value] : info->Attributes)
			pAttrList->SetAttribute(attribute, value);
	
		if (info->NewIndex != -1)
			id = info->NewIndex;
	}
}

MHookRes Inventory::SkinManager::FrameStageNotify(ClientFrameStage stage)
{
	if (!Enable || ITFPlayerInternal::BadLocal())
		return { };

	ILocalPlayer pMe;
	if (stage == ClientFrameStage::PostDataUpdatePre)
		ProcessWeaponHack(pMe);

	return { };
}

void Inventory::SkinManager::FireGameEvent(IGameEvent* Event)
{
	ILocalPlayer pMe;
	if (const TFClass cls = pMe->Class; cls != TFClass::Unknown)
		pMe->Streaks->set(TFStreakType::Kills, Streaks[static_cast<size_t>(cls) - 1]);
}


/*
* Structure of Items.json:
{
"Demoman" : null,
"<Class>" : {		
	"141" : {		//Weapon item def index
		"Attributes" : {	//"Attribute idx" : value
			"1007" : 1,
			"2013" : 2004,
			"2014" : 4,
			"2025" : 3,
			"719" : 49
		},
		"Disabled" : false,	// true if its active, false otherwise
		"Name" : "Pro KS Fes Frontier", //display name
		"New ID" : 1004	// new item index, -1 by default
		},
	}
}
*/
void Inventory::ItemsInfo::init()
{
	for (ItemMap& map : ClassesMap)
		map.clear();

	Json::Value data;
	std::ifstream cfg(path(), std::ios::binary);
	cfg >> data;

	for (size_t i = 0; i < SizeOfArray(TFClassNames); i++)
	{
		const char* cls = TFClassNames[i];
		Json::Value& curClass = data[cls];

		if (!curClass.size())
			continue;

		ItemMap& cur_map = ClassesMap[i];
		
		for (auto members = curClass.getMemberNames(); 
			 const std::string& idx : members)
		{
			if (!idx.size())
				continue;

			Json::Value& curID = curClass[idx];

			WeaponsInfo info;
			info.NewIndex		= curID.isMember("New ID")	? curID["New ID"].asInt()	: -1;
			info.DisplayName	= curID.isMember("Name")	? curID["Name"].asString()	: "Unknown";
			info.IsDisabled		= curID.isMember("Disabled")? curID["Disabled"].asBool(): false;

			Json::Value& Attributes = curID["Attributes"];

			for (auto attributes = Attributes.getMemberNames(); 
				const std::string& key : attributes)
			{
				if (info.Attributes.size() >= 16)
					break;

				const int x = std::stoi(key);
				info.Attributes[x] = Attributes[key].asFloat();
			}

			const int x = std::stoi(idx);
			cur_map[x] = std::move(info);
		}
	}
}

void Inventory::ItemsInfo::save()
{
	Json::Value data;
	Json::StyledWriter writer;

	std::fstream cfg(path(), std::ios::binary | std::ios::in | std::ios::out);

	IFormatterSV sub("{:6d}"sv);
	
	for (size_t i = 0; i < SizeOfArray(TFClassNames); i++)
	{
		const char* cur = TFClassNames[i];

		Json::Value& curClass = data[cur];

		for (ItemMap& curmap = ClassesMap[i]; 
			 auto& entry : curmap)
		{
			Json::Value& curID = curClass[static_cast<const std::string&>(sub(entry.first))];

			WeaponsInfo* infos = &entry.second;
			curID["New ID"] = infos->NewIndex;
			curID["Name"] = infos->DisplayName;
			curID["Disabled"] = infos->IsDisabled;

			Json::Value& Attributes = curID["Attributes"];

			for (auto& inner : infos->Attributes)
				Attributes[static_cast<const std::string&>(sub(inner.first))] = inner.second;
		}
	}

	cfg << writer.write(data);
}

void Inventory::ItemsInfo::insert(TFClass cls, int index, WeaponsInfo&& info, bitmask::mask<WeaponsInfo::InsertFlag> insert_flag)
{
	using InsertFlag = WeaponsInfo::InsertFlag;
	ItemMap& curmap = ClassesMap[static_cast<size_t>(cls) - 1];

	const auto iter = curmap.find(info.NewIndex);
	if (iter == curmap.end())
		curmap[index] = std::move(info);
	else
	{
		WeaponsInfo& cur_info = iter->second;

		if (insert_flag.test(InsertFlag::OverrideIndex))
			cur_info.NewIndex = info.NewIndex;

		if (insert_flag.test(InsertFlag::OverrideState))
			cur_info.IsDisabled = info.IsDisabled;
		
		if (insert_flag.test(InsertFlag::OverrideName))
			cur_info.IsDisabled = info.IsDisabled;

		if (bool override_map = insert_flag.test(InsertFlag::OverrideMap);
			insert_flag.test(InsertFlag::AddMap))
		{
			for (auto& attributes = iter->second.Attributes; 
				 auto& entry : info.Attributes)
			{
				if (attributes.size() >= 16)
					break;

				auto cur_att = attributes.find(entry.first);
				if (cur_att != attributes.end())
				{
					if (override_map)
						cur_att->second = entry.second;
				}
				else attributes[entry.first] = entry.second;
			}
		}
		else if (override_map)
			iter->second.Attributes = std::move(info.Attributes);
	}
}


void Inventory::SkinManager::_OnRender()
{
	if (ImGui::CollapsingHeader("Weapons"))
	{
		ImGui::Text("Kill-streak Infos");
		ImGui::Checkbox("Enabled:", Enable.data());
		ImGui::SameLineHelp(Enable);

		ImGui::Separator();

		ILocalPlayer pMe;
		if (Enable && pMe)
		{
			//Force Full Update
			{
				if (ImGui::Button("Force Full Update"))
				{
					Interfaces::ForceClientFullUpdate();
					if (TFClass cls = pMe->Class; cls != TFClass::Unknown)
						pMe->Streaks->set(TFStreakType::Kills, Streaks[static_cast<size_t>(cls) - 1]);
					
				}
				ImGui::Dummy(ImVec2(0, 10));
			}

			static int curClass;
			//Kill Streak Infos
			{
				ImGui::Text("Kill Streak Infos");

				static bool bSelected[9];

				ImGui::Combo("Class: ", &curClass, TFClassNames, SizeOfArray(TFClassNames));

				if (ImGui::InputInt("Streaks: ", &(Streaks[curClass]), 1, 5, ImGuiInputTextFlags_AllowTabInput))
					if (TFClass cls = pMe->Class; cls != TFClass::Unknown && curClass == static_cast<int>(cls) - 1)
						pMe->Streaks->set(TFStreakType::Kills, Streaks[curClass]);

				ImGui::Dummy(ImVec2(0, 10));
			}

			//ItemsList
			{
				static ItemsInfo::ItemMap* pItems;
				static ITimer item_set;
				if (item_set.trigger_if_elapsed(1.2f))
					pItems = &ItemMgr.get(static_cast<TFClass>(curClass));

				ImGui::Text("Current Items: ");
				if (ImGui::BeginListBox("##ItemList", ImVec2(450.0f, 0.0f)))
				{
					{
						if (pItems)
						{
							for (auto iter = pItems->begin(); iter != pItems->end(); iter++)
							{
								WeaponsInfo& entry = iter->second;

								std::string to_print = std::format("({}) [{}]", entry.IsDisabled ? "Disabled" : "Enabled", entry.DisplayName);

								bool selected = false;
								ImGui::Selectable(to_print.c_str(), &selected);

								if (selected)
									entry.IsDisabled = !entry.IsDisabled;
							}
						}
						else ImGui::Selectable("Empty", false);
					}
					ImGui::EndListBox();
				}

				ImGui::Dummy(ImVec2(0, 10));
			}

			//Save Reload
			{
				if (ImGui::Button("Save"))
					ItemMgr.save();
				ImGui::SameLine();
				if (ImGui::Button("Reload"))
					ItemMgr.init();
			}
		}
	}
}

void Inventory::SkinManager::OnRender()
{
	skin_mgr._OnRender();
}