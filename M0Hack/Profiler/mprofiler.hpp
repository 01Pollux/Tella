#pragma once

#include "mdefines.hpp"
#include "Helper/EnumClass.hpp"
#include <type_traits>

enum class M0PROFILER_GROUP : char8_t
{
	ANY,

	GLOBAL_HOOK_ANY,

	HOOK_PAINT_TRAVERSE,
	HOOK_CREATE_MOVE,
	HOOK_FRAME_STAGE_NOTIFY,
	HOOK_SEND_NETMESSAGE,

	DISPATCH_IMGUI,
	CHEAT_PROFILE,

	DISPATCH_EVENT_CALLBACK,

	COUNT,
};
using M0PROFILER_GROUP_T = std::underlying_type_t<M0PROFILER_GROUP>;

constexpr M0PROFILER_GROUP_NAME M0PROFILE_NAMES[]
{
	"Generics",

	"Function Hooks Callbacks",

	"vguiPanel PaintTravese",
	"ClientDLL CreateMove",
	"ClientDLL FrameStageNotify",
	"ClientDLL SendNetMsg",

	"Dispatching ImGui Items",
	"Profiling Cheats",

	"Dispatch Event Callback"
};

constexpr const char* M0PROFILER_OUT_STREAM = "./Miku/Log/Profiler";
constexpr const char* M0PROFILER_NULL_NAME = nullptr;

enum class M0PROFILER_FLAGS_
{
	Hide_Childrens,		// don't show 'records', only display the results (min, max, avg...)
	Clear_State,		// clear the map after outputing to stringstream
	Stream_Seek_Beg,	// set the stringstream's position back to beginning

	_Highest_Enum,
};
using M0PROFILER_FLAGS = bitmask::mask<M0PROFILER_FLAGS_>;



class M0Profiler
{
public:
	M0Profiler() = default;
	M0Profiler(M0PROFILER_TAG_NAME tag_name, M0PROFILER_GROUP group)
	{
		if (tag_name && IsActive(group))
			start(tag_name, group);
	}
	~M0Profiler();

	void					start(M0PROFILER_TAG_NAME tag_name, M0PROFILER_GROUP group);
	M0PROFILER_TAG_NAME		section_name()		const noexcept { return TagName; }
	M0PROFILER_GROUP		group_id()			const noexcept { return GroupId; }
	M0PROFILER_GROUP_NAME	group_name()		const noexcept { return M0PROFILE_NAMES[static_cast<std::underlying_type_t<M0PROFILER_GROUP>>(GroupId)]; }

public:
	//	is profiling active?
	static bool				IsActive(M0PROFILER_GROUP group) noexcept;

	//	Start/Stop profiling a session(s)
	static void				Toggle(M0PROFILER_GROUP group, bool state) noexcept;
	static void				Start(M0PROFILER_GROUP groups[], size_t num) noexcept;
	static void				Stop(M0PROFILER_GROUP groups[], size_t num) noexcept;

	//	print the current state to a stringstream
	static bool				OutputToStream(M0PROFILER_GROUP group, std::stringstream& stream, M0PROFILER_FLAGS flags = M0PROFILER_FLAGS{ });
	//	get current state
	static M0PROFILER_MAP&	GetDataMap(M0PROFILER_GROUP group) noexcept;
	//	reset current state
	static void				Reset(M0PROFILER_GROUP group) noexcept;

	static void				RenderToImGui(bool*);

	static void				OutputToStream(M0PROFILER_GROUP group, const char* output_name, M0PROFILER_FLAGS flags = M0PROFILER_FLAGS{ });

public:
	M0Profiler(const M0Profiler&) = delete;	M0Profiler& operator=(const M0Profiler&) = delete;
	M0Profiler(M0Profiler&&) = default;		M0Profiler& operator=(M0Profiler&&) = delete;

private:
	M0PROFILER_GROUP		GroupId;
	M0PROFILER_TAG_NAME		TagName;
	M0PROFILER_TIME_POINT	BeginTime;
};


#define PROFILE_SECTION(NAME, GROUP, ID)	M0Profiler prof_##ID(NAME, GROUP)

#define PROFILE_USECTION(NAME, GROUP)		PROFILE_SECTION(NAME, GROUP, sec)

#define PROFILE_FUNCTION(GROUP, ID)			PROFILE_SECTION(__FUNCTION__, GROUP, function_##ID)

#define PROFILE_UFUNCTION(GROUP)			PROFILE_SECTION(__FUNCTION__, GROUP, function)

#define PROFILE_FUNCSIG(GROUP, ID)			PROFILE_SECTION(__FUNCSIG__, GROUP, funcsig_##ID)

#define PROFILE_UFUNCSIG(GROUP)				PROFILE_SECTION(__FUNCSIG__, GROUP, funcsig)