 
#include "Config.hpp"
#include "GlobalHook/listener.hpp"
#include "Debug.hpp"
#include "ConVar.hpp"
#include "Helper/Color.hpp"

#include <fstream>
#include <regex>
#include <tuple>


M0CONFIG_BEGIN;

class ConfigLoader
{
public:
	ConfigLoader()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[this](M0EventData*)
			{
				HasLoaded = true;

				if (!Storage)
					return;

				for (auto& data : *Storage)
				{
					const std::string& key = std::get<std::string>(data);
					if (!M0VarStorage::export_var(key, std::get<M0Config::M0VarStorage*>(data), std::get<const char*>(data)))
						M0Logger::Err("Failed to export config var: {}", key);
				}

				Storage = nullptr;
			},
			EVENT_NULL_NAME
		);
	}

	bool hasloaded() const noexcept
	{
		return HasLoaded;
	}

	void emplace(const std::string& key, M0CONFIG M0VarStorage* cfg, const char* des)
	{
		if (!Storage)
			Storage = std::make_unique<decltype(Storage)::element_type>();

		Storage->emplace_back(key, cfg, des);
	}

private:
	std::unique_ptr<
		std::vector<
			std::tuple<
				std::string, 
				M0CONFIG M0VarStorage*, 
				const char*>
		>> Storage;
	bool HasLoaded = false;
};

static ConfigLoader late_cfgloader;


M0VarStorage::~M0VarStorage() noexcept
{
	detach_var(this);
}


M0VarStorage::M0VarStorage(const std::string& key, const char* des) noexcept
{
	if (!late_cfgloader.hasloaded())
		late_cfgloader.emplace(key, this, des);
	else if (!export_var(key, this, des))
		M0Logger::Err("Failed to export config var: {}", key);
}


bool M0VarStorage::export_var(const std::string& key, M0VarStorage* inst, const char* des)
{
	if (!late_cfgloader.hasloaded())
	{
		late_cfgloader.emplace(key, inst, des);
		return false;
	}
	else
	{
		inst->Description = des;
		return VarStorage.emplace(key, inst).second;
	}
}


void M0VarStorage::detach_var(M0VarStorage* inst)
{
	auto iter = std::find_if(VarStorage.begin(), VarStorage.end(), [inst](const auto& cur) { return cur.second == inst; });
	if (iter != VarStorage.end())
		VarStorage.erase(iter);
}

	
void M0VarStorage::detach_var(const std::string& key)
{
	VarStorage.erase(key);
}


void M0VarStorage::read_var()
{
	Json::Value mainConfig;
	std::ifstream file(M0CONFIG_CHEAT_PATH);
	file >> mainConfig;

	for (auto& [key, cfg] : VarStorage)
	{
		const Json::Value* subsec = &mainConfig;
		const std::regex split("\\.+");

		for (std::sregex_token_iterator end,
			iter(key.cbegin(), key.cend(), split, -1);
			iter != end;
			iter++)
		{
			subsec = &(*subsec)[iter->str()];
		}

		if (subsec->isNull())
			continue;

		const Json::Value& val = (*subsec)[M0CONFIG_VALUE_KEY];
		cfg->_read(val);
	}
}


void M0VarStorage::write_var()
{
	Json::Value mainConfig;

	for (const auto& [key, cfg] : VarStorage)
	{
		Json::Value* subsec = &mainConfig;
		const std::regex split("\\.+");

		for (std::sregex_token_iterator end,
			iter(key.cbegin(), key.cend(), split, -1);
			iter != end;
			iter++)
		{
			subsec = &(*subsec)[iter->str()];
		}

		Json::Value& val = (*subsec)[M0CONFIG_VALUE_KEY];
		cfg->_write(val);

		if (cfg->has_description())
			(*subsec)[M0CONFIG_DESCRIPTION_KEY] = cfg->get_description();
	}

	Json::StyledWriter styledWriter;
	std::ofstream(M0CONFIG_CHEAT_PATH) << styledWriter.write(mainConfig);
}


M0VarStorage* M0VarStorage::import_var(const std::string& strs)
{
	auto iter = VarStorage.find(strs);
	return iter == VarStorage.end() ? nullptr : iter->second;
}


void M0VarStorage::print_vars()
{
	for (auto& [key, varptr] : VarStorage)
	{
		const char* des = varptr->has_description() ? varptr->get_description() : "";
		ReplyToCCmd(
			color::names::green,
			"variable: \"{}\" = \"{}\"  (\"{}\")"sv,
			key,
			des,
			static_cast<const void*>(varptr)
		);
	}
}

M0CONFIG_END;


enum class ConfigDataType
{
	Bool,
	Int8,
	Int32,
	Float,
	String,
	ArrayXY,
	Color,
};

static void Cmd_PrintConfigVar(const CCommand& args)
{
	M0Config::M0VarStorage::print_vars();
}
M01_CONCOMMAND(vars, Cmd_PrintConfigVar, "Print config variables");

