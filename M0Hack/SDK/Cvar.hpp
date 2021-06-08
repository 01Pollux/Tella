#pragma once

#include "AppSystem.hpp"
#include "ConVar_Internal.hpp"
#include "Helper/Color.hpp"

class ConCommandBase;
class ConVar;
class ConCommand;
class IConsoleDisplayFunc;
class ICvarQuery;


class ICVar : public IAppSystem
{
public:
	virtual VCVarDLLIdentifier AllocateDLLIdentifier() = 0;

	virtual void RegisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void UnregisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void UnregisterConCommands(VCVarDLLIdentifier id) = 0;

	// If there is a +<varname> <value> on the command line, this returns the value.
	// Otherwise, it returns NULL.
	virtual const char* GetCommandLineValue(const char* pVariableName) = 0;

	virtual ConCommandBase* FindCommandBase(const char* name) = 0;
	virtual const ConCommandBase* FindCommandBase(const char* name) const = 0;
	virtual ConVar* FindVar(const char* var_name) = 0;
	virtual const ConVar* FindVar(const char* var_name) const = 0;
	virtual ConCommand* FindCommand(const char* name) = 0;
	virtual const ConCommand* FindCommand(const char* name) const = 0;

	// Get first ConCommandBase to allow iteration
	virtual ConCommandBase* GetCommands() = 0;
	virtual const ConCommandBase* GetCommands() const = 0;

	// Install a global change callback (to be called when any convar changes) 
	virtual void			InstallGlobalChangeCallback(ConVarChangeCallback callback) = 0;
	virtual void			RemoveGlobalChangeCallback(ConVarChangeCallback callback) = 0;
	virtual void			CallGlobalChangeCallbacks(ConVar* var, const char* pOldString, float flOldValue) = 0;

	// Install a console printer
	virtual void			InstallConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
	virtual void			RemoveConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
	virtual void			ConsoleColorPrintf(const color::u8rgba& clr, const char* pFormat, ...) const = 0;
	virtual void			ConsolePrintf(const char* pFormat, ...) const = 0;
	virtual void			ConsoleDPrintf(const char* pFormat, ...) const = 0;

	// Reverts cvars which contain a specific flag
	virtual void			RevertFlaggedConVars(ConVarFlag nFlag) = 0;

	// Method allowing the engine ICvarQuery interface to take over
	// A little hacky, owing to the fact the engine is loaded
	// well after ICVar, so we can't use the standard connect pattern
	virtual void			InstallCVarQuery(ICvarQuery* pQuery) = 0;

	virtual bool			IsMaterialThreadSetAllowed() const = 0;
	virtual void			QueueMaterialThreadSetValue(ConVar* pConVar, const char* pValue) = 0;
	virtual void			QueueMaterialThreadSetValue(ConVar* pConVar, int nValue) = 0;
	virtual void			QueueMaterialThreadSetValue(ConVar* pConVar, float flValue) = 0;
	virtual bool			HasQueuedMaterialThreadConVarSets() const = 0;
	virtual int				ProcessQueuedMaterialThreadConVarSets() = 0;

protected:	class ICVarIteratorInternal;
public:
	/// Iteration over all cvars. 
	/// (THIS IS A SLOW OPERATION AND YOU SHOULD AVOID IT.)
	/// usage: 
	/// { ICvar::Iterator iter(g_pCVar); 
	///   for ( iter.SetFirst() ; iter.IsValid() ; iter.Next() )
	///   {  
	///       ConCommandBase *cmd = iter.Get();
	///   } 
	/// }
	/// The Iterator class actually wraps the internal factory methods
	/// so you don't need to worry about new/delete -- scope takes care
	//  of it.
	/// We need an iterator like this because we can't simply return a 
	/// pointer to the internal data type that contains the cvars -- 
	/// it's a custom, protected class with unusual semantics and is
	/// prone to change.
	class Iterator
	{
	public:
		inline Iterator(ICVar* icvar);
		inline ~Iterator();
		inline void		SetFirst();
		inline void		Next();
		inline bool		IsValid();
		inline ConCommandBase* Get();
	private:
		ICVarIteratorInternal* Iter;
	};

protected:
	// internals for  ICVarIterator
	class ICVarIteratorInternal
	{
	public:
		// warning: delete called on 'ICvar::ICVarIteratorInternal' that is abstract but has non-virtual destructor [-Wdelete-non-virtual-dtor]
		virtual ~ICVarIteratorInternal() = default;
		virtual void		SetFirst() = 0;
		virtual void		Next() = 0;
		virtual	bool		IsValid() = 0;
		virtual ConCommandBase* Get() = 0;

	public:
		ICVarIteratorInternal(const ICVarIteratorInternal&) = default;
		ICVarIteratorInternal& operator=(const ICVarIteratorInternal&) = default;
		ICVarIteratorInternal(ICVarIteratorInternal&&) = default;
		ICVarIteratorInternal& operator=(ICVarIteratorInternal&&) = default;
	};

	virtual ICVarIteratorInternal* FactoryInternalIterator() = 0;
	friend class Iterator;
};

inline ICVar::Iterator::Iterator(ICVar* icvar)
{
	Iter = icvar->FactoryInternalIterator();
}

inline ICVar::Iterator::~Iterator()
{
	delete Iter;
}

inline void ICVar::Iterator::SetFirst()
{
	Iter->SetFirst();
}

inline void ICVar::Iterator::Next()
{
	Iter->Next();
}

inline bool ICVar::Iterator::IsValid()
{
	return Iter->IsValid();
}

inline ConCommandBase* ICVar::Iterator::Get()
{
	return Iter->Get();
}

