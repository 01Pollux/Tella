#include "Cheats/Main.hpp"

#include "Hooks/PaintTraverse.hpp"
#include "Helper/DrawTools.hpp"

namespace Visuals
{
	class DrawDeadPlayers
	{
	public:
		DrawDeadPlayers();

	private:
		MHookRes OnPaintTraverse(uint32_t);

		M0Config::Bool Enable{
			"Visual.Spectators.Enabled",
			true,
			"Expose who is spectating you"
		};

		M0Config::ArrayXY<int> DrawOffset{
			"Visual.Spectators.Position",
			{ 12, 130 },
			"Dead players list position"
		};
	};
	static DrawDeadPlayers dummy_drawspeclist;
}

MHookRes Visuals::DrawDeadPlayers::OnPaintTraverse(uint32_t paintID)
{
	if (paintID != VGUI::FocusOverlayPanel)
		return { };

	if (!Enable || ITFPlayerInternal::BadLocal())
		return { };

	ILocalPlayer pMe;
	if (!pMe)
		return { };

	PROFILE_USECTION("Visuals_DrawSpecList", M0PROFILER_GROUP::CHEAT_PROFILE);

	DrawTools::ScreenCoord cord{ DrawOffset[0], DrawTools::ScreenPosition.y + DrawOffset[1] };

	Interfaces::VGUISurface->DrawSetTextFont(VGUI::M0SurfaceTools::Fonts::Arial);
	Interfaces::VGUISurface->DrawSetTextColor(color::names::white);

	IFormatterSV fmt("{}: {}"sv);
	PlayerInfo playerinfo;

	for (int i = 1; i <= Interfaces::GlobalVars->MaxClients; i++)
	{
		ITFPlayer player(i);
		if (!player || player == pMe)
			continue;

		if (player->LifeState != PlayerLifeState::Dead)
			continue;

		const ITFPlayer spectacted(player->ObserverTarget.get());
		if (spectacted != pMe)
			continue;

		const char* mode;
		switch (player->ObserverMode)
		{
		case PlayerObserverMode::InEye:
		{
			mode = "FirstPerson";
			break;
		}

		case PlayerObserverMode::Chase:
		{
			mode = "ThirdPerson";
			break;
		}

		case PlayerObserverMode::FreeCam:
		{
			mode = "FreeCam";
			break;
		}

		default: continue;
		}

		if (Interfaces::EngineClient->GetPlayerInfo(i, &playerinfo))
		{
			const std::wstring str = StringTransform<std::wstring>(fmt(mode, playerinfo.Name));

			Interfaces::VGUISurface->DrawSetTextPos(cord.x, cord.y);
			Interfaces::VGUISurface->DrawPrintText(str.c_str(), str.size());

			cord.y -= VGUI::M0SurfaceTools::StringOffset;
		}
	}

	return { };
}


Visuals::DrawDeadPlayers::DrawDeadPlayers()
{
	M0EventManager::AddListener(
		EVENT_KEY_LOAD_DLL,
		[this](M0EventData*)
		{
			M0HookManager::Policy::PaintTraverse painttraverse(true);
			painttraverse->AddPostHook(HookCall::Late, std::bind(&DrawDeadPlayers::OnPaintTraverse, this, std::placeholders::_1));
		},
		EVENT_NULL_NAME
	);
}