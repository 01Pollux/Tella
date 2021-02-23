
#include "Library.h"

#include <fstream>
#include <mutex>

#include "json.h"
#include "../Source/Debug.h"

#define DLL_STR ".dll"

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
	ILibraryManager vguimatsurfacelib	= ILibraryManager("vguimatsurface",		ILibraryManager::Types::VGUIMatSys);
	ILibraryManager studiorenderlib		= ILibraryManager("studiorender",		ILibraryManager::Types::StudioRender);
	ILibraryManager d3dlib				= ILibraryManager("shaderapidx9",		ILibraryManager::Types::D3D);
}

ILibraryManager::ILibraryManager(const char* dll_name, ILibraryManager::Types type): name(dll_name), lib_type(type)
{
	std::string name = std::string(dll_name) + DLL_STR;
	this->ptr = LoadLibraryA(name.c_str());
}


namespace ISymbols
{
	struct ISigInfo
	{
		std::string value;
		int extra;
		int read;
	};

	ISigInfo QuerySigInfo(const ILibraryManager* lib, const char* sym)
	{
		Json::Value sigs;
		std::ifstream config(".\\Miku\\Signatures.json", std::ifstream::binary);
		config >> sigs;

		const char* name = lib->GetName();
		Json::Value& data = sigs[name][sym];

		int extra{ };
		int read{ };

		if (data.isMember("offset"))
			extra = data["offset"].asInt();

		if (data.isMember("read"))
			read = data["read"].asInt();

		return { data["value"].asString(), extra, read };
	}

	inline void LoadSymbols()
	{
		Json::Value libraries;
		std::ifstream configs(".\\Miku\\Signatures.json", std::ifstream::binary);
		configs >> libraries;
		configs.close();

		using namespace Library;
		constexpr ILibraryManager* libs[] = {
			&clientlib,			&enginelib,			&serverlib,
			&steamclientlib,	&launcherlib,		&steamapilib,
			&libvstd_lib,		&tier0lib,			&inputsyslib,
			&materialsyslib,	&vguimatsurfacelib,	&studiorenderlib,
			&d3dlib,
		};

		for (size_t i = 0; i < ILibraryManager::Types::Count; i++)
		{
			Json::Value& lib = libraries[libs[i]->GetName()];
			if (!lib.empty())
				libs[i]->LoadSymbols();
		}
	}
}


void Library::LoadLibraries()
{
	ISymbols::LoadSymbols();
}


uintptr_t ILibraryManager::FindPattern(const char* sym)
{
	static std::mutex pattern_lock;
	std::lock_guard<std::mutex> guard{ pattern_lock };

	auto iter = m_KeySymbols->find(sym);

	if (iter == m_KeySymbols->end())
	{
		auto res = ISymbols::QuerySigInfo(this, sym);

		if (res.value.empty())
		{
			MIKUDebug::LogCritical(Format("[Library: ", name, "] Empty Signature for: \"", sym ? sym : "<NULL>", "\"."));
			return NULL;
		}

		uintptr_t ptr = SigTools::FindPatternEx(reinterpret_cast<uintptr_t>(this->GetLibrary()), res.value.c_str());
		if (ptr)
		{
			ptr += res.extra;
			for (int i = 0; i < res.read; i++)
				ptr = *reinterpret_cast<uintptr_t*>(ptr);
		}

		m_KeySymbols->insert(std::make_pair(sym, ptr));

		if (!ptr)
			MIKUDebug::LogCritical(Format("[Library: ", name, "] Address of Signature: \"", sym, "\": \"", res.value.c_str(), "\" is NULL."));

		return ptr;
	}
	return iter->second;
}
