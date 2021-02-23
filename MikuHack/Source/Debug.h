#pragma once

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

enum class LogType
{
	Generic,
	Debug,
	Critical,
	Custom
};

namespace MIKUDebug
{
	extern bool& m_bDebugging;
	extern bool& m_bConsolePrint;

	void Init();
	void Shutdown();

	void Log(LogType type, std::string_view txt);

	inline void LogToFile(std::string_view txt) {
		Log(LogType::Generic, txt);
	}

	inline void LogDebug(std::string_view txt) {
		Log(LogType::Debug, txt);
	}

	inline void LogCritical(std::string_view txt) {
		Log(LogType::Critical, txt);
	}

	inline void LogCustom(std::string_view txt) {
		Log(LogType::Custom, txt);
	}
};


inline void _RFormat(std::ostringstream& str)
{
}

template<typename T, typename ...A>
void _RFormat(std::ostringstream& str, T& var, const A& ...args)
{
	str << var;
	_RFormat(str, args...);
}


template<typename ...A>
std::string Format(const A& ...args)
{
	std::ostringstream stream;
	_RFormat(stream, args...);
	return stream.str();
}

//#define MDEBUG MIKUDebug
namespace MDEBUG = MIKUDebug;
namespace MIKUD = MIKUDebug;
