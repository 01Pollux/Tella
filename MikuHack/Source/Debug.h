#pragma once

#include "Main.h"
#include <chrono>
#include <mutex>
#include <sstream>

#include <dxgiformat.h>

#define CFUSCHIA Color(255, 0, 255, 255)

#define REPLY_TO_TARGET(ACTION, TEXT, ...) \
		do { \
			g_pCVar->ConsoleColorPrintf(CFUSCHIA, TEXT, __VA_ARGS__); \
			ACTION; \
		} while (false)

#define DEBUG_CHECK_STATUS(ACTION) \
		if (!MIKUDebug::m_bDebugging) \
			REPLY_TO_TARGET(ACTION, "Debug is not ON\n")

enum LogType
{
	LGENERIC,
	LDEBUG,
	LCRITICAL,
	LCUSTOM
};

namespace MIKUDebug
{
	extern bool& m_bDebugging;
	extern bool& m_bConsolePrint;

	void Init();
	void Shutdown();

	void Log(LogType type, std::string_view txt);

	inline void LogToFile(std::string_view txt) {
		Log(LGENERIC, txt);
	}

	inline void LogDebug(std::string_view txt) {
		Log(LDEBUG, txt);
	}

	inline void LogCritical(std::string_view txt) {
		Log(LCRITICAL, txt);
	}

	inline void LogCustom(std::string_view txt) {
		Log(LCUSTOM, txt);
	}
};


static std::mutex format_mutex;

inline void _RFormat(std::stringstream& str)
{

}

template<typename T, typename ...A>
void _RFormat(std::stringstream& str, T& var, const A& ...args)
{
	str << var;
	_RFormat(str, args...);
}


template<typename ...A>
[[noinline]] std::string Format(const A& ...args)
{
	std::lock_guard format_lock(format_mutex);

	std::stringstream stream;
	_RFormat(stream, args...);

	return stream.str();
}


class ScopeLog
{
	using sys_clock = std::chrono::system_clock;
	using time_point = sys_clock::time_point;

	LogType lvl;
	bool has_began = false;
	std::string name;
	time_point start_time;

public:
	ScopeLog(const ScopeLog&) = delete;
	explicit ScopeLog(const char* name, LogType lvl = LogType::LDEBUG, bool auto_start = false) : name(name), lvl(lvl)
	{
		if (auto_start)
			Start();
	}
	~ScopeLog() { Finish(); }

	void Start();
	void Finish();
	const std::chrono::milliseconds Time() const noexcept
	{
		using namespace std::chrono;
		auto end = time_point_cast<milliseconds>(system_clock::now());
		auto elapsed = end - time_point_cast<milliseconds>(this->start_time);
		return elapsed;
	}
};


/*
enum class ProfType
{
	Never,
	Once,
	Repeat,
	Manual,
	Always,
};

class ILinkedProf
{
	static inline std::list<ILinkedProf*> ProfStorage;
public:

	virtual ~ILinkedProf() { ProfStorage.remove(this); };
	virtual const char* QueryProfName() const abstract;
	virtual ProfType QueryProfType() const abstract;
	virtual bool ShouldRecord() const abstract;
};

*/


#define PROF_FUNCTION(DBG) \
		ScopeLog scope_log_func(__FUNCTION__, DBG, true)

//#define MDEBUG MIKUDebug
namespace MDEBUG = MIKUDebug;
namespace MIKUD = MIKUDebug;
