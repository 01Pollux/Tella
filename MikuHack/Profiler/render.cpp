
#include "mprofiler.h"
#include "defines.h"

#include <../ImGui/imgui.h>

struct M0SnapShot
{
	struct TimeData
	{
		M0PROFILER_TAG_NAME name;
		M0PROFILER_MICRO_SECONDS min{ INT_MAX }, max{ }, total{ };
		std::vector<M0PROFILER_MICRO_SECONDS> times_snapshot;
	};
	std::vector<TimeData> data;

	void copy(const M0PROFILER_MAP& map)
	{
		this->data.clear();
		this->data.reserve(map.size());
		
		for (const auto& [name, records] : map)
		{
			M0PROFILER_MICRO_SECONDS min{ INT_MAX }, max{ }, total{ };
			std::vector<M0PROFILER_MICRO_SECONDS> stamps(records.size());

			for (const M0PROFILER_MICRO_SECONDS& time : records)
			{
				if (time > min)
					min = time;
				if (time < max)
					max = time;

				total += time;
				stamps.emplace_back(time);
			}

			this->data.emplace_back(name, min, max, total, std::move(stamps));
		}
	}
};


void M0ProfileHelper::RenderToImGui(bool* p_open)
{
	ImGui::SetNextWindowSize({640.f, 452.f}, ImGuiCond_Always);
	if (!ImGui::Begin("Profiler Manager", p_open, ImGuiWindowFlags_NoResize))
	{
		ImGui::End();
		return;
	}

	using M0Type = std::underlying_type_t<M0PROFILER_GROUP>;
	constexpr M0Type count = static_cast<M0Type>(M0PROFILER_GROUP::COUNT);
	static M0SnapShot snapshots[count]{ };

	for (M0Type type = 0; type < count; type++)
	{
		M0PROFILER_GROUP_NAME group_name = M0PROFILE_NAMES[type];
		M0PROFILER_GROUP group = static_cast<M0PROFILER_GROUP>(type);

		if (constexpr ImGuiTreeNodeFlags node_flag =
			  ImGuiTreeNodeFlags_Bullet
			| ImGuiTreeNodeFlags_OpenOnDoubleClick
			| ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_SpanFullWidth
			| ImGuiTreeNodeFlags_SpanAvailWidth;

			ImGui::TreeNode(group_name))
		{
			static bool should_draw = false;

			bool state = M0Profiler::IsActive(group);

			static ImVec4 color[2]
			{
				{1.0f, 0.0f, 0.0f, 1.0f},
				{0.0f, 1.0f, 0.0f, 1.0f},
			};

			ImGui::TextColored(color[state ? 1 : 0], "State: %sctive", state ? "A" : "Ina");
			ImGui::SameLine();
			if (ImGui::Button("Toggle"))
			{
				state = !state;
				M0Profiler::Toggle(group, state);
			}

			ImGui::SameLine();
			if (ImGui::Button("Clear"))
				should_draw = false;

			ImGui::SameLine();
			if (ImGui::Button("Take Snapshot"))
			{
				const M0PROFILER_MAP& datamap = M0Profiler::GetDataMap(group);
				snapshots[type].copy(datamap);

				should_draw = true;
			}

			if (should_draw)
			{
				const auto& snapshot = snapshots[type].data;

				if (ImGui::BeginTable("Snapshot Table", 2))
				{
					constexpr ImGuiTableColumnFlags default_flags = ImGuiTableColumnFlags_WidthFixed;

					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Function Name", default_flags);
					ImGui::TableSetupColumn("Infos", default_flags | ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableHeadersRow();


					for (const auto& data : snapshot)
					{
						bool open = ImGui::TreeNodeEx(data.name, ImGuiTreeNodeFlags_SpanFullWidth);

						ImGui::TextUnformatted("---");
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("---");
						ImGui::TableNextColumn();

						if (open)
						{
							for (const M0PROFILER_MICRO_SECONDS& time : data.times_snapshot)
							{
								ImGui::PushID(reinterpret_cast<const void*>(&time));

								constexpr ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet |
																		  ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
								if (ImGui::TreeNodeEx("##SNAPSHOT_DATA", node_flags))
								{
									ImGui::TableNextColumn();
									ImGui::TextUnformatted(data.name);
									ImGui::TableNextColumn();
									ImGui::Text("%6d", time.count());

									ImGui::TreePop();
								}

								ImGui::PopID();
							}

							ImGui::TreePop();
						}
					}
				}
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();
}