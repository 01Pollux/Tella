
#include "mprofiler.h"

#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>


constexpr size_t M0PROFILER_MAX_NCALLS = 34086U;

namespace M0ProfileHelper
{
	struct M0PROFILER_RECORD
	{
		using MIN_MAX_PAIR = std::pair<M0PROFILER_MICRO_SECONDS, M0PROFILER_MICRO_SECONDS>;
		using MIN_MAX_MAP = std::unordered_map<M0PROFILER_TAG_NAME, MIN_MAX_PAIR>;

		struct SINGLE_INFO
		{
			M0PROFILER_TAG_NAME			unique_name;
			M0PROFILER_MICRO_SECONDS	now;
		};

		std::vector<SINGLE_INFO> data;

		static constexpr MIN_MAX_PAIR default_min_max = { M0PROFILER_MICRO_SECONDS(INT_MAX) , { } };
		mutable MIN_MAX_MAP min_max;

		M0PROFILER_NCALLS		times{ };


		void CalcMinMax() noexcept
		{
			for (auto& [name, time] : data)
			{
				auto& [min, max] = min_max[name];

				if (time < min)
					min = time;

				if (time > max)
					max = time;
			}
		}

		static double Perc(const M0PROFILER_MICRO_SECONDS& cur, const M0PROFILER_MICRO_SECONDS& avg) noexcept
		{
			using double_time = std::chrono::duration<double>;

			auto freq = std::chrono::duration_cast<double_time>(cur).count() / std::chrono::duration_cast<double_time>(avg).count();
			return (freq) * 100.0;
		}
	};

	class M0PROFILE_STATUS
	{
		using true_type = std::underlying_type_t<M0PROFILER_GROUP>;
		bool stats[static_cast<true_type>(M0PROFILER_GROUP::COUNT)]{ };

	public:
		bool& operator[](M0PROFILER_GROUP group) noexcept
		{
			return stats[static_cast<true_type>(group)];
		}
		bool& operator[](size_t group) noexcept
		{
			return stats[group];
		}
	};

	class M0PROFILER_MAPS
	{
		using true_type = std::underlying_type_t<M0PROFILER_GROUP>;
		M0PROFILER_MAP profilers[static_cast<true_type>(M0PROFILER_GROUP::COUNT)];

	public:
		M0PROFILER_MAP& operator[](M0PROFILER_GROUP group) noexcept
		{
			return profilers[static_cast<true_type>(group)];
		}
	};


	M0PROFILE_STATUS	status;
	M0PROFILER_MAPS		profilers;


	constexpr M0PROFILER_GROUP_NAME GetName(M0PROFILER_GROUP group)
	{
		return M0PROFILE_NAMES[static_cast<std::underlying_type_t<M0PROFILER_GROUP>>(group)];
	}

	bool IsActive(M0PROFILER_GROUP group) noexcept
	{
		return status[group];
	}

	

	class MProfileManager
	{
	public:
		void OnSectionStart(M0PROFILER_INSTANCE instance) noexcept;
		void OnSectionEnd(const M0PROFILER_MICRO_SECONDS time, M0PROFILER_INSTANCE instance);
	} g_MProfileManager;

};

using namespace M0ProfileHelper;


M0Profiler::M0Profiler(M0PROFILER_TAG_NAME name, M0PROFILER_GROUP group) : name(name), group(group)
{
	if (IsActive(group))
		g_MProfileManager.OnSectionStart(this);
}

M0Profiler::~M0Profiler()
{
	if (IsActive(group))
		g_MProfileManager.OnSectionEnd(std::chrono::duration_cast<M0PROFILER_MICRO_SECONDS>(timer.Time()), this);
}


void M0Profiler::Start(M0PROFILER_GROUP group)
{
	status[group] = true;
}

void M0Profiler::Start(M0PROFILER_GROUP groups[], size_t num)
{
	for (size_t i = 0; i < num; i++)
		status[groups[i]] = true;
}

void M0Profiler::Stop(M0PROFILER_GROUP group)
{
	status[group] = false;
}

void M0Profiler::Stop(M0PROFILER_GROUP groups[], size_t num)
{
	for (size_t i = 0; i < num; i++)
		status[groups[i]] = false;
}


void M0Profiler::OutputToFile(M0PROFILER_GROUP group, std::ofstream& stream, M0PROFILER_FLAGS flags)
{
	auto& profile = profilers[group];

	stream << std::setw(10);

	stream << "Summary for \"" << GetName(group) << "\":\n";

	bool dont_record_childrens = flags & M0PROFILER_FLAGS::RESULTS_ONLY;

	for (auto& [section, info] : profile)
	{
		stream << "\n----------------------------------------------------------------------------------------------\n\n";

		info->CalcMinMax();

		const auto& [min, max] = info->min_max[section];
		const M0PROFILER_NANO_SECONDS avg = (max + min) / 2;

		M0PROFILER_NANO_SECONDS total{};

		std::cout << "Section: " << section << '\n';
		stream << section
			<< "\tReference Count: "
			<< info->times
			<< "\tAvg: " << avg.count()
			<< "ns\tMin: " << min.count()
			<< "ns\tMax: " << max.count() << "ns\n\n";

		for (auto& child = info->data;
			auto & [name, time]: child)
		{
			total += time;

			if (!dont_record_childrens)
			{
				stream <<
					"\t> \"" << name << "\": " <<
					"\t( Elapsed: " << time.count() << "ns  /  ~" << std::chrono::duration_cast<M0PROFILER_MICRO_SECONDS>(time).count() << " microsec )\n";
			}
		}

		auto mt = (total / info->data.size());

		stream << "\nTotal Time Spent: " << total.count() << "ns  /  ~" << std::chrono::duration_cast<M0PROFILER_MICRO_SECONDS>(mt).count() << "microsec \n"
			<< "Average: " << mt.count() << "ns  /  ~" << std::chrono::duration_cast<M0PROFILER_MICRO_SECONDS>(mt).count() << "microsec\n\n";
	}

	stream << "\n----------------------------------------------------------------------------------------------" << std::endl;
}

inline M0PROFILER_MAP& M0Profiler::GetOutputMap(M0PROFILER_GROUP group)
{
	return profilers[group];
}

void M0Profiler::Reset(M0PROFILER_GROUP group)
{
	profilers[group].clear();
}

inline void MProfileManager::OnSectionStart(M0PROFILER_INSTANCE instance) noexcept
{
	instance->timer.Start();
}

void MProfileManager::OnSectionEnd(const M0PROFILER_MICRO_SECONDS time, M0PROFILER_INSTANCE instance)
{
	const char* name = instance->name;
	auto& profile = profilers[instance->group];
	auto iter = profile.find(name);

	if (iter == profile.end())
	{
		iter = profile.insert(std::make_pair(name, std::make_unique<M0PROFILER_RECORD>())).first;
		iter->second->min_max[name] = M0PROFILER_RECORD::default_min_max;
	}

	auto& record = iter->second;

	if (record->data.size() < M0PROFILER_MAX_NCALLS)
	{
		record->data.push_back({ name, time });
		++record->times;
	}
}