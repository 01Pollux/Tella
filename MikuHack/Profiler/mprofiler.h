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
	CHEAT_PROFILE,

	COUNT,
};
using M0PROFILER_GROUP_T = std::underlying_type_t<M0PROFILER_GROUP>;

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
	EMPTY = 0,
	HIDE_CHILDRENS = 1 << 0,
	CLEAR_STATE = 1 << 2
};

constexpr bool operator&(const M0PROFILER_FLAGS& a, const M0PROFILER_FLAGS& b)
{
	using true_type = std::underlying_type_t<M0PROFILER_FLAGS>;
	return static_cast<true_type>(a) & static_cast<true_type>(b);
}

constexpr M0PROFILER_FLAGS operator|(const M0PROFILER_FLAGS& a, const M0PROFILER_FLAGS& b)
{
	using true_type = std::underlying_type_t<M0PROFILER_FLAGS>;
	return static_cast<M0PROFILER_FLAGS>(static_cast<true_type>(a) | static_cast<true_type>(b));
}


class M0Profiler
{
public:
	M0Profiler(M0PROFILER_TAG_NAME tag_name, M0PROFILER_GROUP group);
	~M0Profiler();

	M0PROFILER_TAG_NAME		GetSectionName()	const noexcept { return name; }
	M0PROFILER_GROUP		GetGroup()			const noexcept { return group; }
	M0PROFILER_GROUP_NAME	GetGroupName()		const noexcept { return M0PROFILE_NAMES[static_cast<std::underlying_type_t<M0PROFILER_GROUP>>(group)]; }

public:
	//	is profiling active?
	static bool				IsActive(M0PROFILER_GROUP group) noexcept;

	//	Start/Stop profiling a session(s)
	static void				Toggle(M0PROFILER_GROUP group, bool state) noexcept;
	static void				Start(M0PROFILER_GROUP groups[], size_t num) noexcept;
	static void				Stop(M0PROFILER_GROUP groups[], size_t num) noexcept;

	//	print the current state to a stringstream
	static bool				OutputToSteam(M0PROFILER_GROUP group, std::stringstream& stream, M0PROFILER_FLAGS flags = M0PROFILER_FLAGS::EMPTY);
	//	get current state
	static M0PROFILER_MAP&	GetDataMap(M0PROFILER_GROUP group) noexcept;
	//	reset current state
	static void				Reset(M0PROFILER_GROUP group) noexcept;


public:
	M0Profiler(const M0Profiler&) = delete;
	M0Profiler& operator=(const M0Profiler&) = delete;
	M0Profiler(M0Profiler&&) = default;
	M0Profiler& operator=(M0Profiler&&) = delete;


private:
	M0PROFILER_GROUP			group;
	M0PROFILER_TAG_NAME			name;
	M0PROFILER_TIMER			timer;

	friend class M0ProfileHelper::M0ProfileManager;
};