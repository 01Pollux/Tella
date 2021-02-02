#pragma once

#include "AutoList.h"
#include <unordered_map>
#include "../Helpers/SigTools.h"
#include <functional>


using CreateInterfaceFn = void* (*)(const char*, int*);

class ILibraryManager
{
public:
	enum Types
	{
		Invalid = -1,
		Client,
		Engine,
		Server,
		SteamClient,
		Launcher,
		SteamAPI,
		VSTD,
		Tier0,
		InputSys,
		MatSys,
		FileSys,
//		GameUI,
		VGUI2,
		VGUIMatSys,
		StudioRender,
		D3D,
		Count
	};

	ILibraryManager() = delete;
	ILibraryManager(const ILibraryManager&) = delete;
	explicit ILibraryManager(const char* name, Types type);
	~ILibraryManager()
	{
		if (this->ptr)
		{
			FreeLibrary(reinterpret_cast<HMODULE>(this->ptr));
			this->ptr = nullptr;

			if (m_KeySymbols)
			{
				delete m_KeySymbols;
				m_KeySymbols = nullptr;
			}
		}
	}
	const char* GetName() { return this->name; };

	uintptr_t FindPattern(const char* sym);

	void* GetLibrary() { return this->ptr; }
	void SetPointer(void* base) { this->ptr = base; }

	template<typename C>
	C* FindInterface(const char* name)
	{
		void* pModule = SigTools::ResolveSymbol(ptr, "CreateInterface");
		CreateInterfaceFn factory = reinterpret_cast<CreateInterfaceFn>(pModule);
		if (!factory)
			return nullptr;

		return reinterpret_cast<C*>(factory(name, nullptr));
	}

	void LoadSymbols()
	{
		m_KeySymbols = new std::unordered_map<std::string, uintptr_t>;
	}

private:
	std::unordered_map<std::string, uintptr_t>* m_KeySymbols{ };

	const char* name = "";
	Types lib_type = Types::Invalid;
	void* ptr = nullptr;

};

namespace Library
{
	extern ILibraryManager clientlib;
	extern ILibraryManager enginelib;
	extern ILibraryManager serverlib;
	extern ILibraryManager steamclientlib;
	extern ILibraryManager launcherlib;
	extern ILibraryManager steamapilib;
	extern ILibraryManager libvstd_lib;
	extern ILibraryManager tier0lib;
	extern ILibraryManager inputsyslib;
	extern ILibraryManager materialsyslib;
	extern ILibraryManager filesyslib;
//	extern ILibraryManager gameuilib;
	extern ILibraryManager vgui2lib;
	extern ILibraryManager vguimatsurfacelib;
	extern ILibraryManager studiorenderlib;
	extern ILibraryManager d3dlib;

	void LoadLibraries();
	void UnLoadLibraries();
}
