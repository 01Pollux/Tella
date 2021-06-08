#pragma once

#include "Helper/Config.hpp"
#include "Helper/EnumClass.hpp"

#include "Helper/Format.hpp"

#define ESP_RED_TEAM "Red"
#define ESP_BLU_TEAM "Blu"

#define ESP_PLR_MGR	"Player Manager"
#define ESP_BLD_MGR	"Building Manager"
#define ESP_OBJ_MGR	"Object Manager"

#define ESP_SECTION_PLAYER(...)		std::format("ESP.Player Manager.{}.{}"sv, is_red ? ESP_RED_TEAM : ESP_BLU_TEAM, __VA_ARGS__).c_str()
#define ESP_SECTION_BUILDING(...)	std::format("ESP.Building Manager.{}.{}"sv, is_red ? ESP_RED_TEAM : ESP_BLU_TEAM, __VA_ARGS__).c_str()
#define ESP_SECTION_OBJECT(...)		"ESP.Object Manager." ## __VA_ARGS__


namespace ESPInfo
{
	enum class DrawMode : char8_t
	{
		None,
		Box = 0b001,
		BoxOutline = 0b010,
	};
}
using ESPDrawMode = ESPInfo::DrawMode;

M0CONFIG_BEGIN

class ESPMode final : public Custom<ESPDrawMode, false>
{
	M0CONFIG_INHERIT_FROM(ESPMode, ESPDrawMode, Custom<ESPDrawMode, false>);

public:
	using Custom<ESPDrawMode, false>::operator=;

	M0CONFIG_IMPL_MAP(ESPDrawMode)
	{
		return
		{
			{ "None", ESPDrawMode::None },
			{ "Box", ESPDrawMode::Box },
			{ "Box Outline", ESPDrawMode::BoxOutline},
		};
	}
};

M0CONFIG_END

namespace ESPInfo
{
	constexpr size_t TEAM_RED = 1, TEAM_BLUE = 0, MAX_TEAMS = 2;

	struct Shared
	{
		M0Config::Bool Enable{ false, M0CONFIG_NULL_DESCRIPTION };

		M0Config::U8Color DrawColor{ { 255, 255, 255, 255 }, "ESP's draw color" };
		M0Config::U8Color FontColor{ { 255, 255, 255, 255 }, "ESP's text color" };

		M0Config::Int MaxDistance{ 1024, "ESP's max draw distance" };

		M0Config::ESPMode DrawMode{ M0Config::ESPMode::type::Box, "ESP's draw mode" };

		M0Config::Bool DrawName{ false, M0CONFIG_NULL_DESCRIPTION };
		M0Config::Bool DrawDistance{ false, M0CONFIG_NULL_DESCRIPTION };
	};

	struct Player : public Shared
	{
		M0Config::Bool DrawClass{ false, "Draw player tf-class" };
		M0Config::Bool DrawCond{ false,	"Draw some player's conds" };
		M0Config::Bool DrawUberPerc{ false,	"Draw how much uber a medic has" };
		M0Config::Bool IgnoreCloak{ true, "Don't draw cloaked spies" };
		M0Config::Bool DrawHealth{ true, "Draw how much health a player has" };
		M0Config::Bool DrawTeam{ false,	"Draw player's team" };

		Player(bool is_red) noexcept;
	};

	struct Building : public Shared
	{
		M0Config::Bool DrawOwner{ false, "Draw building's owner" };
		M0Config::Bool DrawAmmo{ false,	"Draw building's ammo" };
		M0Config::Bool DrawLevel{ false, "Draw building's level" };
		M0Config::Bool DrawBState{ false, "Draw building's state" };
		M0Config::Bool DrawHealth{ false, "Draw building's health" };
		M0Config::Bool DrawTeam{ false, "Draw building's team" };

		Building(bool is_red) noexcept;
	};

	struct Object : public Shared
	{
		M0Config::Bool DrawPacks{ false, "Draw Packs" };
		M0Config::Bool DrawRockets{ false,"Draw Rockets" };
		M0Config::Bool DrawPipes{ false, "Draw Pipe bomb" };
		M0Config::Bool DrawStickies{ false, "Draw stickie bomb"};

		Object() noexcept;
	};
}

