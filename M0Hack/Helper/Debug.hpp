#pragma once

#include "Format.hpp"

enum class LogType : char8_t
{
	Generic,
	Debug,
	Critical,
};

namespace M0Logger
{
	void LogToFile(LogType type, const std::string& text);

	template<typename... _Args>
	void Msg(const std::string_view& fmt, const _Args&... args)
	{
		M0Logger::LogToFile(LogType::Generic, std::format(fmt, args...));
	}

	template<typename... _Args>
	void Dbg(const std::string_view& fmt, const _Args&... args)
	{
		M0Logger::LogToFile(LogType::Debug, std::format(fmt, args...));
	}
	
	template<typename... _Args>
	void Err(const std::string_view& fmt, const _Args&... args)
	{
		M0Logger::LogToFile(LogType::Critical, std::format(fmt, args...));
	}
}