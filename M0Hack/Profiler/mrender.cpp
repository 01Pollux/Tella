
#include "mprofiler.hpp"
#include "mdefines.hpp"

#include "ImGui/imgui_helper.h"
#include "Helper/Format.hpp"

#include <sstream>
#include <fstream>

/*
> Cheat Profiling
	> Aimbot
		Count:
		Min: 
		Max: 
		Avg: 
		Total Time:
		Total Avg:
		> Calls:
			{	<Sortable> + <Deleteable>	}
			|	nanoSec		|	microSec	|	milliSec	|	Sec			|
			|				|				|				|				|

*/

struct M0SnapShot
{
public:
	struct M0Data
	{
		M0PROFILER_TAG_NAME Name;
		M0PROFILER_RECORD Calls;

		size_t count() const noexcept
		{
			return Calls.size();
		}
	};
	std::vector<M0Data> records;

	bool should_draw() const noexcept
	{
		return !records.empty();
	}

	void clear() noexcept
	{
		records.clear();
	}

	void record(const M0PROFILER_MAP& map)
	{
		for (const auto& [name, records] : map)
			this->records.emplace_back(name, records);
	}

	void save(M0PROFILER_GROUP group)
	{
		M0PROFILER_MAP& map = M0Profiler::GetDataMap(group);
		map.clear();

		for (auto& data : records)
			map[data.Name] = data.Calls;
	}

	void export_and_clear(M0PROFILER_GROUP group)
	{
		std::string time = FormatTime("___{0:%g}_{0:%h}_{0:%d}_{0:%H}_{0:%OM}_{0:%OS}"sv);
		IFormatterT fmt("{}/{}{} ({}).txt"sv);

		M0PROFILER_MAP& map = M0Profiler::GetDataMap(group);
		constexpr M0PROFILER_FLAGS flags = bitmask::to_mask(M0PROFILER_FLAGS_::Stream_Seek_Beg, M0PROFILER_FLAGS_::Clear_State);

		int num = 0;
		for (auto& data : records)
		{
			map[data.Name] = std::move(data.Calls);

			M0Profiler::OutputToStream(
				group,
				fmt(
					M0PROFILER_OUT_STREAM,
					M0PROFILE_NAMES[static_cast<std::underlying_type_t<M0PROFILER_GROUP>>(group)],
					time,
					num
				),
				flags
			);

			num++;
		}

		records.clear();
	}

	void draw();

public:
	struct M0SingleData
	{
		M0PROFILER_NANO_SECONDS Min{ INT_MAX };
		M0PROFILER_NANO_SECONDS Max{ };
		M0PROFILER_NANO_SECONDS Total{ };
	};
};


