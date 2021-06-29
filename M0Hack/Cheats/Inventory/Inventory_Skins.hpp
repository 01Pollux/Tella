#pragma once

#include "Cheats/Main.hpp"
#include "GlobalHook/vhook.hpp"
#include "cdll_int.hpp"
#include "GameEvent.hpp"

namespace Inventory
{
	struct WeaponsInfo
	{
		using AttributeMap = std::map<int, float>;

		AttributeMap	Attributes;
		int				NewIndex = -1;
		bool			IsDisabled = false;
		std::string		DisplayName = "Unknown";

		enum class InsertFlag
		{
			OverrideIndex,
			OverrideName,
			OverrideState,
			AddMap,			// add items to map
			OverrideMap,	// std::move the new map to the old one

			_Highest_Enum,
		};
	};

	class ItemsInfo
	{
	public:
		using ItemMap = std::map<int, WeaponsInfo>;

		void init();
		void save();
		
		void insert(TFClass cls, int index, WeaponsInfo&& info, bitmask::mask<WeaponsInfo::InsertFlag> info_flag);
		static constexpr const char* path() { return "./Miku/Items.json"; }

		ItemMap& get(TFClass cls) noexcept { return ClassesMap[static_cast<size_t>(cls)]; }
		WeaponsInfo* get(TFClass cls, int index) noexcept
		{
			ItemMap& map = get(cls);
			auto iter = map.find(index);
			return iter == map.end() ? nullptr : &iter->second;
		}

	private:
		ItemMap ClassesMap[9];
	};

	class SkinManager : public IGameEventListener
	{
	public:
		SkinManager();

		static void OnRender();

		MHookRes FrameStageNotify(ClientFrameStage stage);

		void FireGameEvent(IGameEvent*) final;

	private:
		void _OnRender();

		void ProcessWeaponHack(ITFPlayer);

		ItemsInfo ItemMgr;

		M0Config::Bool Enable{ 
			"Inventory Manager.Weapons.Enable", 
			false,
			"Enable custom weapons"
		};

		std::array<int, 9> Streaks;
	};
}
