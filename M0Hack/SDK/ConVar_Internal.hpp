#pragma once

#include "Helper/EnumClass.hpp"
#include <string>

class ConVar_Internal;
class ConVar;
class ConCommandBase;

using ConVarChangeCallback = void(*)(ConVar* var, const char* pOldValue, float flOldValue);
using VCVarDLLIdentifier = int;

enum class ConVarFlag
{
	Unregistered, // If this is set, don't add to linked list, etc.
	DevOnly, // Hidden in released products. Flag is removed automatically if ALLOW_DEVELOPMENT_CVARS is defined.
	GameDLL, // defined by the game DLL
	ClientDLL, // defined by the client DLL
	Hidden, // Hidden. Doesn't appear in find or autocomplete. Like DEVELOPMENTONLY, but can't be compiled out.

	Protected, // It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
	SPOnly, // This cvar cannot be changed by clients connected to a multiplayer server.
	Archive, // set to cause it to be saved to vars.rc
	Notify, // notifies players when changed
	UserInfo, // changes the client's info string
	Cheat = 14, // Only useable in singleplayer / debug / multiplayer & sv_cheats

	PrintableOnly = 10,		// This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
	Unlogged,				// This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
	NotString,				// never try to print that cvar

	Replicated,				// server setting enforced on clients, TODO rename to FCAR_SERVER at some time
	Demo = 16, // record this cvar when starting a demo file
	DontRecord, // don't record these command in demofiles
	ReloadMaterials = 20, // If this cvar changes, it forces a material reload
	ReloadTextures, // If this cvar changes, if forces a texture reload

	NotConnected, // cvar cannot be changed by a client that is connected to a server
	MaterialSystemThread, // Indicates this cvar is read from the material system thread
	ArchiveXBox, // cvar written to config.cfg on the Xbox

	AccessibleFromThreads, // used as a debugging tool necessary to check material system thread convars

	ServerCanExecute = 28, // the server is allowed to execute this command on clients via ClientCommand/NET_StringCmd/CBaseClientState::ProcessStringCmd.
	ServerCantQuery, // If this is set, then the server is not allowed to query this cvar's value (via IServerPluginHelpers::StartQueryCvarValue).
	ClientCmdCanExecute, // IVEngineClient::ClientCmd is allowed to execute this command. Note: IVEngineClient::ClientCmd_Unrestricted can run any client command.

	ExecuteDespiteDefault, // -default causes a lot of commands to be ignored (but still be recorded as though they had run). This causes them to be executed anyways.

	InternalUse = 15, // This var isn't archived, but is exposed to players--and its use is allowed in competitive play.
	AllowedInComp = 18, // This convar can be changed in competitive (strict) settings mode even though it is not archived. Meant for one-offs like cl_showfps that are not user facing settings but are benign

	_Highest_Enum = 32,

	Mask_MaterialThread = 1 << ReloadMaterials | 1 << ReloadTextures | 1 << MaterialSystemThread,
};
using MConVarFlag = bitmask::mask<ConVarFlag>;


class IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(ConCommandBase* pVar) abstract;
};

class ConVar_Internal
{
public:
	virtual void SetValue(const char* pValue) abstract;
	virtual void SetValue(float flValue) abstract;
	virtual void SetValue(int nValue) abstract;

	virtual const char* GetName() const abstract;

	virtual bool IsFlagSet(int nFlag) const abstract;
};

class ConCommandBase
{
	friend class CCvar;
	friend class ConVar;
	friend class ConCommand;
public:
	ConCommandBase() = default;
	ConCommandBase(const char* pName, const char* pHelpString = 0, MConVarFlag flags = MConVarFlag{});

	virtual ~ConCommandBase() = default;

	virtual	bool IsCommand() const noexcept { return true; }

	virtual bool IsFlagSet(uint32_t flag) const noexcept { return (Flags & flag) == flag; }
	virtual void AddFlags(uint32_t flags) noexcept { Flags |= flags; }

	virtual const char* GetName() const noexcept { return Name; }

	virtual const char* GetHelpText() const noexcept { return HelpString; }

	const ConCommandBase* GetNext() const noexcept { return NextBase; }
	ConCommandBase* GetNext() noexcept { return NextBase; }

	virtual bool IsRegistered() const noexcept { return Is_Registered; }

	virtual VCVarDLLIdentifier GetDLLIdentifier() const;

	static void Register(uint32_t nCVarFlag = 0, IConCommandBaseAccessor* pAccessor = nullptr);
	static void Unregister();

	ConCommandBase(const ConCommandBase&) = default;
	ConCommandBase& operator=(const ConCommandBase&) = default;
	ConCommandBase(ConCommandBase&&) = default;
	ConCommandBase& operator=(ConCommandBase&&) = default;
protected:
	virtual void CreateBase(const char* pName, const char* pHelpString = 0, uint32_t flags = 0);

	// Used internally by OneTimeInit to initialize/shutdown
	virtual void Init()
	{
		if (Accessor)
			Accessor->RegisterConCommandBase(this);
	}
	void Shutdown();

	// Internal copy routine ( uses new operator from correct module )
	std::string CopyString(const char* from);

public:
	// Next ConVar in chain
	// Prior to register, it points to the next convar in the DLL.
	// Once registered, though, m_pNext is reset to point to the next
	// convar in the global list
	ConCommandBase* NextBase{ };

	// Has the cvar been added to the global list?
	bool Is_Registered{ };

	// Static data
	const char* Name{ };
	const char* HelpString{ };

	// ConVar flags
	uint32_t Flags;

protected:
	// ConVars add themselves to this list for the executable. 
	// Then ConVar_Register runs through  all the console variables 
	// and registers them into a global list stored in vstdlib.dll
	static inline ConCommandBase* ConCommandBases;

	// ConVars in this executable use this 'global' to access values.
	static inline IConCommandBaseAccessor* Accessor;
};

