#pragma once

#include "Cheats/Main.hpp"
#include "Library/Lib.hpp"

namespace Inventory
{
	struct PlayerModelInfo
	{
		using modelmap = std::map<std::string, std::string>;

		std::string Current;
		modelmap	ModelMap;

		/// <summary> Check if ::Current model == incoming model /// </summary>
		/// 
		/// <param name="str"> model to set </param>
		void set_or_toggle(const std::string& str)
		{
			if (Current[0] == str[0] && Current == str)
				Current = "";
			else
				Current = str;
		}

		std::string_view get() noexcept
		{
			return Current.empty() ? std::string_view{ nullptr, 0 } : ModelMap.find(Current)->second;
		}
	};

	class PlayerDataModel
	{
	public:
		void init();

		void set(TFClass playercls, std::string_view newmdl) noexcept;
		void reset(TFClass playercls) noexcept { set(playercls, { nullptr, 0 }); }
		void reset() noexcept
		{
			for (size_t i = 0; i < static_cast<size_t>(TFClass::Count); i++)
				reset(static_cast<TFClass>(i));
		}

	private:
		IFuncThunk<void**, uint32_t> GetPlayerClassData{ };
	};

	static constexpr std::string_view ActualTFModels[] =
	{
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

	static_assert(SizeOfArray(ActualTFModels) == static_cast<size_t>(TFClass::Engineer) + 1);

	class ModelManager
	{
	public:
		ModelManager();

		static void OnRender();

	private:
		void PrecacheModels();

		M0Config::Bool Enable{
			"Inventory Manager.Models.Enable",
			false,
			"Enable Custom Models"
		};

		static constexpr size_t sizeof_Models = static_cast<size_t>(TFClass::Count) - 1;
		std::array<PlayerModelInfo, sizeof_Models> Models;

		PlayerDataModel DataModel;
	};
}