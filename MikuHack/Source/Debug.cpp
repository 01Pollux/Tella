#pragma once


#include "Debug.h"
#include <ctime> 
#include <icvar.h>
#include <iomanip>
#include <fstream>
#include <future>

#include "../Helpers/Commons.h"
#include "../Helpers/AutoItem.h"

using namespace std;

AutoBool _m_bDebugging("MIKUDebug.m_bDebugging");
AutoBool _m_bConPrint("MIKUDebug.m_bConsolePrint");

bool& MIKUDebug::m_bDebugging = *_m_bDebugging;
bool& MIKUDebug::m_bConsolePrint = *_m_bConPrint;

static constexpr size_t streamtypes = static_cast<std::underlying_type_t<LogType>>(LogType::Custom) + 1;
static ofstream fileStreams[streamtypes];

void MIKUDebug::Init()
{
	constexpr std::string_view files[] = {
		"Logs.txt",
		"Debug.txt",
		"Critical.txt",
		"Custom.txt"
	};
	
	for (size_t i = 0; i < SizeOfArray(files); i++)
		fileStreams[i].open(".\\Miku\\Log\\" + std::string(files[i]), ios::out | ios::app);
}

void MIKUDebug::Shutdown()
{
	for (auto& streams : fileStreams)
	{
		if (streams.is_open())
			streams.close();
	}
}


void MIKUDebug::Log(LogType type, string fmt)
{
	auto _logfn = 
		[&]() {
		auto& stream = fileStreams[static_cast<std::underlying_type_t<LogType>>(type)];

		time_t time = chrono::system_clock::to_time_t(chrono::system_clock::now());
		stream << "[ " << put_time(gmtime(&time), "%c") << " ]:  " << std::move(fmt) << '\n';
	};
	std::async(std::move(_logfn));
}