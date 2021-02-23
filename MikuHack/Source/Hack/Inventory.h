#pragma once

#include "../Main.h"
#include "../GlobalHook/load_routine.h"
#include "../GlobalHook/vhook.h"

#include "../Interfaces/CAttributes.h"
#include "../Interfaces/CBaseEntity.h"

#include <unordered_map>
#include <igameevents.h>

struct PlayerModelInfo
{
	std::string pos = "";
	std::unordered_map<std::string, std::string> models;

	const char* GetCurrent()
	{
		if (pos.size())
			return models[pos].c_str();
		else return nullptr;
	}

	int GetModelIndex()
	{
		if (pos.size())
			return modelinfo->GetModelIndex(models[pos].c_str());
		else return 0;
	}
};

class InventoryHack : public MenuPanel, public IMainRoutine, public IGameEventListener2
{
public: //InventoryHack
	AutoBool bWeaponEnabled{ "InventoryHack::bWeaponEnabled", false };
	AutoBool bModelEnabled{ "InventoryHack::bModelEnabled", false };
	IAutoArray<int, 9>				iStreaks{ "InventoryHack::iStreaks", { 10, 10, 10, 10, 10, 10, 10, 10, 10 } };
	IAutoArray<PlayerModelInfo, 9>	Models{ "InventoryHack::Models" };

public:
	void OnLoadDLL() override;
	void OnUnloadDLL() override;

	void ProcessWeaponHack();

public:	//MenuPanel
	void OnRender() override;
	void JsonCallback(Json::Value& json, bool read) override;

public:	//IGameEventListener2
	void FireGameEvent(IGameEvent* pEvent) override;
};


struct WeaponInfo
{
	std::unordered_map<int, float> m_Attributes;
	int m_iNewIndex = -1;
	bool m_bDisabled = false;
	std::string m_szDisplayName = "Unknown";
};

class IItemManager
{
public:
	using AttributeMap = std::unordered_map<int, float>;
	using ItemsList = std::unordered_map<int, WeaponInfo>;

	IItemManager()
	{
		for (int i = 0; i < 9; i++)
			(m_Classes[i]).clear();
		InitFromFile();
	}

	ItemsList& GetItemsList(TFClass cls) noexcept { return m_Classes[cls]; }
	WeaponInfo* GetWeaponInfo(TFClass cls, int index) noexcept;

	void InitFromFile();
	void SaveToFile();

	void Register(TFClass cls, int index, int new_index, std::string display_name, AttributeMap& map);

	static constexpr const char* GetPath()
	{
		return ".\\Miku\\Items.json";
	}

private:
	ItemsList m_Classes[9];
};

extern IItemManager* g_ItemManager;
