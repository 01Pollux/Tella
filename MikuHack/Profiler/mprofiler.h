#pragma once


#include "defines.h"
#include <type_traits>

enum class M0PROFILER_GROUP : char8_t
{
	ANY,
	GLOBAL_VHOOK_ANY,
	GLOBAL_DHOOK_ANY,

	HOOK_PAINT_TRAVERSE,
	HOOK_CREATE_MOVE,
	HOOK_FRAME_STAGE_NOTIFY,
	HOOK_SEND_NETMESSAGE,

	DISPATCH_IMGUI,
	CHEAR_PROFILE,

	COUNT,
};

constexpr M0PROFILER_GROUP_NAME M0PROFILE_NAMES[]
{
	"Generics",

	"Virtual Function Hooks",
	"Detoured Function Hooks",

	"vgui::Panel::PaintTravese",
	"ClientDLL::CreateMove",
	"ClientDLL::FrameStageNotify",
	"ClientDLL::SendNetMsg",

	"Dispatching ImGui Items",
	"Profiling Cheats"
};

constexpr const char* M0PROFILER_OUT_STREAM = ".\\Miku\\Log\\Profiler\\";

enum class M0PROFILER_FLAGS : char8_t
{
	EMPTY				=	0,
	RESULTS_ONLY		=	(1 << 0),
};


constexpr bool operator&(const M0PROFILER_FLAGS& a, const M0PROFILER_FLAGS& b)
{
	using true_type = std::underlying_type_t<M0PROFILER_FLAGS>;
	return static_cast<true_type>(a) & static_cast<true_type>(b);
}

constexpr bool operator|(const M0PROFILER_FLAGS& a, const M0PROFILER_FLAGS& b)
{
	using true_type = std::underlying_type_t<M0PROFILER_FLAGS>;
	return static_cast<true_type>(a) | static_cast<true_type>(b);
}


namespace M0ProfileHelper
{
	class M0Timer
	{
		using hrc = std::chrono::high_resolution_clock;
		hrc::time_point	start_time;

	public:
		void Start() noexcept
		{
			start_time = hrc::now();
		}

		M0PROFILER_NANO_SECONDS Time() const noexcept
		{
			using namespace std::chrono;

			hrc::time_point end = hrc::now();
			return time_point_cast<M0PROFILER_NANO_SECONDS>(end) - start_time;
		}
	};
	
	class MProfileManager;
}


class M0Profiler
{
	M0PROFILER_GROUP			group;
	M0PROFILER_TAG_NAME			name;
	M0PROFILER_TIMER			timer;
	
	friend class M0ProfileHelper::MProfileManager;

public:
	M0Profiler(M0PROFILER_TAG_NAME tag_name, M0PROFILER_GROUP group);
	~M0Profiler();
	
	M0PROFILER_TAG_NAME	GetSectionName() const noexcept { return name; }


public:
	//	Start profiling a session(s)
	static void				Start(M0PROFILER_GROUP group);
	static void				Start(M0PROFILER_GROUP groups[], size_t num);
	//	Stop profiling a session(s)
	static void				Stop(M0PROFILER_GROUP group);
	static void				Stop(M0PROFILER_GROUP groups[], size_t num);

	//	print the current state to a file
	static void				OutputToFile(M0PROFILER_GROUP group, std::ofstream& stream, M0PROFILER_FLAGS flags = M0PROFILER_FLAGS::EMPTY);
	//	get current state
	static M0PROFILER_MAP&	GetOutputMap(M0PROFILER_GROUP group);
	//	reset current state
	static void				Reset(M0PROFILER_GROUP group);


public:
	M0Profiler(const M0Profiler&) = delete;
	M0Profiler& operator=(const M0Profiler&) = delete;
	M0Profiler(M0Profiler&&) = default;
	M0Profiler& operator=(M0Profiler&&) = delete;
};


