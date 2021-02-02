
#include "Library.h"
#include <Windows.h>
#include <optional>

#include <fstream>
#include <mutex>
#include <deque>

#include "json.h"

#if defined _WINDOWS
#define DLL_STR ".dll"
#elif defined _LINUX
#define DLL_STR ".so"
#elif defined _OSX
#define DLL_STR ".dylib"
#endif

#include "../Source/Debug.h"

namespace Library
{
	ILibraryManager clientlib			= ILibraryManager("client",				ILibraryManager::Types::Client);
	ILibraryManager enginelib			= ILibraryManager("engine",				ILibraryManager::Types::Engine);
	ILibraryManager serverlib			= ILibraryManager("server",				ILibraryManager::Types::Server);
	ILibraryManager steamclientlib		= ILibraryManager("steamclient",		ILibraryManager::Types::SteamClient);
	ILibraryManager launcherlib			= ILibraryManager("launcher",			ILibraryManager::Types::Launcher);
	ILibraryManager steamapilib			= ILibraryManager("libsteam_api",		ILibraryManager::Types::SteamAPI);
	ILibraryManager libvstd_lib			= ILibraryManager("vstdlib",			ILibraryManager::Types::VSTD);
	ILibraryManager tier0lib			= ILibraryManager("libtier0",			ILibraryManager::Types::Tier0);
	ILibraryManager inputsyslib			= ILibraryManager("inputsystem",		ILibraryManager::Types::InputSys);
	ILibraryManager materialsyslib		= ILibraryManager("materialsystem",		ILibraryManager::Types::MatSys);
	ILibraryManager filesyslib			= ILibraryManager("filesystem_stdio",	ILibraryManager::Types::FileSys);
//	ILibraryManager gameuilib			= ILibraryManager("gameui",				ILibraryManager::Types::GameUI);
	ILibraryManager vgui2lib			= ILibraryManager("vgui2",				ILibraryManager::Types::VGUI2);
	ILibraryManager vguimatsurfacelib	= ILibraryManager("vguimatsurface",		ILibraryManager::Types::VGUIMatSys);
	ILibraryManager studiorenderlib		= ILibraryManager("studiorender",		ILibraryManager::Types::StudioRender);
	ILibraryManager d3dlib				= ILibraryManager("shaderapidx9",		ILibraryManager::Types::D3D);
}

ILibraryManager::ILibraryManager(const char* dll_name, ILibraryManager::Types type): name(dll_name), lib_type(type)
{
	std::string name = std::string(dll_name) + DLL_STR;
	this->ptr = LoadLibraryA(name.c_str());
}

static inline void LoadAllLibraries(bool* state)
{
	using namespace Library;
	ILibraryManager* libs[] = {
		&clientlib,			&enginelib,			&serverlib,
		&steamclientlib,	&launcherlib,		&steamapilib,
		&libvstd_lib,		&tier0lib,			&inputsyslib,
		&materialsyslib,	&filesyslib,		//&gameuilib,
		&vgui2lib,			&vguimatsurfacelib,	&studiorenderlib,
		&d3dlib,
	};

	for (size_t i = 0; i < ILibraryManager::Types::Count; i++)
	if (state[i])
		libs[i]->LoadSymbols();
}

namespace ISymbols
{
	class SigResults
	{
	public:
		struct SigInfo {
			enum Types {
				NONE,
				OFFSET,
				READ
			};

			std::string name;
			std::string value;
			Types type;
			int extra;
			int read;
		};
		std::deque<SigInfo> m_Symbols{};

		[[noinline]] std::optional<SigInfo> GetPattern(std::string pat)
		{
			auto pos = std::find_if(m_Symbols.begin(), m_Symbols.end(), [&pat](const SigInfo& o) { return o.name == (pat); });
			if (pos == m_Symbols.end())
				return {};

			return *pos;
		}

		void Insert(std::string _name, std::string pat, SigInfo::Types types = SigInfo::Types::NONE, int offset = 0, int _read = 0)
		{
			m_Symbols.push_back(SigInfo{ .name = _name, .value = pat, .type = types, .extra = offset, .read = _read });
		}
	};
	SigResults* m_Libraries[ILibraryManager::Types::Count]{ };

	void FillIn()
	{
		Json::Value libraries;
		std::ifstream configDoc(".\\Miku\\Signatures.json", std::ifstream::binary);
		configDoc >> libraries;

		const std::string lib_names[] = {
			"client",		"engine",			"server", 
			"steamclient",	"launcher",			"steamapi",
			"valve_std",	"tier0",			"inputsys",
			"mat_sys",		"file_sys",			"vgui2", 
			"vgui2_matsys", "studio_render",	"d3d"
		};

		bool states[ILibraryManager::Types::Count]{};

		for (size_t i = 0; i < ILibraryManager::Types::Count; i++)
		{
			Json::Value& lib = libraries[lib_names[i]];
			if (lib.empty())
				continue;

			m_Libraries[i] = new SigResults;
			auto map = m_Libraries[i];

			for (auto names = lib.getMemberNames(); std::string& name : names)
			{
				if (name.size())
				{
					Json::Value& infos = lib[name];

					SigResults::SigInfo::Types types{ };
					int extra{ };
					int read{ };

					std::string value = infos["value"].asString();

					if (infos.isMember("offset"))
					{
						types = SigResults::SigInfo::Types::OFFSET;
						extra = infos["offset"].asInt();
					}
					if (infos.isMember("read"))
					{
						types = SigResults::SigInfo::Types::READ;
						read = infos["read"].asInt();
					}

					map->Insert(name, value, types, extra, read);

					states[i] = true;
				}
			}
		}

		LoadAllLibraries(states);
	}
}

void Library::LoadLibraries()
{
	ISymbols::FillIn();
}

void Library::UnLoadLibraries()
{
	for (auto& sig : ISymbols::m_Libraries)
	if (sig)
		delete sig;
}

uintptr_t ILibraryManager::FindPattern(const char* sym)
{
	static std::mutex _m;
	std::lock_guard<std::mutex> lock(_m);

	if (!m_KeySymbols)
	{
		MIKUDebug::LogCritical(Format("[Library: ", name, "] Expexted \"", sym, "\", but m_KeySymbols was not initialized!"));
		return NULL;
	}

	auto iter = m_KeySymbols->find(sym);

	if (iter == m_KeySymbols->end())
	{
		const auto sig = ISymbols::m_Libraries[this->lib_type];
		auto res = sig->GetPattern(sym);

		if (!res.has_value())
		{
			MIKUDebug::LogCritical(Format("[Library: ", name, "] Empty Signature for: \"", sym ? sym : "<NULL>", "\"."));
			return NULL;
		}

		uintptr_t ptr = SigTools::FindPatternEx(reinterpret_cast<uintptr_t>(this->GetLibrary()), res->value.c_str());
		if (ptr)
		{
			ptr += res->extra;
			for (int i = 0; i < res->read; i++)
				ptr = *reinterpret_cast<uintptr_t*>(ptr);
		}

		m_KeySymbols->insert(std::make_pair(sym, ptr));

		if (!ptr)
			MIKUDebug::LogCritical(Format("[Library: ", name, "] Address of Signature: \"", sym, "\": \"", res->value.c_str(), "\" is NULL."));

		return ptr;
	}
	return iter->second;
}
