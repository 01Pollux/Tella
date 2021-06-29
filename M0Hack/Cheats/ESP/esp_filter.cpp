#include "esp.hpp"
#include "VGUI.hpp"

#include "ResourceEntity.hpp"

void IESPHack::CollectEntities()
{
	DrawData.clear();

	const ILocalPlayer pMe;
	const Vector& my_pos = pMe->GetAbsOrigin();

	const int maxClients = Interfaces::GlobalVars->MaxClients;
	for (int i = 1; i <= maxClients; i++)
	{
		ITFPlayer player(i);
		if (!player || player == pMe)
			continue;

		ProcessPlayer_FirstPass(player, my_pos);
	}

	for (int i = maxClients + 1; i <= IBaseEntityInternal::GetHighestEntityIndex(); i++)
	{
		IBaseEntity ent(i);
		if (ent)
			ProcessEntity(ent, my_pos);
	}

	std::sort(DrawData.begin(), DrawData.end(),
		[](ESPData& a, ESPData& b)
		{
			return a.Distance > b.Distance;
		}
	);
}


MHookRes IESPHack::OnPaintTraverse(uint32_t paintID)
{
	if (paintID != VGUI::FocusOverlayPanel)
		return { };

	if (ITFPlayerInternal::BadLocal())
		return { };

	static bool should_begin = false;
	static AutoCTimer<2.1f> timer_check;

	if (timer_check.trigger_if_elapsed())
		should_begin = IsAnyActive();

	if (!should_begin)
		return { };

	{
		PROFILE_USECTION("ESP::CollectEntities", M0PROFILER_GROUP::CHEAT_PROFILE);
		CollectEntities();
	}

	{
		PROFILE_USECTION("ESP::DrawEntities", M0PROFILER_GROUP::CHEAT_PROFILE);
		for (auto& data : DrawData)
			DrawFromData(&data);
	}

	return { };
}


static void DrawTextEntity(const ESPData* info, const ESPInfo::Shared* espdata, const int max_x, const int max_y)
{
	const ESPType type = info->Type;
	IBaseEntity entity{ info->Entity };

	Interfaces::VGUISurface->DrawSetTextFont(VGUI::M0SurfaceTools::Fonts::ArialSmall);
	Interfaces::VGUISurface->DrawSetTextColor(espdata->FontColor);

	//Draw Strings
	int draw_offset = 0;
	for (const std::wstring& str : info->Strings)
	{
		Interfaces::VGUISurface->DrawSetTextPos(max_x, max_y + draw_offset);
		Interfaces::VGUISurface->DrawPrintText(str.c_str(), str.size());

		draw_offset += VGUI::M0SurfaceTools::StringOffset;
	}
}


static void DrawEntityBox(const ESPInfo::Shared* espdata, const ESPBoxInfo* databox)
{
	Interfaces::VGUISurface->DrawSetColor(espdata->DrawColor);

	switch (espdata->DrawMode)
	{
	case ESPDrawMode::Box:
	{
		for (int i = 0; i < 4; i++)
		{
			using PosType = ESPBoxInfo::PosType;

			const auto& cur_bot = databox->GetPosition<PosType::Bottom>(i);
			const auto& cur_up = databox->GetPosition<PosType::Upper>(i);

			Interfaces::VGUISurface->DrawLine(
				cur_bot.x, cur_bot.y,
				cur_up.x, cur_up.y
			);

			const auto& next_bot = databox->GetPosition<PosType::Bottom>((i + 1) % 4);
			const auto& next_up = databox->GetPosition<PosType::Upper>((i + 1) % 4);

			Interfaces::VGUISurface->DrawLine(
				next_bot.x, next_bot.y,
				cur_bot.x, cur_bot.y
			);

			Interfaces::VGUISurface->DrawLine(
				next_up.x, next_up.y,
				cur_up.x, cur_up.y
			);
		}
		break;
	}

	case ESPDrawMode::BoxOutline:
	{
		using PosType = ESPBoxInfo::PosType;

		Interfaces::VGUISurface->DrawSetColor(color::names::black);
		Interfaces::VGUISurface->DrawSetTexture(VGUI::M0SurfaceTools::Textures::Cyan);

		auto DrawOutlineRect =
			[](const ESPBoxInfo* databox, const Vector2D extra, const float width, const float weight)
		{
			auto RectAtPos =
				[width, weight, extra](const DrawTools::ScreenCoord& beg, const DrawTools::ScreenCoord& end)
			{
				{
					const VGUI::Vertex beg_pos{ extra + Vector2D{ static_cast<float>(beg.x), static_cast<float>(beg.y) } };
					const VGUI::Vertex end_pos{ extra + Vector2D{ static_cast<float>(end.x), static_cast<float>(end.y) } };

					VGUI::Vertex vert[4]{
						beg_pos,
						end_pos,
						end_pos.Position + Vector2D{ width, weight },
						beg_pos.Position + Vector2D{ width, weight },
					};

					Interfaces::VGUISurface->DrawTexturedPolygon(SizeOfArray(vert), vert);
				}
			};

			for (int i = 0; i < 4; i++)
			{
				using PosType = ESPBoxInfo::PosType;

				const auto& cur_bot = databox->GetPosition<PosType::Bottom>(i);
				const auto& cur_up = databox->GetPosition<PosType::Upper>(i);

				RectAtPos(cur_bot, cur_up);

				{
					const auto& next_bot = databox->GetPosition<PosType::Bottom>((i + 1) % 4);
					RectAtPos(cur_bot, next_bot);
				}

				{
					const auto& next_up = databox->GetPosition<PosType::Upper>((i + 1) % 4);
					RectAtPos(cur_up, next_up);
				}
			}
		};

		DrawOutlineRect(databox, Vector2D{ }, 7.5f, -3.9f);

		Interfaces::VGUISurface->DrawSetColor(espdata->DrawColor);
		DrawOutlineRect(databox, Vector2D{ 2.15f, -2.05f }, 3.f, 1.35f);
		break;
	}
	}
}


