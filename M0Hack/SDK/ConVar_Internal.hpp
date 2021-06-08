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
	None,
	Unregistered = 1, // If this is set, don't add to linked list, etc.
	DevOnly = 1 << 1, // Hidden in released products. Flag is removed automatically if ALLOW_DEVELOPMENT_CVARS is defined.
	GameDLL = 1 << 2, // defined by the game DLL
	ClientDLL = 1 << 3, // defined by the client DLL
	Hidden = 1 << 4, // Hidden. Doesn't appear in find or autocomplete. Like DEVELOPMENTONLY, but can't be compiled out.

	Protected = 1 << 5, // It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
	SPOnly = 1 << 6, // This cvar cannot be changed by clients connected to a multiplayer server.
	Archive = 1 << 7, // set to cause it to be saved to vars.rc
	Notify = 1 << 8, // notifies players when changed
	UserInfo = 1 << 9, // changes the client's info string
	Cheat = 1 << 14, // Only useable in singleplayer / debug / multiplayer & sv_cheats

	PrintableOnly = 1 << 10, // This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
	Unlogged = 1 << 11, // This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
	NotString = 1 << 12, // never try to print that cvar

	Replicated = 1 << 13, // server setting enforced on clients, TODO rename to FCAR_SERVER at some time
	Demo = 1 << 16, // record this cvar when starting a demo file
	DontRecord = 1 << 17, // don't record these command in demofiles
	ReloadMaterials = 1 << 20, // If this cvar changes, it forces a material reload
	ReloadTextures = 1 << 21, // If this cvar changes, if forces a texture reload

	NotConnected = 1 << 22, // cvar cannot be changed by a client that is connected to a server
	MaterialSystemThread = 1 << 23, // Indicates this cvar is read from the material system thread
	ArchiveXBox = 1 << 24, // cvar written to config.cfg on the Xbox

	AccessibleFromThreads = 1 << 25, // used as a debugging tool necessary to check material system thread convars

	ServerCanExecute = 1 << 28, // the server is allowed to execute this command on clients via ClientCommand/NET_StringCmd/CBaseClientState::ProcessStringCmd.
	ServerCantQuery = 1 << 29, // If this is set, then the server is not allowed to query this cvar's value (via IServerPluginHelpers::StartQueryCvarValue).
	ClientCmdCanExecute = 1 << 30, // IVEngineClient::ClientCmd is allowed to execute this command. Note: IVEngineClient::ClientCmd_Unrestricted can run any client command.

	ExecuteDespiteDefault = 1 << 31, // -default causes a lot of commands to be ignored (but still be recorded as though they had run). This causes them to be executed anyways.

	InternalUse = 1 << 15, // This var isn't archived, but is exposed to players--and its use is allowed in competitive play.
	AllowedInComp = 1 << 18, // This convar can be changed in competitive (strict) settings mode even though it is not archived. Meant for one-offs like cl_showfps that are not user facing settings but are benign

	MaterialThreadMask = ReloadMaterials | ReloadTextures | MaterialSystemThread
};
ECLASS_BITWISE_OPERATORS(ConVarFlag);



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
	ConCommandBase(const char* pName, const char* pHelpString = 0, ConVarFlag flags = ConVarFlag::None);

	virtual ~ConCommandBase() = default;

	virtual	bool IsCommand() const noexcept { return true; }

	virtual bool IsFlagSet(ConVarFlag flag) const noexcept { return (Flags & flag) == flag; }
	virtual void AddFlags(ConVarFlag flags) noexcept { Flags |= flags; }

	virtual const char* GetName() const noexcept { return Name; }

	virtual const char* GetHelpText() const noexcept { return HelpString; }

	const ConCommandBase* GetNext() const noexcept { return NextBase; }
	ConCommandBase* GetNext() noexcept { return NextBase; }

	virtual bool IsRegistered() const noexcept { return Is_Registered; }

	virtual VCVarDLLIdentifier GetDLLIdentifier() const;

	static void Register(ConVarFlag nCVarFlag = ConVarFlag::None, IConCommandBaseAccessor* pAccessor = nullptr);
	static void Unregister();

	ConCommandBase(const ConCommandBase&) = default;
	ConCommandBase& operator=(const ConCommandBase&) = default;
	ConCommandBase(ConCommandBase&&) = default;
	ConCommandBase& operator=(ConCommandBase&&) = default;
protected:
	virtual void CreateBase(const char* pName, const char* pHelpString = 0, ConVarFlag flags = ConVarFlag::None);

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
	ConVarFlag Flags{ };

protected:
	// ConVars add themselves to this list for the executable. 
	// Then ConVar_Register runs through  all the console variables 
	// and registers them into a global list stored in vstdlib.dll
	static inline ConCommandBase* ConCommandBases;

	// ConVars in this executable use this 'global' to access values.
	static inline IConCommandBaseAccessor* Accessor;
};

