#include "esp.hpp"

IESPHack::IESPHack()
{
	DrawData.reserve(32U);

	M0EventManager::AddListener(
		EVENT_KEY_LOAD_DLL,
		[this](M0EventData*)
		{
			M0HookManager::Policy::PaintTraverse painttraverse(true);
			painttraverse->AddPostHook(HookCall::Late, std::bind(&IESPHack::OnPaintTraverse, this, std::placeholders::_1));
		},
		EVENT_NULL_NAME
	);

	M0EventManager::AddListener(
		EVENT_KEY_RENDER_MENU,
		std::bind(&IESPHack::OnRender, this),
		"ESP::OnRenderMenu"
	);
}

static IESPHack esp_hack;


ESPInfo::Player::Player(bool is_red) noexcept
{
	DrawClass.export_var(ESP_SECTION_PLAYER("Draw Class"));
	DrawCond.export_var(ESP_SECTION_PLAYER("Draw Conditions"));
	DrawUberPerc.export_var(ESP_SECTION_PLAYER("Draw UberCharge"));
	IgnoreCloak.export_var(ESP_SECTION_PLAYER("Ignore cloaked players"));
	DrawHealth.export_var(ESP_SECTION_PLAYER("Draw Health"));
	DrawTeam.export_var(ESP_SECTION_PLAYER("Draw Team"));

	DrawColor.export_var(ESP_SECTION_PLAYER("Draw Color"));
	FontColor.export_var(ESP_SECTION_PLAYER("Font Color"));
	MaxDistance.export_var(ESP_SECTION_PLAYER("Max Distance"));
	DrawMode.export_var(ESP_SECTION_PLAYER("Draw Mode"));

	Enable.set_description("Draw player's ESP");
	Enable.export_var(ESP_SECTION_PLAYER("Enable"));

	DrawName.set_description("Draw player's name");
	DrawName.export_var(ESP_SECTION_PLAYER("Draw Name"));

	DrawDistance.set_description("Draw distance between localplayer and the player");
	DrawDistance.export_var(ESP_SECTION_PLAYER("Draw Distance"));
}


ESPInfo::Building::Building(bool is_red) noexcept
{
	DrawOwner.export_var(ESP_SECTION_BUILDING("Draw Owner"));
	DrawAmmo.export_var(ESP_SECTION_BUILDING("Draw Ammo"));
	DrawLevel.export_var(ESP_SECTION_BUILDING("Draw Level"));
	DrawBState.export_var(ESP_SECTION_BUILDING("Draw State"));
	DrawHealth.export_var(ESP_SECTION_BUILDING("Draw Health"));
	DrawTeam.export_var(ESP_SECTION_BUILDING("Draw Team"));

	DrawColor.export_var(ESP_SECTION_BUILDING("Draw Color"));
	FontColor.export_var(ESP_SECTION_BUILDING("Font Color"));
	MaxDistance.export_var(ESP_SECTION_BUILDING("Max Distance"));
	DrawMode.export_var(ESP_SECTION_BUILDING("Draw Mode"));

	Enable.set_description("Draw building's ESP");
	Enable.export_var(ESP_SECTION_BUILDING("Enable"));

	DrawName.set_description("Draw building's name");
	DrawName.export_var(ESP_SECTION_BUILDING("Draw Name"));

	DrawDistance.set_description("Draw distance between localplayer and the building");
	DrawDistance.export_var(ESP_SECTION_BUILDING("Draw Distance"));
}


ESPInfo::Object::Object() noexcept
{
	DrawPacks.export_var(ESP_SECTION_OBJECT("Draw Packs"));
	DrawRockets.export_var(ESP_SECTION_OBJECT("Draw Rockets"));
	DrawPipes.export_var(ESP_SECTION_OBJECT("Draw Pipes"));
	DrawStickies.export_var(ESP_SECTION_OBJECT("Draw Stickies"));

	Enable.set_description("Draw object's ESP");
	Enable.export_var(ESP_SECTION_OBJECT("Enable"));

	DrawName.set_description("Draw object's name");
	DrawName.export_var(ESP_SECTION_OBJECT("Draw Name"));

	DrawDistance.set_description("Draw distance between localplayer and the object");
	DrawDistance.export_var(ESP_SECTION_OBJECT("Draw Distance"));
}