void M0SnapShot::draw()
{
	using namespace std::chrono;

	constexpr ImGuiTableFlags talble_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_NoHostExtendX;
	const float IMGUI_CHAR_LINE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();;
	const float IMGUI_CHAR_WIDTH = ImGui::CalcTextSize("A").x;

	for (auto& [name, records] : records)
	{
		if (!name)
			continue;

		ImGui::PushID(records.data());

		if (ImGui::TreeNode(name))
		{
			M0SnapShot::M0SingleData incoming;

			for (const M0PROFILER_MICRO_SECONDS& time : records)
			{
				if (time < incoming.Min)
					incoming.Min = duration_cast<M0PROFILER_NANO_SECONDS>(time);

				if (time > incoming.Max)
					incoming.Max = duration_cast<M0PROFILER_NANO_SECONDS>(time);

				incoming.Total += duration_cast<M0PROFILER_NANO_SECONDS>(time);
			}

			if (incoming.Total.count() && ImGui::BeginTable("DATA_INFO", 3, talble_flags))
			{
				class ImDrawProfile
				{
				public:

					void draw() const
					{
						ImGui::TableNextRow();
						{
							if (ImGui::TableNextColumn())
								ImGui::TextUnformatted(Name);

							if (ImGui::TableNextColumn())
								ImGui::Text(FirstFormat, FirstValue);

							if (SecondFormat && ImGui::TableNextColumn())
								ImGui::Text(SecondFormat, SecondValue);
						}
					}

					ImDrawProfile(const char* name,
								  const char* first_format, long long first_value,
								  const char* second_format = nullptr, long long second_value = 0) noexcept :
								  Name(name),
								  FirstFormat(first_format), FirstValue(first_value),
								  SecondFormat(second_format), SecondValue(second_value)
								  { }

				private:
					const char* Name;

					const char* FirstFormat;
					long long	FirstValue;

					const char* SecondFormat = nullptr;
					long long	SecondValue{ };
				};

				const ImDrawProfile datas[]
				{
					{ "Reference Count", "%d", records.size() },
					{ "Min", "%lld ns", incoming.Min.count(), "%lld ms", duration_cast<M0PROFILER_MICRO_SECONDS>(incoming.Min).count() },
					{ "Max", "%lld ns", incoming.Max.count(), "%lld ms", duration_cast<M0PROFILER_MICRO_SECONDS>(incoming.Max).count() },
					{ "Avg", "%lld ns", ((incoming.Min + incoming.Max) / 2).count(), "%lld ms", duration_cast<M0PROFILER_MICRO_SECONDS>((incoming.Min + incoming.Max) / 2).count() },
					{ "Total Time", "%lld ns", (incoming.Total).count(), "%lld ms", duration_cast<M0PROFILER_MICRO_SECONDS>(incoming.Total).count() },
					{ "Total Avg", "%lld ns", (incoming.Total / records.size()).count(), "%lld ms", duration_cast<M0PROFILER_MICRO_SECONDS>(incoming.Total / records.size()).count() },
				};

				for (const auto& to_draw : datas)
					to_draw.draw();

				ImGui::EndTable();

				if (ImGui::TreeNodeEx("Display Calls", ImGuiTreeNodeFlags_Bullet))
				{
					ImGui::Help("Double Left-click on an entry to erase it.\n"
									"Click Save to override the original results.");

					if (ImGui::BeginTable("Calls", 4, talble_flags | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Reorderable, { 0.0f, IMGUI_CHAR_LINE_HEIGHT * 15.f }))
					{
						ImGui::TableSetupColumn("Nano-seconds", ImGuiTableColumnFlags_PreferSortAscending);
						ImGui::TableSetupColumn("Micro-seconds (Approx)", ImGuiTableColumnFlags_PreferSortAscending);
						ImGui::TableSetupColumn("Milli-seconds (Approx)", ImGuiTableColumnFlags_NoSort);
						ImGui::TableSetupColumn("Seconds (Approx)", ImGuiTableColumnFlags_NoSort);
						ImGui::TableSetupScrollFreeze(0, 1);
						ImGui::TableHeadersRow();
						
						if (ImGuiTableSortSpecs* SortSpec = ImGui::TableGetSortSpecs();
							SortSpec && SortSpec->SpecsDirty)
						{
							if (records.size())
							{
								const ImGuiSortDirection dir = SortSpec->Specs->SortDirection == ImGuiSortDirection_Ascending;

								std::sort(records.begin(), records.end(),
									[dir](const M0PROFILER_MICRO_SECONDS& a, const M0PROFILER_MICRO_SECONDS& b)
									{
										return dir ? a < b : b < a;
									}
								);
							}

							SortSpec->SpecsDirty = false;
						}
						
						char buf[24];
						bool invalidated = false;

						ImGuiListClipper clipper;
						clipper.Begin(records.size());
						while (clipper.Step() && !invalidated)
						{
							for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
							{
								bool selected;

								ImGui::TableNextRow();

								ImGui::TableNextColumn();
								snprintf(buf, sizeof(buf), "%lld", duration_cast<M0PROFILER_NANO_SECONDS>(records[i]).count());
								if (ImGui::Selectable(buf, &selected, ImGuiSelectableFlags_AllowDoubleClick))
								{
									if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
									{
										records.erase(records.cbegin() + i);
										invalidated = true;
										break;
									}
								}

								ImGui::TableNextColumn();
								ImGui::Text("%lld", records[i].count());

								ImGui::TableNextColumn();
								ImGui::Text("%lld", duration_cast<milliseconds>(records[i]).count());

								ImGui::TableNextColumn();
								ImGui::Text("%lld", duration_cast<seconds>(records[i]).count());
							}
						}

						ImGui::EndTable();
					}

					ImGui::TreePop();
				}
			}

			ImGui::Dummy({ 0.f, 20 });

			ImGui::TreePop();
		}

		ImGui::PopID();

		ImGui::Separator();
		ImGui::Dummy({ 0.f, 6.f });
	}
}

