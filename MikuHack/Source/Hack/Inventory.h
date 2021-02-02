#pragma once

#include "../Main.h"
#include "../Helpers/VTable.h"
#include "../Helpers/sdk.h"
#include "../Interfaces/CAttributes.h"
#include <unordered_map>

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

class InventoryHack : public MenuPanel, public IGameEventListener2
{
public: //InventoryHack
	AutoBool bWeaponEnabled{ "InventoryHack::bWeaponEnabled", false };
	AutoBool bModelEnabled{ "InventoryHack::bModelEnabled", false };
	AutoArray<int, 9>			 iStreaks{ "InventoryHack::iStreaks", 10 };
	AutoArray<PlayerModelInfo, 9>Models{ "InventoryHack::Models" };

public:
	InventoryHack()
	{
		using namespace IGlobalEvent;
		using std::bind;

		LoadDLL::Hook::Register(bind(&InventoryHack::OnLoad, this));
		UnloadDLL::Hook::Register(bind(&InventoryHack::OnUnload, this));
		LevelInit::Hook::Register(bind(&InventoryHack::OnLevelInit, this));
		FrameStageNotify::Hook::Register(bind(&InventoryHack::OnFrameStageNotify, this, std::placeholders::_1));
	};

	HookRes OnLoad();
	HookRes OnUnload();

	HookRes OnLevelInit();

	HookRes OnFrameStageNotify(ClientFrameStage_t);
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

class ItemManager
{
public:
	using AttributeMap = std::unordered_map<int, float>;
	using ItemsList = std::unordered_map<int, WeaponInfo>;

	ItemManager() = delete;
	ItemManager(const char* path): m_szPath(path)
	{
		for (int i = 0; i < 9; i++)
			(m_Classes[i]).clear();

		InitFromFile();
	}

	ItemsList& GetItemsList(TFClass cls) { return m_Classes[cls]; }
	WeaponInfo* GetWeaponInfo(TFClass cls, int index);

	void InitFromFile();
	void SaveToFile();

	void Register(TFClass cls, int index, int new_index, std::string display_name, AttributeMap& map);
	void PrintList(TFClass cls);

private:
	ItemsList m_Classes[9];
	std::string m_szPath;
};

extern ItemManager* g_ItemManager;
