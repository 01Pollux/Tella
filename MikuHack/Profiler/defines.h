#pragma once

#include <memory>
#include <unordered_map>
#include <chrono>


namespace M0ProfileHelper
{
	class	M0Timer;
	class	M0PROFILE_STATUS;
	class	M0PROFILER_MAPS;
	struct	M0PROFILER_RECORD;
};

class M0Profiler;
struct M0PROFILER_RECORD;


using M0PROFILER_GROUP_NAME		= const char*;
using M0PROFILER_TAG_NAME		= const char*;
using M0PROFILER_INSTANCE		= M0Profiler*;
using M0PROFILER_NCALLS			= size_t;


using M0PROFILER_TIMER			= M0ProfileHelper::M0Timer;
using M0PROFILER_NANO_SECONDS	= std::chrono::nanoseconds;
using M0PROFILER_MICRO_SECONDS	= std::chrono::microseconds;


using M0PROFILER_MAP			= std::unordered_map<M0PROFILER_TAG_NAME, std::unique_ptr<M0ProfileHelper::M0PROFILER_RECORD>>;
using M0PROFILER_PMAP			= std::unique_ptr<M0PROFILER_MAP>;