static void Cmd_GetConfigVar(const CCommand& args)
{
	if (!args.has_count(2))
	{
		ReplyToCCmd(color::names::red, "usage: m01_getvar <type> <var name>");
		ReplyToCCmd(
			color::names::red, 
			"\tbool\t\t: 0\n"
			"\tint8\t\t: 1\n"
			"\tint32\t: 2\n"
			"\tfloat\t: 3\n"
			"\tstring\t: 4\n"
			"\tarrayxy\t: 5\n"
			"\tcolor\t: 6"
		);
		return;
	}
	
	M0Config::M0VarStorage* pVar = M0Config::M0VarStorage::import_var(args[2]);
	if (!pVar)
	{
		ReplyToCCmd(color::names::red, "variable: \"{}\" doesn't exists", args[2]);
		return;
	}

	std::string val;

	auto type = static_cast<ConfigDataType>(atoi(args[1]));
	switch (type)
	{
	case ConfigDataType::Bool:
	{
		val = std::format("{}", static_cast<M0Config::Bool*>(pVar)->get());;
		break;
	}
	case ConfigDataType::Int8:
	{
		val = std::format("{:d}", *reinterpret_cast<int8_t*>(static_cast<M0Config::Int*>(pVar)->data()));;
		break;
	}
	case ConfigDataType::Int32:
	{
		val = std::format("{}", static_cast<M0Config::Int*>(pVar)->get());
		break;
	}
	case ConfigDataType::Float:
	{
		val = std::format("{:6f}", static_cast<M0Config::Float*>(pVar)->get());
		break;
	}
	case ConfigDataType::String:
	{
		val = static_cast<M0Config::String*>(pVar)->get();
		break;
	}
	case ConfigDataType::ArrayXY:
	{
		auto arr = static_cast<M0Config::ArrayXY<int>*>(pVar)->get();
		val = std::format("<{:d}, {:d}>", arr[0], arr[1]);
		break;
	}
	case ConfigDataType::Color:
	{
		color::u8rgba clr = static_cast<M0Config::U8Color*>(pVar)->get();
		val = std::format("{}, {}, {}, {}", static_cast<int>(clr.r), static_cast<int>(clr.g), static_cast<int>(clr.b), static_cast<int>(clr.a));
		break;
	}
	default:
	{
		ReplyToCCmd(color::names::red, "variable: \"{1}\" with type \"{0}\" is invalid", args[1], args[2]);
		return;
	}
	}

	const char* des = pVar->has_description() ? pVar->get_description() : "no description";
	ReplyToCCmd(color::names::green, "variable: \"{}\" = \"{}\"  (\"{}\")", args[2], val, des);
}
M01_CONCOMMAND(get, Cmd_GetConfigVar, "Get config value");

static void Cmd_SetConfigVar(const CCommand& args)
{
	if (!args.has_atleast(3))
	{
		ReplyToCCmd(color::names::red, "usage: m01_setvar <type> <var name> <new value> <notify = 0>");
		ReplyToCCmd(
			color::names::red, 
			"\tbool\t\t: 0\n"
			"\tint8\t\t: 1\n"
			"\tint32\t: 2\n"
			"\tfloat\t: 3\n"
			"\tstring\t: 4\n"
			"\tarrayxy\t: 5\n"
			"\tcolor\t: 6"
		);
		return;
	}
	
	M0Config::M0VarStorage* pVar = M0Config::M0VarStorage::import_var(args[2]);
	if (!pVar)
	{
		ReplyToCCmd(color::names::red, "variable: \"{}\" doesn't exists", args[2]);
		return;
	}

	ConfigDataType type = static_cast<ConfigDataType>(atoi(args[1]));
	const char* val = args[3];
	bool has_notifier = args.has_atleast(4) ? args[4][0] != '0' : false;

	switch (type)
	{
	case ConfigDataType::Bool:
	{
		if (has_notifier)
			static_cast<M0Config::BoolN*>(pVar)->notify_and_change(atoi(val) != 0);
		else static_cast<M0Config::Bool*>(pVar)->get() = atoi(val) != 0;
		break;
	}
	case ConfigDataType::Int8:
	{
		if (has_notifier)
			static_cast<M0Config::Custom<int8_t, true>*>(pVar)->notify_and_change(static_cast<int8_t>(atoi(val)));
		else static_cast<M0Config::Custom<int8_t, false>*>(pVar)->get() = static_cast<int8_t>(atoi(val));
		break;
	}
	case ConfigDataType::Int32:
	{
		if (has_notifier)
			static_cast<M0Config::IntN*>(pVar)->notify_and_change(atoi(val));
		else static_cast<M0Config::Int*>(pVar)->get() = atoi(val);
		break;
	}
	case ConfigDataType::Float:
	{
		if (has_notifier)
			static_cast<M0Config::FloatN*>(pVar)->notify_and_change(static_cast<float>(atof(val)));
		else static_cast<M0Config::Float*>(pVar)->get() = static_cast<float>(atof(val));
		break;
	}
	case ConfigDataType::String:
	{
		if (has_notifier)
			static_cast<M0Config::StringN*>(pVar)->notify_and_change(val);
		else static_cast<M0Config::String*>(pVar)->get() = val;
		break;
	}
	case ConfigDataType::ArrayXY:
	{
		int x{ }, y{ };
		sscanf_s(val, "%i %i", &x, &y);

		if (has_notifier)
			static_cast<M0Config::ArrayXYN<int>*>(pVar)->notify_and_change({ x, y });
		else static_cast<M0Config::ArrayXY<int>*>(pVar)->get() = { x, y };
		break;
	}
	case ConfigDataType::Color:
	{
		int r, g, b, a;
		sscanf_s(val, "%i %i %i %i", &r, &g, &b, &a);

		color::u8rgba clr{ static_cast<char8_t>(r), static_cast<char8_t>(g), static_cast<char8_t>(b), static_cast<char8_t>(a) };

		if (has_notifier)
			static_cast<M0Config::U8ColorN*>(pVar)->notify_and_change(clr);
		else static_cast<M0Config::U8Color*>(pVar)->get() = clr;
		break;
	}
	default:
	{
		ReplyToCCmd(color::names::red, "variable: \"{1}\" with type \"{0}\" is invalid", args[1], args[2]);
		return;
	}
	}
}
M01_CONCOMMAND(set, Cmd_SetConfigVar, "Get config value");