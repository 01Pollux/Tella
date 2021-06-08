#pragma once

class IClientEntityList;
namespace Interfaces
{
	extern IClientEntityList* EntityList;
	constexpr const char* EntityListName = "VClientEntityList003";
}

class IBaseHandle;
class IClientNetworkable;
class IClientUnknown;
class IBaseEntityInternal;

class IClientEntityList
{
public:
	// Get IClientNetworkable interface for specified entity
	virtual IClientNetworkable* GetClientNetworkable(int entnum) abstract;
	virtual IClientNetworkable* GetClientNetworkableFromHandle(IBaseHandle hEnt) abstract;
	virtual IClientUnknown*		GetClientUnknownFromHandle(IBaseHandle hEnt) abstract;

	// NOTE: This function is only a convenience wrapper.
	// It returns GetClientNetworkable( entnum )->GetIClientEntity().
	virtual IBaseEntityInternal* GetClientEntity(int entnum) abstract;
	virtual IBaseEntityInternal* GetClientEntityFromHandle(IBaseHandle hEnt) abstract;

	// Returns number of entities currently in use
	virtual int					NumberOfEntities(bool bIncludeNonNetworkable) abstract;

	// Returns highest index actually used
	virtual int					GetHighestEntityIndex() abstract;

	// Sizes entity list to specified size
	virtual void				SetMaxEntities(int maxents) abstract;
	virtual int					GetMaxEntities() abstract;
};