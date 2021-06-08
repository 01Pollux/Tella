#pragma once

#include <map>
#include <vector>
#include <chrono>
#include <iostream>


namespace M0ProfileHelper
{
	class	M0PROFILE_STATUS;
	class	M0PROFILER_MAPS;
};

class M0Profiler;


using M0PROFILER_GROUP_NAME = const char*;
using M0PROFILER_TAG_NAME = const char*;
using M0PROFILER_INSTANCE = M0Profiler*;
using M0PROFILER_NCALLS = size_t;


using M0PROFILER_CLOCK = std::chrono::steady_clock;
using M0PROFILER_TIME_POINT = M0PROFILER_CLOCK::time_point;
using M0PROFILER_NANO_SECONDS = std::chrono::nanoseconds;
using M0PROFILER_MICRO_SECONDS = std::chrono::microseconds;
using M0PROFILER_RECORD = std::vector<M0PROFILER_MICRO_SECONDS>;


using M0PROFILER_MAP = std::map<M0PROFILER_TAG_NAME, M0PROFILER_RECORD>;
using M0PROFILER_PMAP = std::unique_ptr<M0PROFILER_MAP>;