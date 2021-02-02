#pragma once


#include "Debug.h"
#include <ctime> 
#include <icvar.h>
#include <iomanip>
#include <fstream>
#include <future>

#include "../Helpers/Commons.h"

using namespace std;
using namespace std::chrono;

AutoBool _m_bDebugging("MIKUDebug.m_bDebugging");
AutoBool _m_bConPrint("MIKUDebug.m_bConsolePrint");

bool& MIKUDebug::m_bDebugging = *_m_bDebugging;
bool& MIKUDebug::m_bConsolePrint = *_m_bConPrint;

static constexpr size_t streamtypes = static_cast<size_t>(LogType::LCUSTOM) + 1;
static ofstream fileStreams[streamtypes];

void MIKUDebug::Init()
{
	constexpr const char* files[] = {
		"Logs.txt",
		"Debug.txt",
		"Critical.txt",
		"Custom.txt"
	};
	
	constexpr size_t size = SizeOfArray(files);

	for (size_t i = 0; i < size; i++)
	{
		string actual = string(".\\Miku\\Log\\") + files[i];
		fileStreams[i].open(actual, ios::out | ios::app);
	}
}

void MIKUDebug::Shutdown()
{
	for (auto& streams : fileStreams)
	{
		if (streams.is_open())
			streams.close();
	}
}


void MIKUDebug::Log(LogType type, string_view fmt)
{
	auto _logfn = 
		[&]() {
		auto& stream = fileStreams[type];

		time_t time = system_clock::to_time_t(system_clock::now());
		stream << "[ " << put_time(gmtime(&time), "%c") << " ]:  " << fmt << endl;

		if (m_bConsolePrint)
		{
			Color c{ 0, 125, 255, 255 };
			g_pCVar->ConsoleColorPrintf(c, "[01Miku]: %s\n", string(fmt).c_str());
		}
	};
	std::async(std::move(_logfn));
}

void ScopeLog::Start()
{
	start_time = system_clock::now();
	has_began = true;
}

void ScopeLog::Finish()
{
	using namespace std::chrono;

	if (has_began)
	{
		has_began = false;
		auto elapsed = Time();
		MIKUDebug::Log(lvl, Format("< ", name, " > Profiling End: \"", elapsed.count(), "\" elapsed"));
	}
}

HAT_COMMAND(throw_test, "RaiseExecption")
{
	int* p = nullptr;
	*p = 0;
}