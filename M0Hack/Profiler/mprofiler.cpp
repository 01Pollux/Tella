
#include "mprofiler.hpp"

#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>


constexpr size_t M0PROFILER_MAX_NCALLS = 2'097'152U;
constexpr size_t M0PROFILER_MIN_CAPACITY = 2'048;

namespace M0ProfileHelper
{
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

	void OnSectionEnd(const M0PROFILER_INSTANCE instance, const M0PROFILER_TIME_POINT begin_time);
};

using namespace M0ProfileHelper;


M0Profiler::~M0Profiler()
{
	if (IsActive(GroupId))
		OnSectionEnd(this, BeginTime);
}

bool M0Profiler::IsActive(M0PROFILER_GROUP group) noexcept
{
	return status[group];
}

void M0Profiler::Toggle(M0PROFILER_GROUP group, bool state) noexcept
{
	status[group] = state;
}

void M0Profiler::start(M0PROFILER_TAG_NAME name, M0PROFILER_GROUP group)
{
	BeginTime = M0PROFILER_CLOCK::now();

	this->TagName = name;
	this->GroupId = group;
}

void M0Profiler::Start(M0PROFILER_GROUP groups[], size_t num) noexcept
{
	for (size_t i = 0; i < num; i++)
		status[groups[i]] = true;
}

void M0Profiler::Stop(M0PROFILER_GROUP groups[], size_t num) noexcept
{
	for (size_t i = 0; i < num; i++)
		status[groups[i]] = false;
}


bool M0Profiler::OutputToStream(M0PROFILER_GROUP group, std::stringstream& stream, M0PROFILER_FLAGS flags)
{
	auto& profile = profilers[group];
	if (profile.empty())
		return false;

	stream << std::setw(10);

	stream << "Summary for \"" << GetName(group) << "\":\n";

	const bool dont_record_childrens = HasBitSet(flags, M0PROFILER_FLAGS::HIDE_CHILDRENS);

	for (const auto& [name, record] : profile)
	{
		stream << "\n----------------------------------------------------------------------------------------------\n";

		M0PROFILER_MICRO_SECONDS min{ INT_MAX }, max{ }, total{ };

		for (const auto& times = record;
			 const M0PROFILER_MICRO_SECONDS& time : times)
		{
			total += time;

			if (!dont_record_childrens)
			{
				stream << "\t<( Elapsed: "
					<< std::chrono::duration_cast<M0PROFILER_NANO_SECONDS>(time).count()
					<< " ns  |  ~" << time.count() << " microsec )>\n";
			}

			if (time < min)
				min = time;
			if (time > max)
				max = time;
		}

		const size_t num_calls = record.size();
		const M0PROFILER_MICRO_SECONDS avg = (max + min) / 2;
		const M0PROFILER_MICRO_SECONDS mt = (total / num_calls);

		stream
			<< "\nFunction Name: \t"
			<< name

			<< "\nReference Count: \t"
			<< num_calls

			<< "\nAverage Time: \t"
			<< std::chrono::duration_cast<M0PROFILER_NANO_SECONDS>(avg).count()
			<< " ns  |  ~" << avg.count() << " microsec"

			<< "\nMinimum Time: \t"
			<< std::chrono::duration_cast<M0PROFILER_NANO_SECONDS>(min).count()
			<< " ns  |  ~" << min.count() << " microsec"

			<< "\nMaximum Time: \t"
			<< std::chrono::duration_cast<M0PROFILER_NANO_SECONDS>(max).count()
			<< " ns  |  ~" << max.count() << " microsec"

			<< "\nTotal Time: \t"
			<< std::chrono::duration_cast<M0PROFILER_NANO_SECONDS>(total).count()
			<< " ns  |  ~" << total.count() << " microsec"

			<< "\nTotal Average Time: \t"
			<< std::chrono::duration_cast<M0PROFILER_NANO_SECONDS>(mt).count()
			<< " ns  |  ~" << mt.count() << " microsec\n";
	}

	stream << "\n----------------------------------------------------------------------------------------------" << std::endl;

	if (HasBitSet(flags, M0PROFILER_FLAGS::CLEAR_STATE))
		Reset(group);

	if (HasBitSet(flags, M0PROFILER_FLAGS::STREAM_SEEK_BEG))
		stream.seekp(0);

	return true;
}

void M0Profiler::OutputToStream(M0PROFILER_GROUP group, const char* output_name, M0PROFILER_FLAGS flags)
{
	std::stringstream str;
	if (OutputToStream(group, str, flags | M0PROFILER_FLAGS::STREAM_SEEK_BEG))
	{
		std::ofstream stream(output_name, std::ios::app | std::ios::out);
		stream << str.rdbuf();
	}
}

M0PROFILER_MAP& M0Profiler::GetDataMap(M0PROFILER_GROUP group) noexcept
{
	return profilers[group];
}

void M0Profiler::Reset(M0PROFILER_GROUP group) noexcept
{
	profilers[group].clear();
}

void M0ProfileHelper::OnSectionEnd(const M0PROFILER_INSTANCE instance, const M0PROFILER_TIME_POINT begin_time)
{
	const M0PROFILER_TIME_POINT now = M0PROFILER_CLOCK::now();

	M0PROFILER_TAG_NAME name = instance->section_name();
	M0PROFILER_MAP& profiler = profilers[instance->group_id()];
	M0PROFILER_MAP::iterator iter = profiler.find(name);

	if (iter == profiler.end())
	{
		M0PROFILER_RECORD record;
		record.reserve(M0PROFILER_MIN_CAPACITY);
		iter = profiler.emplace(name, std::move(record)).first;
	}

	auto& record = iter->second;

	if (record.size() < M0PROFILER_MAX_NCALLS)
		record.emplace_back(std::chrono::duration_cast<M0PROFILER_MICRO_SECONDS>(now - begin_time));
	else status[instance->group_id()] = false;
}