void IESPHack::DrawFromData(const ESPData* info)
{
	PROFILE_USECTION("ESP::DrawFromData", M0PROFILER_GROUP::CHEAT_PROFILE);

	ESPBoxInfo databox;
	if (!GetBoxInfo(info, &databox))
		return;

	IBaseEntity entity = info->Entity;

	ESPInfo::Shared* espdata{ };
	bool should_draw_health = false;

	switch (info->Type)
	{
	case ESPType::Player:
	{
		espdata = &PlayerESPInfo[static_cast<int>(info->Entity->TeamNum.get()) - 2];
		should_draw_health = static_cast<ESPInfo::Player*>(espdata)->DrawHealth;
		break;
	}
	case ESPType::Building:
	{
		espdata = &BuildingESPInfo[static_cast<int>(info->Entity->TeamNum.get()) - 2];
		should_draw_health = static_cast<ESPInfo::Building*>(espdata)->DrawHealth;
		break;
	}
	case ESPType::Objects:
	{
		espdata = &ObjectESPInfo;
		break;
	}
	}

	//	Draw Box
	DrawEntityBox(espdata, &databox);

	const ESPType type = info->Type;

	//Draw Health Bar + Strings
	{
		auto draw_pos = databox.GetPosition<ESPBoxInfo::PosType::Upper>(ESPBoxInfo::BOTTOM_L);
		constexpr int health_width = 70, health_height = 16;

		int x = draw_pos.x, y = draw_pos.y;

		//Draw Health
		if (should_draw_health)
		{
			int health, max_health;
			switch (type)
			{
			case ESPType::Player:
			{
				const int idx = entity->entindex();
				health = TFResourceEntity::Player->Health[idx];
				max_health = TFResourceEntity::Player->MaxHealth[idx];
				break;
			}
			case ESPType::Building:
			{
				health = static_cast<IBaseObject>(entity)->Health;
				max_health = static_cast<IBaseObject>(entity)->MaxHealth;
				break;
			}
			}

			const float perc = std::min(static_cast<float>(health) / static_cast<float>(max_health), 1.0f);
			const color::u8rgba color = DrawTools::GetHealth(health, max_health);

			// Draw Health backgroud
			{
				Interfaces::VGUISurface->DrawSetColor(color::names::white);
				Interfaces::VGUISurface->DrawFilledRect(x - 3, y, x + health_width, y + health_height);
			}

			// Draw Health
			{
				Interfaces::VGUISurface->DrawSetColor(color);
				Interfaces::VGUISurface->DrawFilledRect(x - 3, y, x + static_cast<int>(health_width * perc), y + health_height);
			}
		}

		DrawTextEntity(info, espdata, x + 8, y + 14);
	}
}


bool IESPHack::GetBoxInfo(const ESPData* info, ESPBoxInfo* boxinfo)
{
	const ICollideable* pCol = info->Entity->CollisionProp.data();

	const Vector& origin = pCol->GetCollisionOrigin();
	const Vector mins = pCol->OBBMins() + origin;
	const Vector maxs = pCol->OBBMaxs() + origin;

	const Vector delta = maxs - mins;

	Vector corners[]{
		mins,
		mins + Vector{ delta.x, 0.f,		0.f },
		mins + Vector{ delta.x, delta.y,	0.f },
		mins + Vector{ 0.f,		delta.y,	0.f },

		mins + Vector{ 0.f,		0.f,		delta.z },
		mins + Vector{ delta.x,	0.f,		delta.z },
		mins + Vector{ delta.x,	delta.y,	delta.z },
		mins + Vector{ 0.f,		delta.y,	delta.z },
	};

	static_assert(SizeOfArray(corners) == 8);

	for (size_t i = 0; i < 8; i++)
		if (!DrawTools::GetVectorInHudSpace(corners[i], boxinfo->corners[i]))
			return false;

	return true;
}