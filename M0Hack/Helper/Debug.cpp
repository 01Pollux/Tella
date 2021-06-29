#include "Debug.hpp"
#include <fstream>
#include <iomanip>

#include "GlobalHooks/event_listener.hpp"
#include "ConVar.hpp"

#include "Helper/Config.hpp"
#include "Helper/Color.hpp"

namespace tella
{
	namespace log
	{
		class DebugInfo
		{
		public:
			static const char* GetFileName(types lt)
			{
				constexpr const char* err[] = {
					"./Miku/Log/Logs.log",		//LOG
					"./Miku/Log/Debug.log",		//DEBUG
					"./Miku/Log/Critical.log",	//ERROR
				};

				return err[static_cast<std::underlying_type_t<types>>(lt)];
			}

			static void PrintFirstTime(std::ofstream& file)
			{
				file << "||----------------------------------------------------------||\n"sv
					"||----------------------------------------------------------||\n"sv
					"||\t\tLogging Session Begin:\t\t\t    ||\n"sv
					"||----------------------------------------------------------||\n"sv
					"||----------------------------------------------------------||\n\n\n"sv;
			}

			static void PrintLastTime(std::ofstream& file)
			{
				file << "\n\n"sv
					"||----------------------------------------------------------||\n"sv
					"||----------------------------------------------------------||\n"sv
					"||\t\tLogging Session End\t\t\t    ||\n"sv
					"||----------------------------------------------------------||\n"sv
					"||----------------------------------------------------------||\n\n"sv;
			}

			DebugInfo();

			void insert(types type, const std::string& fmt);

			bool is_active(types type) { return DebugsOn[static_cast<std::underlying_type_t<types>>(type)]; }

			bool first_time(types type) noexcept
			{
				if (bool& first_time = FirstTime[static_cast<std::underlying_type_t<types>>(type)]; !first_time)
				{
					first_time = true;
					return true;
				}
				return false;
			}

		private:
			void OnRender();

			TCONFIG Bool DebugsOn[3]{
				{ "Debug.Types.Msg", true, "Enable print messages" },
				{ "Debug.Types.Dbg", false, "Enable debug messages" },
				{ "Debug.Types.Err", true, "Enable error messages" }
			};
			TCONFIG Bool DebugTime{ "Debug.Enable Time", true, "Debug Time" };

			TCONFIG Bool EchoEnable{ "Debug.Echo.Enable", false, "Print debug messages to tf-console" };
			TCONFIG U8Color EchoColor{ "Debug.Echo.Color", color::names::fuschia, "Print color" };

			bool FirstTime[3]{ };
		};

		static DebugInfo DbgInfo;

		void _log_internal(types type, const std::string& fmt)
		{
			if (DbgInfo.is_active(type))
				DbgInfo.insert(type, fmt);
		}
	}
}

namespace tella
{
	namespace log
	{
		void DebugInfo::insert(types type, const std::string& fmt)
		{
			std::ofstream file(GetFileName(type), std::ios::out | std::ios::app);

			if (first_time(type))
				PrintFirstTime(file);

			if (DebugTime)
				file << "[ " << FormatTime("{:%c}"sv) << " ]:\t";

			file << fmt << '\n';

			if (EchoEnable)
				Interfaces::CVar->ConsoleColorPrintf(EchoColor, "%s\n", fmt.c_str());
		}

		void DebugInfo::OnRender()
		{
			if (ImGui::CollapsingHeader("Debug"))
			{
				constexpr const char* names[]{
					"Enable Message",
					"Enable Debug",
					"Enable Error"
				};
				static_assert(SizeOfArray(names) == std::extent_v<decltype(DebugInfo::DebugsOn)>);

				for (size_t i = 0; i < SizeOfArray(DebugsOn); i++)
				{
					ImGui::Checkbox(names[i], DebugsOn[i].data());
					ImGui::SameLineHelp(DebugsOn[i]);
				}

				ImGui::Checkbox("Debug Time", DebugTime.data());
				ImGui::SameLineHelp(DebugTime);

				ImGui::Checkbox("Enable Echo", EchoEnable.data());
				ImGui::SameLineHelp(EchoEnable);

				ImGui::ColorEdit4("Echo Color", EchoColor.get(), ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8);
				ImGui::SameLineHelp(EchoColor);
			}
		}

		DebugInfo::DebugInfo()
		{
			event_listener::insert(
				event_listener::names::RenderExtra,
				std::bind(&DebugInfo::OnRender, this),
				event_listener::names::Null
			);

			event_listener::insert(
				event_listener::names::UnLoadDLL,
				[this](event_listener::data*)
				{
					for (size_t i = 0; i < SizeOfArray(FirstTime); i++)
					{
						if (FirstTime[i])
						{
							std::ofstream file(GetFileName(static_cast<types>(i)), std::ios::out | std::ios::app);
							PrintLastTime(file);
						}
					}
				},
				event_listener::names::Null
			);
		}
	}
}