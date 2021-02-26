#pragma once

#include "ESP.h"
#include "../Helpers/Commons.h"
#include "../Helpers/DrawTools.h"

#include <algorithm>
#include <mutex>

#include "../Interfaces/IClientListener.h"
#include "../Helpers/Timer.h"
#include "../Interfaces/VGUIS.h"

IESPHack EMenu;

struct ESPData
{
	IESPHack::ESPType_t type = IESPHack::ESPType_t::Objects;
	short color_offset = 0;

	float dist;

	IClientShared* pEnt;
	std::vector<std::string> strings;

	Vector collide_min;
	Vector collide_max;

	Color color;

	void AddEntityString(std::string&& str) { strings.push_back(std::move(str)); }
	void AddEntityString(std::string&& str, std::vector<std::string>::iterator iter) { strings.insert(iter, std::move(str)); }
};
static std::vector<ESPData> espdata;

#define ADD_STRING(ESP, TYPE, ...) \
		if (ESP##.m_bDraw##TYPE) \
			data.AddEntityString(__VA_ARGS__)

#define GET_RANDOM(VAL, TYPE) \
		{ \
			switch (data.type) \
			{ \
			case ESPType_t::Player: \
				VAL = player_esp[pEnt->GetTeam() - 2].##TYPE; \
			break; \
						\
			case ESPType_t::Building: \
				VAL = building_esp[pEnt->GetTeam() - 2].##TYPE; \
			break; \
					 \
			default: \
				VAL = objects_esp.##TYPE; \
			break; \
			} \
		}

void ProcessEntity(const MyClientCacheList& pEnt, const Vector&);


void IESPHack::OnRender()
{
	constexpr const char* ESPTeam[] = { "RED", "BLU" };

	if (ImGui::BeginTabItem("ESP"))
	{
		if (ImGui::CollapsingHeader("Player Manager"))
		{
			ImGui::PushID("PM");
			for (uint i = 0; i < 2; i++)
			{
				ImGui::PushID(i);

				if (ImGui::TreeNode(ESPTeam[i]))
				{
					ImGui::Checkbox("Enable", &(player_esp[i].base.m_bActive));
					ImGui::Separator();

					//colors
					{
						ImGui::Text("Draw Color:");
						ImGui::SliderInt4("##COLOR1", player_esp[i].base.m_iDrawColor, 0, 255);
						ImGui::Text("Font Color:");
						ImGui::SliderInt4("##COLOR2", player_esp[i].base.m_iFontColor, 0, 255);
						ImGui::Separator();
					}

					//distance
					{
						ImGui::Text("Max Distance:");
						ImGui::DragFloat("##MAXDIST", &(player_esp[i].base.m_flMaxDist));
						ImGui::Separator();
					}

					//Boxes, outline, particles mode
					{
						static std::string display_str[2]{ "None", "None" };
						ImGui::Text("Modes:");
						if (ImGui::BeginCombo("##MODES", display_str[i].c_str()))
						{
							constexpr const char* modes[] = { "Box", "Box 3D" };
							static bool enabled[2][SizeOfArray(modes)]{ };
							static int actual = 0;

							static std::vector<const char*> vec;
							vec.clear();

							for (uint x = 0; x < SizeOfArray(modes); x++)
							{
								ImGui::Selectable(modes[x], &enabled[i][x], ImGuiSelectableFlags_DontClosePopups);
								if (enabled[i][x])
								{
									vec.push_back(modes[x]);
									actual |= (1 << x);
								}
								else {
									actual &= ~(1 << x);
								}
							}

							if (vec.size())
							{
								for (size_t x = 0; x < vec.size(); x++)
								{
									if (!x)
										display_str[i] = vec[x];
									else
										display_str[i] += " | " + std::string(vec[x]);
								}
							}
							else
							{
								display_str[i] = "None";
							}

							player_esp[i].base.m_bitsDrawMode = static_cast<ESPStruct::ESPMode>(actual);
							ImGui::EndCombo();
						}

						ImGui::SameLine();
						ImGui::DrawHelp("Select display type");
						ImGui::Separator();
					}

					//what to display
					{
						ImGui::Dummy(ImVec2(0, 10));

						ImGui::DrawHelp("Select what to display");

						ImGui::Dummy(ImVec2(0, 10));

						ImGui::Checkbox("Draw Name", &(player_esp[i].base.m_bDrawName));
						ImGui::Checkbox("Draw Distance", &(player_esp[i].base.m_bDrawDistance));


						ImGui::Checkbox("Draw Class", &(player_esp[i].m_bDrawClass));
						ImGui::Checkbox("Draw Conditions", &(player_esp[i].m_bDrawCond));
						ImGui::Checkbox("Draw UberCharge", &(player_esp[i].m_bDrawUber));
						ImGui::Checkbox("Ignore Cloaked", &(player_esp[i].m_bIgnoreCloak));

						ImGui::Checkbox("Draw Health", &(player_esp[i].m_bDrawHealth));
						ImGui::Checkbox("Draw Team", &(player_esp[i].m_bDrawTeam));

						ImGui::Dummy(ImVec2(0, 10));

						ImGui::Separator();
					}

					ImGui::TreePop();
				}

				ImGui::PopID();
			}
			ImGui::PopID();
		}
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Building Manager"))
		{
			ImGui::PushID("BM");
			for (uint i = 0; i < 2; i++)
			{
				if (ImGui::TreeNode(ESPTeam[i]))
				{
					ImGui::Checkbox("Enable", &(building_esp[i].base.m_bActive));
					ImGui::Separator();

					//colors
					{
						ImGui::Text("Draw Color:");
						ImGui::SliderInt4("##COLOR1", building_esp[i].base.m_iDrawColor, 0, 255);
						ImGui::Text("Font Color:");
						ImGui::SliderInt4("##COLOR2", building_esp[i].base.m_iFontColor, 0, 255);
						ImGui::Separator();
					}

					//distance
					{
						ImGui::Text("Max Distance:");
						ImGui::DragFloat("##MAXDIST", &(building_esp[i].base.m_flMaxDist));
						ImGui::Separator();
					}

					//Boxes, outline, particles mode
					{
						static std::string display_str[2]{ "None", "None" };
						ImGui::Text("Modes:");
						if (ImGui::BeginCombo("##MODES", display_str[i].c_str()))
						{
							static const char* modes[] = { "Box", "Box 3D"};
							static bool enabled[2][SizeOfArray(modes)]{ };
							static int actual = 0;

							static std::vector<const char*> vec;
							vec.clear();

							for (uint x = 0; x < IM_ARRAYSIZE(modes); x++)
							{
								ImGui::Selectable(modes[x], &enabled[i][x], ImGuiSelectableFlags_DontClosePopups);
								if (enabled[i][x])
								{
									vec.push_back(modes[x]);
									actual |= (1 << i);
								}
								else {
									actual &= ~(1 << i);
								}
							}

							if (vec.size())
							{
								for (size_t x = 0; x < vec.size(); x++)
								{
									if (!x)
										display_str[i] = vec[x];
									else
										display_str[i] += " | " + std::string(vec[x]);
								}
							}
							else
							{
								display_str[i] = "None";
							}

							building_esp[i].base.m_bitsDrawMode = static_cast<ESPStruct::ESPMode>(actual);
							ImGui::EndCombo();
						}
						ImGui::SameLine();
						ImGui::DrawHelp("Select display type");
						ImGui::Separator();
					}

					//what to display
					{
						ImGui::Dummy(ImVec2(0, 10));

						ImGui::DrawHelp("Select what to display");

						ImGui::Dummy(ImVec2(0, 10));

						ImGui::Checkbox("Draw Name", &(building_esp[i].base.m_bDrawName));
						ImGui::Checkbox("Draw Distance", &(building_esp[i].base.m_bDrawDistance));

						ImGui::Checkbox("Draw Owner", &(building_esp[i].m_bDrawOwner));
						ImGui::Checkbox("Draw Ammo", &(building_esp[i].m_bDrawAmmo));
						ImGui::Checkbox("Draw Level", &(building_esp[i].m_bDrawLevel));
						ImGui::Checkbox("Draw State", &(building_esp[i].m_bDrawState));

						ImGui::Checkbox("Draw Health", &(building_esp[i].m_bDrawHealth));
						ImGui::Checkbox("Draw Team", &(building_esp[i].m_bDrawTeam));

						ImGui::Dummy(ImVec2(0, 10));

						ImGui::Separator();
					}

					ImGui::TreePop();
				}
			}
			ImGui::PopID();
		}
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Objects Manager"))
		{
			ImGui::PushID("OM");

			ImGui::Checkbox("Enable", &(objects_esp.base.m_bActive));
			ImGui::Separator();
			
			//colors
			{
				ImGui::Text("Draw Color:");
				ImGui::SliderInt4("##COLOR1", objects_esp.base.m_iDrawColor, 0, 255);
				ImGui::Text("Font Color:");
				ImGui::SliderInt4("##COLOR2", objects_esp.base.m_iFontColor, 0, 255);
				ImGui::Separator();
			}

			//distance
			{
				ImGui::Text("Max Distance:");
				ImGui::DragFloat("##MAXDIST", &(objects_esp.base.m_flMaxDist));
				ImGui::Separator();
			}

			//Boxes, outline, particles mode
			{
				static std::string display_str = "";
				ImGui::Text("Modes:");
				if (ImGui::BeginCombo("##MODES", display_str.c_str()))
				{
					static const char* modes[] = { "Box", "Box 3D" };
					static bool enabled[SizeOfArray(modes)]{ };
					static int actual = 0;

					static std::vector<const char*> vec;
					vec.clear();

					for (int x = 0; x < IM_ARRAYSIZE(modes); x++)
					{
						ImGui::Selectable(modes[x], &enabled[x], ImGuiSelectableFlags_DontClosePopups);
						if (enabled[x])
						{
							vec.push_back(modes[x]);
							actual |= (1 << x);
						}
						else {
							actual &= ~(1 << x);
						}
					}

					if (vec.size())
						for (size_t x = 0; x < vec.size(); x++)
						{
							if (!x) display_str = vec[x];
							else	display_str += " | " + std::string(vec[x]);
						}
					else			display_str = "None";

					objects_esp.base.m_bitsDrawMode = static_cast<ESPStruct::ESPMode>(actual);

					ImGui::EndCombo();
				}
				ImGui::SameLine();
				ImGui::DrawHelp("Select display type");
				ImGui::Separator();
			}

			//what to display
			{
				ImGui::Dummy(ImVec2(0, 10));

				ImGui::DrawHelp("Select what to display");

				ImGui::Dummy(ImVec2(0, 10));

				ImGui::Checkbox("Draw Name", &(objects_esp.base.m_bDrawName));
				ImGui::Checkbox("Draw Distance", &(objects_esp.base.m_bDrawDistance));

				ImGui::Checkbox("Draw Packs", &(objects_esp.m_bDrawPacks));
				ImGui::Checkbox("Draw Rockets", &(objects_esp.m_bDrawRockets));
				ImGui::Checkbox("Draw Stickies", &(objects_esp.m_bDrawStickies));

				ImGui::Dummy(ImVec2(0, 10));

				ImGui::Separator();
			}

			ImGui::PopID();
		}
		ImGui::Separator();
		
		ImGui::EndTabItem();
	}
}


HookRes IESPHack::OnPaintTraverse(uint pId)
{
	if (pId != PID_FocusOverlay)
		return HookRes::Continue;

	if (::BadLocal())
		return HookRes::Continue;

	static std::mutex esp_mutex;
	std::lock_guard<std::mutex> mutex(esp_mutex);

	static bool m_bBeginDraw = false;
	static Timer timer_check;

	if (timer_check.trigger_if_elapsed(2500ms))
		m_bBeginDraw = IsActive();

	if (!m_bBeginDraw)
		return HookRes::Continue;

	CollectEntities();

	M0Profiler watch_esp("ESPMenu::OnPaintTraverse::DrawEntities", M0PROFILER_GROUP::CHEAT_PROFILE);
	for (auto& data : espdata)
		DrawEntities(data);

	return HookRes::Continue;
}

void IESPHack::DrawEntities(ESPData& data)
{
	IClientShared* pEnt = data.pEnt;
	Vector origin;

	if (!DrawTools::WorldToScreen(pEnt->GetAbsOrigin(), origin))
		return;

	if (!DrawBox(data))
		return;

	const ESPType_t& type = data.type;
	bool m_bDrawHealth = false;
	switch (type)
	{
	case ESPType_t::Player: 
		m_bDrawHealth = player_esp[pEnt->GetTeam() - 2].m_bDrawHealth;
	break; 

	case ESPType_t::Building: 
		m_bDrawHealth = building_esp[pEnt->GetTeam() - 2].m_bDrawHealth;
	break; 
	}

	//Draw Health Bar + Strings
	{
		const int& max_x = data.collide_max.x;
		const int& max_y = data.collide_max.y;
		const int& min_x = data.collide_max.x;
		const int& min_y = data.collide_min.y;

		short color_offs = data.color_offset;

		//Draw Health
		if (m_bDrawHealth)
		{
			int health, max_health;
			if (type == ESPType_t::Player)
			{
				health = ctfresource.GetEntProp<int>(pEnt, "m_iHealth");
				max_health = ctfresource.GetEntProp<int>(pEnt, "m_iMaxHealth");
			}
			else if (type == ESPType_t::Building) {
				health = reinterpret_cast<IBaseObject*>(pEnt)->GetBuildingHealth();
				max_health = reinterpret_cast<IBaseObject*>(pEnt)->GetBuildingHealth();
			}
			else goto Draw_Text;
			
			int hbw = static_cast<int>((max_y - min_y - 2) * min(static_cast<float>(health) / static_cast<float>(max_health), 1.0f));

			Color color = DrawTools::ColorTools::GetHealth(health, max_health);

			DrawTools::OutlinedRect(min_x, min_y - 6, max_x - min_x + 1, 7, DrawTools::ColorTools::FromArray(DrawTools::ColorTools::White<char8_t>));
			DrawTools::Rect(min_x + hbw, min_y - 5, -hbw, 5, color);
		}

Draw_Text:
		{
			int m_iDrawOffset = 0;

			static Color color[2][3]{};
			{
				static Timer update_color[2][3];
				if (update_color[color_offs][type].trigger_if_elapsed(3s))
				{
					int* colors;
					GET_RANDOM(colors, base.m_iFontColor);
					color[color_offs][type].SetColor(colors[0], colors[1], colors[2], colors[3]);
				}
			}
			
			//Draw Strings
			for (std::string& str : data.strings)
			{
				DrawTools::DrawString(max_x - 2, min_y + m_iDrawOffset, color[color_offs][type], str);
				m_iDrawOffset += DrawTools::m_iStringOffset;
			}
		}
	}
}

bool IESPHack::DrawBox(ESPData& data)
{
	IClientShared* pEnt = data.pEnt;
	Color& color = data.color;

	ICollideable* pCol = pEnt->GetCollideable();
	const Vector& origin = pCol->GetCollisionOrigin();
	const Vector& mins = pCol->OBBMins() + origin;
	const Vector& maxs = pCol->OBBMaxs() + origin;

	static Vector points[8];

	int max_x = -1, max_y = -1, min_x = 65536, min_y = 65536;

	{
		float x = maxs.x - mins.x;
		float y = maxs.y - mins.y;
		float z = maxs.z - mins.z;

		Vector corners[8]{ mins, mins, mins, mins, mins, mins, mins, mins };

		corners[1] += Vector(x, 0, 0);
		corners[2] += Vector(x, y, 0);
		corners[3] += Vector(0, y, 0);
		corners[4] += Vector(0, 0, z);
		corners[5] += Vector(x, 0, z);
		corners[6] += Vector(x, y, z);
		corners[7] += Vector(0, y, z);

		for (uint i = 0; i < 8; i++)
			if (!DrawTools::WorldToScreen(corners[i], points[i]))
				return false;

		for (uint i = 1; i < 8; i++)
		{
			if (points[i].x > max_x)
				max_x = points[i].x;
			if (points[i].y > max_y)
				max_y = points[i].y;

			if (points[i].x < min_x)
				min_x = points[i].x;
			if (points[i].y < min_y)
				min_y = points[i].y;
		}

		data.collide_max.Init(max_x, max_y, 0.0f);
		data.collide_min.Init(min_y, min_y, 0.0f);
	}

	ESPStruct::ESPMode bits;

	GET_RANDOM(bits, base.m_bitsDrawMode);

//	Draw Box
	{
		if (bits & ESPStruct::ESPMode::BOX3D)
		{
			for (uint i = 1; i <= 4; i++)
			{
				DrawTools::Line((points[i - 1].x), (points[i - 1].y), (points[i % 4].x) - (points[i - 1].x), (points[i % 4].y) - (points[i - 1].y), color);
				DrawTools::Line((points[i - 1].x), (points[i - 1].y), (points[i + 3].x) - (points[i - 1].x), (points[i + 3].y) - (points[i - 1].y), color);
				DrawTools::Line((points[i + 3].x), (points[i + 3].y), (points[i % 4 + 4].x) - (points[i + 3].x), (points[i % 4 + 4].y) - (points[i + 3].y), color);
			}
		}

		if (bits & ESPStruct::ESPMode::BOX)
		{
			constexpr int size = 20;
			using namespace DrawTools::ColorTools;

			DrawTools::Rect(min_x, min_y, size, 3, FromArray(Black<char8_t>));
			DrawTools::Rect(min_x, min_y + 3, 3, size - 3, FromArray(Black<char8_t>));

			DrawTools::Rect(max_x - size + 1, min_y, size, 3, FromArray(Black<char8_t>));
			DrawTools::Rect(max_x - 3 + 1, min_y + 3, 3, size - 3, FromArray(Black<char8_t>));

			DrawTools::Rect(min_x, max_y - 3, size, 3, FromArray(Black<char8_t>));
			DrawTools::Rect(min_x, max_y - size, 3, size - 3, FromArray(Black<char8_t>));

			DrawTools::Rect(max_x - size + 1, max_y - 3, size, 3, FromArray(Black<char8_t>));
			DrawTools::Rect(max_x - 2, max_y - size, 3, size - 3, FromArray(Black<char8_t>));


			DrawTools::Line(min_x + 1, min_y + 1, size - 2, 0, color);
			DrawTools::Line(min_x + 1, min_y + 1, 0, size - 2, color);

			DrawTools::Line(max_x - 1, min_y + 1, -(size - 2), 0, color);
			DrawTools::Line(max_x - 1, min_y + 1, 0, size - 2, color);

			DrawTools::Line(min_x + 1, max_y - 2, size - 2, 0, color);
			DrawTools::Line(min_x + 1, max_y - 2, 0, -(size - 2), color);

			DrawTools::Line(max_x - 1, max_y - 2, -(size - 2), 0, color);
			DrawTools::Line(max_x - 1, max_y - 2, 0, -(size - 2), color);
		}
	}

	return true;
}

void IESPHack::CollectEntities()
{
	M0Profiler watch_esp("ESPMenu::OnPaintTraverse::CollectEntities", M0PROFILER_GROUP::CHEAT_PROFILE);

	espdata.clear();
	const Vector& myOrg = ::ILocalPtr()->GetAbsOrigin();
	
	for (const auto& infos = ent_infos.GetInfos(); 
		const auto& cache : infos)
	{
		ProcessEntity(cache, myOrg);
	}

	std::sort(espdata.begin(), espdata.end(),
		[](ESPData& a, ESPData& b)
		{
			return a.dist > b.dist;
		}
	);
}


void IESPHack::JsonCallback(Json::Value& json, bool read)
{
	Json::Value& ESPCfg = json["ESP"];
	constexpr const char* ESPTeam[] = { "RED", "BLU" };

	Json::Value& PM = ESPCfg["Player Manager"];
	Json::Value& BM = ESPCfg["Building Manager"];
	Json::Value& OM = ESPCfg["Object Manager"];

	if (read)
	{
		int temp{ };
		for (uint i = 0; i < 2; i++)
		{
			{
				Json::Value& curPM = PM[ESPTeam[i]];
				{
					PROCESS_JSON_READ(curPM, "Active", Bool, player_esp[i].base.m_bActive);
					PROCESS_JSON_READ(curPM, "Max Distance", Float, player_esp[i].base.m_flMaxDist);

					PROCESS_JSON_READ_COLOR(curPM, "Draw Color", Int, player_esp[i].base.m_iDrawColor);
					PROCESS_JSON_READ_COLOR(curPM, "Font Color", Int, player_esp[i].base.m_iFontColor);
				}
				{
					PROCESS_JSON_READ(curPM, "bits Draw", Int, temp);
					player_esp[i].base.m_bitsDrawMode = static_cast<IESPHack::ESPStruct::ESPMode>(temp);
				}
				{
					PROCESS_JSON_READ(curPM, "Draw Name", Bool, player_esp[i].base.m_bDrawName);
					PROCESS_JSON_READ(curPM, "Draw Distance", Bool, player_esp[i].base.m_bDrawDistance);
					PROCESS_JSON_READ(curPM, "Draw Class", Bool, player_esp[i].m_bDrawClass);
					PROCESS_JSON_READ(curPM, "Draw Conditions", Bool, player_esp[i].m_bDrawCond);
					PROCESS_JSON_READ(curPM, "Draw UberCharge", Bool, player_esp[i].m_bDrawUber);
					PROCESS_JSON_READ(curPM, "Draw Cloaked", Bool, player_esp[i].m_bIgnoreCloak);
					PROCESS_JSON_READ(curPM, "Draw Health", Bool, player_esp[i].m_bDrawHealth);
					PROCESS_JSON_READ(curPM, "Draw Team", Bool, player_esp[i].m_bDrawTeam);
				}
			}

			{
				Json::Value& curBM = BM[ESPTeam[i]];
				{
					PROCESS_JSON_READ(curBM, "Active", Bool, building_esp[i].base.m_bActive);
					PROCESS_JSON_READ(curBM, "Max Distance", Float, building_esp[i].base.m_flMaxDist);

					PROCESS_JSON_READ_COLOR(curBM, "Draw Color", Int, building_esp[i].base.m_iDrawColor);
					PROCESS_JSON_READ_COLOR(curBM, "Font Color", Int, building_esp[i].base.m_iFontColor);
				}
				{
					PROCESS_JSON_READ(curBM, "bits Draw", Int, temp);
					building_esp[i].base.m_bitsDrawMode = static_cast<IESPHack::ESPStruct::ESPMode>(temp);
				}
				{
					PROCESS_JSON_READ(curBM, "Draw Name", Bool, building_esp[i].base.m_bDrawName);
					PROCESS_JSON_READ(curBM, "Draw Distance", Bool, building_esp[i].base.m_bDrawDistance);
					PROCESS_JSON_READ(curBM, "Draw Owner", Bool, building_esp[i].m_bDrawOwner);
					PROCESS_JSON_READ(curBM, "Draw Ammo", Bool, building_esp[i].m_bDrawAmmo);
					PROCESS_JSON_READ(curBM, "Draw Level", Bool, building_esp[i].m_bDrawLevel);
					PROCESS_JSON_READ(curBM, "Draw State", Bool, building_esp[i].m_bDrawState);
					PROCESS_JSON_READ(curBM, "Draw Health", Bool, building_esp[i].m_bDrawHealth);
					PROCESS_JSON_READ(curBM, "Draw Team", Bool, building_esp[i].m_bDrawTeam);
				}
			}
		}
		{
			{
				PROCESS_JSON_READ(OM, "Active", Bool, objects_esp.base.m_bActive);
				PROCESS_JSON_READ(OM, "Max Distance", Float, objects_esp.base.m_flMaxDist);

				PROCESS_JSON_READ_COLOR(OM, "Draw Color", Int, objects_esp.base.m_iDrawColor);
				PROCESS_JSON_READ_COLOR(OM, "Font Color", Int, objects_esp.base.m_iFontColor);
			}
			{
				PROCESS_JSON_READ(OM, "bits Draw", Int, temp);
				objects_esp.base.m_bitsDrawMode = static_cast<IESPHack::ESPStruct::ESPMode>(temp);
			}
			{
				PROCESS_JSON_READ(OM, "Draw Name", Bool, objects_esp.base.m_bDrawName);
				PROCESS_JSON_READ(OM, "Draw Distance", Bool, objects_esp.base.m_bDrawDistance);
				PROCESS_JSON_READ(OM, "Draw Packs", Bool, objects_esp.m_bDrawPacks);
				PROCESS_JSON_READ(OM, "Draw Rockets", Bool, objects_esp.m_bDrawRockets);
				PROCESS_JSON_READ(OM, "Draw Stickies", Bool, objects_esp.m_bDrawStickies);
			}
		}
	}
	else
	{
		for (uint i = 0; i < 2; i++)
		{
			{
				Json::Value& curPM = PM[ESPTeam[i]];
				{
					PROCESS_JSON_WRITE(curPM, "Active", player_esp[i].base.m_bActive);
					PROCESS_JSON_WRITE(curPM, "Max Distance", player_esp[i].base.m_flMaxDist);

					PROCESS_JSON_WRITE_COLOR(curPM, "Draw Color", player_esp[i].base.m_iDrawColor);
					PROCESS_JSON_WRITE_COLOR(curPM, "Font Color", player_esp[i].base.m_iFontColor);
				}
				{
					PROCESS_JSON_WRITE(curPM, "bits Draw", static_cast<int>(player_esp[i].base.m_bitsDrawMode));
				}
				{
					PROCESS_JSON_WRITE(curPM, "Draw Name", player_esp[i].base.m_bDrawName);
					PROCESS_JSON_WRITE(curPM, "Draw Distance", player_esp[i].base.m_bDrawDistance);
					PROCESS_JSON_WRITE(curPM, "Draw Class", player_esp[i].m_bDrawClass);
					PROCESS_JSON_WRITE(curPM, "Draw Conditions", player_esp[i].m_bDrawCond);
					PROCESS_JSON_WRITE(curPM, "Draw UberCharge", player_esp[i].m_bDrawUber);
					PROCESS_JSON_WRITE(curPM, "Draw Cloaked", player_esp[i].m_bIgnoreCloak);
					PROCESS_JSON_WRITE(curPM, "Draw Health", player_esp[i].m_bDrawHealth);
					PROCESS_JSON_WRITE(curPM, "Draw Team", player_esp[i].m_bDrawTeam);
				}
			}

			{
				Json::Value& curBM = BM[ESPTeam[i]];
				{
					PROCESS_JSON_WRITE(curBM, "Active", building_esp[i].base.m_bActive);
					PROCESS_JSON_WRITE(curBM, "Max Distance", building_esp[i].base.m_flMaxDist);

					PROCESS_JSON_WRITE_COLOR(curBM, "Draw Color", building_esp[i].base.m_iDrawColor);
					PROCESS_JSON_WRITE_COLOR(curBM, "Font Color", building_esp[i].base.m_iFontColor);
				}
				{
					PROCESS_JSON_WRITE(curBM, "bits Draw",  static_cast<int>(building_esp[i].base.m_bitsDrawMode));
				}
				{
					PROCESS_JSON_WRITE(curBM, "Draw Name", building_esp[i].base.m_bDrawName);
					PROCESS_JSON_WRITE(curBM, "Draw Distance", building_esp[i].base.m_bDrawDistance);
					PROCESS_JSON_WRITE(curBM, "Draw Owner", building_esp[i].m_bDrawOwner);
					PROCESS_JSON_WRITE(curBM, "Draw Ammo", building_esp[i].m_bDrawAmmo);
					PROCESS_JSON_WRITE(curBM, "Draw Level", building_esp[i].m_bDrawLevel);
					PROCESS_JSON_WRITE(curBM, "Draw State", building_esp[i].m_bDrawState);
					PROCESS_JSON_WRITE(curBM, "Draw Health", building_esp[i].m_bDrawHealth);
					PROCESS_JSON_WRITE(curBM, "Draw Team", building_esp[i].m_bDrawTeam);
				}
			}
		}
		{
			{
				PROCESS_JSON_WRITE(OM, "Active", objects_esp.base.m_bActive);
				PROCESS_JSON_WRITE(OM, "Max Distance", objects_esp.base.m_flMaxDist);

				PROCESS_JSON_WRITE_COLOR(OM, "Draw Color", objects_esp.base.m_iDrawColor);
				PROCESS_JSON_WRITE_COLOR(OM, "Font Color", objects_esp.base.m_iFontColor);
			}
			{
				PROCESS_JSON_WRITE(OM, "bits Draw", static_cast<int>(objects_esp.base.m_bitsDrawMode));
			}
			{
				PROCESS_JSON_WRITE(OM, "Draw Name", objects_esp.base.m_bDrawName);
				PROCESS_JSON_WRITE(OM, "Draw Distance", objects_esp.base.m_bDrawDistance);
				PROCESS_JSON_WRITE(OM, "Draw Packs", objects_esp.m_bDrawPacks);
				PROCESS_JSON_WRITE(OM, "Draw Rockets", objects_esp.m_bDrawRockets);
				PROCESS_JSON_WRITE(OM, "Draw Stickies", objects_esp.m_bDrawStickies);
			}
		}
	}
}


static void ProcessPlayer(ESPData& data, ITFPlayer* pPlayer)
{
	if (pPlayer->GetLifeState() != LIFE_STATE::ALIVE)
		return;

	auto& players = EMenu.player_esp[pPlayer->GetTeam() - 2];
	//Draw Name
	{
		static player_info_t infos;
		if (engineclient->GetPlayerInfo(pPlayer->entindex(), &infos))
			ADD_STRING(players.base, Name, infos.name, data.strings.begin());
	}

	//Draw Class
	{
		TFClass cls = pPlayer->GetClass();
		ADD_STRING(players, Class, m_szTFClasses[cls - 1]);
	}

	//Draw Uber
	{
		if (pPlayer->GetClass() == TF_Medic)
		{
			IBaseHandle* pWeaponList = pPlayer->GetWeaponList();
			for (uint i = 0; pWeaponList[i].IsValid() && i < 3; i++)
			{
				IBaseObject* pMedigun = reinterpret_cast<IBaseObject*>(clientlist->GetClientEntityFromHandle(pWeaponList[i]));
				if (pMedigun && pMedigun->IsClassID(ClassID_CWeaponMedigun))
				{
					float flcharge = floor(*pMedigun->GetEntProp<float, PropType::Recv>("m_flChargeLevel") * 100.0);
					std::string charge = std::to_string(static_cast<int>(flcharge));

					if (*pMedigun->GetEntProp<int, PropType::Recv>("m_iItemDefinitionIndex") != 998) // The Vaccinator
						ADD_STRING(players, Uber, charge + "% Uber");
					else ADD_STRING(players, Uber, charge + "% Uber | Charges: " + std::to_string(floor(flcharge / 0.25)));
					
					break;
				}
			}
//			IBaseObject* pMedicgun = reinterpret_cast<IBaseObject*>((pPlayer->GetWeaponList()[2]));
		}
	}

	//Draw Conds
	{
		if (players.m_bDrawCond)
		{
			for (auto&& [cond, name] : TFCondMap)
				if (pPlayer->InCond(cond))
					data.AddEntityString(name);
		}
	}

	data.color.SetColor(players.base.m_iDrawColor[0], 
						players.base.m_iDrawColor[1],
						players.base.m_iDrawColor[2],
						players.base.m_iDrawColor[3]);

	espdata.push_back(std::move(data));
}

static void ProcessBuilding(ESPData& data, IBaseObject* pObject, int classid)
{
	auto& buildings = EMenu.building_esp[pObject->GetTeam() - 2];

	int level = pObject->GetUpgradeLvl();
	bool IsMini = *pObject->GetEntProp<uint8_t, PropType::Recv>("m_bMiniBuilding");
	bool sapped = *pObject->GetEntProp<uint8_t, PropType::Recv>("m_bHasSapper");

	if (!IsMini) ADD_STRING(buildings, Level, "Lvl " + std::to_string(level));
	else		 ADD_STRING(buildings.base, Name, "<Mini>");

	if (sapped)
		ADD_STRING(buildings, State, "<SAPPED>");

	if(*pObject->GetEntProp<bool, PropType::Recv>("m_bCarried"))
	{
		ADD_STRING(buildings, State, "Carried");
		espdata.push_back(data);
		return;
	}
	else if (*pObject->GetEntProp<bool, PropType::Recv>("m_bBuilding"))
	{
		ADD_STRING(buildings, State, "Building");
		espdata.push_back(data);
		return;
	}

	data.color.SetColor(buildings.base.m_iDrawColor[0], buildings.base.m_iDrawColor[1], buildings.base.m_iDrawColor[2], buildings.base.m_iDrawColor[3]);

	switch (classid)
	{
	case ClassID_CObjectTeleporter:
	{
		int state = *pObject->GetEntProp<int, PropType::Recv>("m_iState");
		switch (state)
		{
		case 0: //Building
		{
			ADD_STRING(buildings, State, "Building...");
			break;
		}

		case 2:	//Ready
		{
			ADD_STRING(buildings, State, "Ready");
			break;
		}

		case 6:	//Recharging
		{
			float m_flRechargeTime = *pObject->GetEntProp<float, PropType::Recv>("m_flRechargeTime");
			float flPercent = m_flRechargeTime - gpGlobals->curtime;

			ADD_STRING(buildings, State, std::string("Charging: " + std::to_string(flPercent) + "s"));
			break;
		}

		default:
			break;
		}

		break;
	}

	case ClassID_CObjectSentrygun:
	{
		bool m_bPlayerControlled = *pObject->GetEntProp<bool, PropType::Recv>("m_bPlayerControlled");
		int m_iAmmoShells = *pObject->GetEntProp<int, PropType::Recv>("m_iAmmoShells"), m_iAmmoRockets = *pObject->GetEntProp<int, PropType::Recv>("m_iAmmoRockets");
		int m_iMaxAmmoShells = *pObject->GetEntProp<int, PropType::Recv>("m_iAmmoShells", 4);

		if (m_bPlayerControlled)
			ADD_STRING(buildings, State, "Wrangled");

		ADD_STRING(buildings, Ammo, "Ammo: " + std::to_string(m_iAmmoShells) + " / " + std::to_string(m_iMaxAmmoShells));

		if (level == 3)
			ADD_STRING(buildings, Ammo, "Rockets: " + std::to_string(m_iAmmoRockets) + " / 20");
	break;
	}

	case ClassID_CObjectDispenser:
	{
		using namespace Offsets::IBaseEntity::Dispenser;

		auto m_hHealingTargets = pObject->GetEntProp<CUtlVector<IBaseHandle>, PropType::Recv>("m_iMiniBombCounter", m_iMiniBombCounter__To__m_hHealingTargets);

		if (int num_players = m_hHealingTargets->Count())
			ADD_STRING(buildings, State, "Healing: " + std::to_string(num_players) + " Players");

		int num_ammo = *pObject->GetEntProp<int, PropType::Recv>("m_iState", m_iState__To__m_iAmmoMetal);
		ADD_STRING(buildings, State, "Ammo: " + std::to_string(num_ammo));
		break;
	}

	default:
		break;
	}

	espdata.push_back(std::move(data));
}

void ProcessEntity(const MyClientCacheList& cache, const Vector& myOrg)
{
	IClientShared* pEnt = cache.pEnt;
	if (pEnt->IsDormant())
		return;

	const Vector& org = pEnt->GetAbsOrigin();
	float dist = org.DistTo(myOrg);

	EntFlag flag = cache.flag;

	switch (flag)
	{
	case EntFlag::Player:
	{
		int team = pEnt->GetTeam() - 2;
		if (team < 0)
			return;

		auto& PlayerESP = EMenu.player_esp[team];
		if (!PlayerESP.base.m_bActive)
			return;

		if (PlayerESP.base.m_flMaxDist > 50.0f && PlayerESP.base.m_flMaxDist < dist)
			return;

		if (PlayerESP.m_bIgnoreCloak && reinterpret_cast<ITFPlayer*>(pEnt)->InCond(TF_COND_STEALTHED))
			return;

		ESPData data{ };
		data.pEnt = pEnt;
		data.dist = dist;

		ADD_STRING(PlayerESP.base, Distance, "Distance: " + std::to_string(dist) + " HU");
		ADD_STRING(PlayerESP, Team, m_szTeams[team]);

		data.type = IESPHack::ESPType_t::Player;
		data.color_offset = team;

		ProcessPlayer(data, reinterpret_cast<ITFPlayer*>(pEnt));
		return;
	}
	[[fallthrough]];
	case EntFlag::Building:
	{
		ClassID id = static_cast<ClassID>(pEnt->GetClientClass()->m_ClassID);
		switch (id)
		{
		case ClassID_CObjectDispenser:
		{
			int team = pEnt->GetTeam() - 2;
			if (team < 0)
				break;

			auto& BuildingESP = EMenu.building_esp[team];
			if (!BuildingESP.base.m_bActive)
				break;

			if (BuildingESP.base.m_flMaxDist > 50.0f && BuildingESP.base.m_flMaxDist < dist)
				break;

			ESPData data{ };
			data.pEnt = pEnt;
			data.dist = dist;

			ADD_STRING(BuildingESP, Team, m_szTeams[team]);
			ADD_STRING(BuildingESP.base, Name, "Dispenser");

			data.type = IESPHack::ESPType_t::Building;
			data.color_offset = team;

			ProcessBuilding(data, reinterpret_cast<IBaseObject*>(pEnt), id);
			break;
		}

		case ClassID_CObjectTeleporter:
		{
			int team = pEnt->GetTeam() - 2;
			if (team < 0)
				break;

			auto& BuildingESP = EMenu.building_esp[team];
			if (!BuildingESP.base.m_bActive)
				break;

			if (BuildingESP.base.m_flMaxDist > 50.0f && BuildingESP.base.m_flMaxDist < dist)
				break;

			ESPData data{ };
			data.pEnt = pEnt;
			data.dist = dist;

			ADD_STRING(BuildingESP, Team, m_szTeams[team]);
			ADD_STRING(BuildingESP.base, Name, "Teleporter");	//TODO : m_iObjectMode!

			data.type = IESPHack::ESPType_t::Building;
			data.color_offset = team;

			ProcessBuilding(data, reinterpret_cast<IBaseObject*>(pEnt), id);
			break;
		}

		case ClassID_CObjectSentrygun:
		{
			int team = pEnt->GetTeam() - 2;
			if (team < 0)
				break;

			auto& BuildingESP = EMenu.building_esp[team];
			if (!BuildingESP.base.m_bActive)
				break;

			if (BuildingESP.base.m_flMaxDist > 50.0f && BuildingESP.base.m_flMaxDist < dist)
				break;

			ESPData data{ };
			data.pEnt = pEnt;
			data.dist = dist;

			ADD_STRING(BuildingESP, Team, m_szTeams[team]);
			ADD_STRING(BuildingESP.base, Name, "Sentrygun");

			data.type = IESPHack::ESPType_t::Building;
			data.color_offset = team;

			ProcessBuilding(data, reinterpret_cast<IBaseObject*>(pEnt), id);
			break;
		}
		}
		return;
	}
	[[fallthrough]];
	case EntFlag::Extra:
	{
		ClassID id = static_cast<ClassID>(pEnt->GetClientClass()->m_ClassID);
		switch (id)
		{
		case ClassID_CTFProjectile_Rocket:
		case ClassID_CTFProjectile_SentryRocket:
		{
			auto& objects = EMenu.objects_esp;
			if (!objects.base.m_bActive || !objects.m_bDrawRockets)
				break;

			if (objects.base.m_flMaxDist > 50.0f && objects.base.m_flMaxDist < dist)
				break;

			ESPData data{ };
			data.pEnt = pEnt;
			data.dist = dist;

			data.color.SetColor(objects.base.m_iDrawColor[0],
				objects.base.m_iDrawColor[1],
				objects.base.m_iDrawColor[2],
				objects.base.m_iDrawColor[3]);

			ADD_STRING(objects.base, Name, "Rocket!");
			espdata.push_back(data);
			break;
		}

		case ClassID_CTFGrenadePipebombProjectile:
		{
			auto& objects = EMenu.objects_esp;
			if (!objects.base.m_bActive || !objects.m_bDrawStickies)
				break;

			if (*pEnt->GetEntProp<int, PropType::Recv>("m_iType") != 1)
				break;

			ESPData data{ };
			data.pEnt = pEnt;
			data.dist = dist;

			data.color.SetColor(objects.base.m_iDrawColor[0],
				objects.base.m_iDrawColor[1],
				objects.base.m_iDrawColor[2],
				objects.base.m_iDrawColor[3]);

			ADD_STRING(objects.base, Name, "Stickie!");
			espdata.push_back(data);
			break;
		}
		}
		return;
	}
	
	}

	auto& objects = EMenu.objects_esp;
	if (!objects.m_bDrawPacks)
		return;

	if (pEnt->IsAmmoPack())
	{
		ESPData data{ };
		data.pEnt = pEnt;
		data.dist = dist;

		ADD_STRING(EMenu.objects_esp.base, Name, "Ammo pack");

		data.color.SetColor(objects.base.m_iDrawColor[0],
							objects.base.m_iDrawColor[1],
							objects.base.m_iDrawColor[2],
							objects.base.m_iDrawColor[3]);

		espdata.push_back(data);
	}
	else if (pEnt->IsHealthPack())
	{
		ESPData data{ };
		data.pEnt = pEnt;
		data.dist = dist;

		ADD_STRING(EMenu.objects_esp.base, Name, "Healthkit");

		data.color.SetColor(objects.base.m_iDrawColor[0],
							objects.base.m_iDrawColor[1],
							objects.base.m_iDrawColor[2],
							objects.base.m_iDrawColor[3]);

		espdata.push_back(data);
	}
}