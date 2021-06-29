#pragma once

#include "Format.hpp"

namespace tella
{
	namespace log
	{
		enum class types : char8_t
		{
			Generic,
			Debug,
			Critical,
		};

		void _log_internal(types type, const std::string& text);

		template<typename... _Args>
		void msg(const std::string_view& fmt, const _Args&... args)
		{
			tella::_log_internal(types::Generic, std::format(fmt, args...));
		}

		template<typename... _Args>
		void dbg(const std::string_view& fmt, const _Args&... args)
		{
			tella::_log_internal(types::Debug, std::format(fmt, args...));
		}

		template<typename... _Args>
		void err(const std::string_view& fmt, const _Args&... args)
		{
			tella::_log_internal(types::Critical, std::format(fmt, args...));
		}
	}
}