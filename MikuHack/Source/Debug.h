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

	void Log(LogType type, std::string txt);

	inline void LogToFile(std::string txt) {
		Log(LogType::Generic, txt);
	}

	inline void LogDebug(std::string txt) {
		Log(LogType::Debug, txt);
	}

	inline void LogCritical(std::string txt) {
		Log(LogType::Critical, txt);
	}

	inline void LogCustom(std::string txt) {
		Log(LogType::Custom, txt);
	}
};

//#define MDEBUG MIKUDebug
namespace MDEBUG = MIKUDebug;
namespace MIKUD = MIKUDebug;