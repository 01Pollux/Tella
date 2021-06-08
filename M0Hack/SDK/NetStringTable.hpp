#pragma once

#include "Interfaces.hpp"

class INetworkStringTableContainer;
class INetworkStringTable;


M0_INTERFACE;

extern INetworkStringTableContainer* NSTContainer;
constexpr const char* NSTContainerName = "VEngineClientStringTable001";

M0_END;


#define INVALID_NST_INDEX -1
#define MAX_NST_TABLES	32  // Table id is 4 bits

#define NST_CLIENT_SIDE false
#define NST_SERVER_SIDE true

using NSTTableID = int;
using NSTChangedCallback = void(*)(void* object, INetworkStringTable*, NSTTableID stringnum, const char* newstring, const void* newdata);

class INetworkStringTable
{
public:

	virtual					~INetworkStringTable() = default;

	// Table Info
	virtual const char*		GetTableName() const abstract;
	virtual NSTTableID		GetTableId() const abstract;
	virtual int				GetNumStrings() const abstract;
	virtual int				GetMaxStrings() const abstract;
	virtual int				GetEntryBits() const abstract;

	// Networking
	virtual void			SetTick(int tick) abstract;
	virtual bool			ChangedSinceTick(int tick) const abstract;

	// Accessors (length -1 means don't change user data if string already exits)
	virtual int				AddString(bool NST_SIDE, const char* value, int length = -1, const void* userdata = 0) abstract;

	virtual const char*		GetString(int stringNumber) abstract;
	virtual void			SetStringUserData(int stringNumber, int length, const void* userdata) abstract;
	virtual const void*		GetStringUserData(int stringNumber, int* length) abstract;
	virtual int				FindStringIndex(char const* string) abstract; // returns INVALID_NST_INDEX if not found

	// Callbacks
	virtual void			SetStringChangedCallback(void* object, NSTChangedCallback changeFunc) abstract;
};

class INetworkStringTableContainer
{
public:

	virtual					~INetworkStringTableContainer() = default;

	// table creation/destruction
	virtual INetworkStringTable* CreateStringTable(const char* tableName, int maxentries, int userdatafixedsize = 0, int userdatanetworkbits = 0) abstract;
	virtual void				RemoveAllTables() abstract;

	// table infos
	virtual INetworkStringTable* FindTable(const char* tableName) const abstract;
	virtual INetworkStringTable* GetTable(NSTTableID stringTable) const abstract;
	virtual int					GetNumTables() const abstract;

	virtual INetworkStringTable* CreateStringTableEx(const char* tableName, int maxentries, int userdatafixedsize = 0, int userdatanetworkbits = 0, bool bIsFilenames = false) abstract;
	virtual void				SetAllowClientSideAddString(INetworkStringTable* table, bool bAllowClientSideAddString) abstract;
};