void M0Profiler::RenderToImGui(bool* p_open)
{
	ImGui::SetNextWindowSize({840.0f, 754.f}, ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("M0Profiler Manager", p_open))
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginTabBar("##MPROFILER", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll))
	{
		constexpr M0PROFILER_GROUP_T profile_groups = static_cast<M0PROFILER_GROUP_T>(M0PROFILER_GROUP::COUNT);

		static bool tabs_to_open[profile_groups]{ };
		static M0SnapShot snapshots[profile_groups];

		if (ImGui::TabItemButton("Tabs", ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip))
			ImGui::OpenPopup("##TabsNames");

		if (ImGui::BeginPopup("##TabsNames"))
		{
			for (M0PROFILER_GROUP_T i = 0; i < profile_groups; i++)
				ImGui::Checkbox(M0PROFILE_NAMES[i], &tabs_to_open[i]);
			
			ImGui::EndPopup();
		}

		for (M0PROFILER_GROUP_T i = 0; i < profile_groups; i++)
		{
			if (!tabs_to_open[i])
				continue;

			const M0PROFILER_GROUP group = static_cast<M0PROFILER_GROUP>(i);

			if (ImGui::BeginTabItem(M0PROFILE_NAMES[i], &tabs_to_open[i]))
			{
				const bool state = M0Profiler::IsActive(group);

				static ImVec4 color[]
				{
					{ 1.0f, 0.0f, 0.0f, 1.0f },
					{ 0.0f, 1.0f, 0.0f, 1.0f },
				};

				ImGui::Dummy({ 0.f, 6.f });

				ImGui::PushStyleColor(ImGuiCol_Text, color[state ? 1 : 0]);
				ImGui::BulletText("State: %sctive", state ? "A" : "Ina");
				ImGui::PopStyleColor();

				ImGui::Dummy({ 0.f, 6.f });

				if (ImGui::Button("Toggle"))
					M0Profiler::Toggle(group, !state);

				ImGui::SameLine();
				if (ImGui::Button("Clear"))
					snapshots[i].clear();

				ImGui::SameLine();
				if (ImGui::Button("Reset"))
					M0Profiler::Reset(group);

				ImGui::SameLine();
				if (ImGui::Button("Record"))
				{
					auto& infos = M0Profiler::GetDataMap(group);
					if (!infos.empty())
						snapshots[i].record(M0Profiler::GetDataMap(group));
				}

				if (ImGui::Button("Save"))
					snapshots[i].save(group);

				ImGui::SameLine();
				if (ImGui::Button("Export And Clear"))
					snapshots[i].export_and_clear(group);

				if (snapshots[i].should_draw())
					snapshots[i].draw();

				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}
	
	ImGui::End();
}


#include "ConVar.hpp"

static void DumpProfiler(const CCommand& args)
{
	constexpr M0PROFILER_FLAGS flags = bitmask::to_mask(M0PROFILER_FLAGS_::Stream_Seek_Beg, M0PROFILER_FLAGS_::Clear_State);
	std::string time = FormatTime("__{0:%g}_{0:%h}_{0:%d}_{0:%H}_{0:%OM}_{0:%OS}.txt"sv);
	IFormatter fmt("{}/{}{}.log");

	const int type = atoi(args[1]);

	switch (type)
	{
	case -1:
	{
		ReplyToCCmd(color::names::violet, "Dumping all Profilers"sv);
		for (M0PROFILER_GROUP_T i = 0; i < static_cast<M0PROFILER_GROUP_T>(M0PROFILER_GROUP::COUNT); ++i)
		{
			M0Profiler::OutputToStream(
				static_cast<M0PROFILER_GROUP>(i),
				fmt(M0PROFILER_OUT_STREAM, M0PROFILE_NAMES[i], time).c_str(),
				flags
			);
		}
		break;
	}
	case 0:
	{
		ReplyToCCmd(color::names::red, "Type -1 to dump all profilers, or one of the followings:"sv);
		for (int i = 0; i < static_cast<size_t>(M0PROFILER_GROUP::COUNT); i++)
			ReplyToCCmd(color::names::red, "{}: \"{}\""sv, i, M0PROFILE_NAMES[i]);

		break;
	}
	default:
	{
		if (type >= 0 && type < static_cast<M0PROFILER_GROUP_T>(M0PROFILER_GROUP::COUNT))
		{
			M0Profiler::OutputToStream(
				static_cast<M0PROFILER_GROUP>(type),
				fmt(M0PROFILER_OUT_STREAM, M0PROFILE_NAMES[type], time).c_str(),
				flags
			);

			ReplyToCCmd(color::names::violet, "Dumping \"%s\""sv, M0PROFILE_NAMES[type]);
		}
		break;
	}
	}
}
M01_CONCOMMAND(dump_mprof, DumpProfiler, "Dump M0Profiler's results manually");
