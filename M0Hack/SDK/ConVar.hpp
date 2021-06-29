#pragma once

#include "ConVar_Internal.hpp"
#include "CVar.hpp"
#include "UtlVector.hpp"

#include <memory>

namespace Interfaces
{
	extern ICVar* CVar;
	constexpr const char* CVarName = "VEngineCvar004";
}

#define CONCOMMAND_COMPLETION_MAXITEMS		64
#define CONCOMMAND_COMPLETION_ITEM_LENGTH	64

class CCommand;

using ConCommandCallbackV = void(*)();
using ConCommandCallback = void(*)(const CCommand&);
using ConCommandCompletionCallback = int(*)(const char* partial, char commands[CONCOMMAND_COMPLETION_MAXITEMS][CONCOMMAND_COMPLETION_ITEM_LENGTH]);



class ConCommand : public ConCommandBase
{
	friend class ICVar;

public:
	ConCommand(const char* pName, ConCommandCallbackV callback,
		const char* pHelpString = nullptr, MConVarFlag flags = MConVarFlag{}, ConCommandCompletionCallback completionFunc = nullptr);
	ConCommand(const char* pName, ConCommandCallback callback,
		const char* pHelpString = nullptr, MConVarFlag flags = MConVarFlag{}, ConCommandCompletionCallback completionFunc = nullptr);

	virtual ~ConCommand() = default;

	bool IsCommand() const noexcept override { return true; }

	virtual int AutoCompleteSuggest(const char* partial, ValveUtlVector<const char*>& commands);

	virtual bool CanAutoComplete() { return HasCompletionCallback; }

	virtual void Dispatch(const CCommand& command);

	ConCommand(const ConCommand&) = default;
	ConCommand& operator=(const ConCommand&) = default;
	ConCommand(ConCommand&&) = default;
	ConCommand& operator=(ConCommand&&) = default;
private:
	union
	{
		ConCommandCallbackV VoidCallback;
		ConCommandCallback Callback;
	};

	ConCommandCompletionCallback CompletionCallback;

	bool HasCompletionCallback : 1;
	bool UsingNewCommandCallback : 1;
	bool UsingCommandCallbackInterface : 1;
};


class ConVar : public ConCommandBase, public ConVar_Internal
{
public:
	ConVar(
		const char* pName, const char* pDefaultValue, MConVarFlag flags = MConVarFlag{})
	{
		Create(pName, pDefaultValue, flags);
	}

	ConVar(
		const char* pName, const char* pDefaultValue, MConVarFlag flags,
		const char* pHelpString)
	{
		Create(pName, pDefaultValue, flags, pHelpString);
	}

	ConVar(
		const char* pName, const char* pDefaultValue, MConVarFlag flags,
		const char* pHelpString, bool bMin, float fMin, bool bMax, float fMax)
	{
		Create(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax);
	}

	ConVar(
		const char* pName, const char* pDefaultValue, MConVarFlag flags,
		const char* pHelpString, ConVarChangeCallback callback)
	{
		Create(pName, pDefaultValue, flags, pHelpString, false, 0.0, false, 0.0, false, 0.0, false, 0.0, callback);
	}

	ConVar(
		const char* pName, const char* pDefaultValue, MConVarFlag flags,
		const char* pHelpString, bool bMin, float fMin, bool bMax, float fMax,
		ConVarChangeCallback callback)
	{
		Create(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax, false, 0.0, false, 0.0, callback);
	}

	ConVar(
		const char* pName, const char* pDefaultValue, MConVarFlag flags,
		const char* pHelpString, bool bMin, float fMin, bool bMax, float fMax,
		bool bCompMin, float fCompMin, bool bCompMax, float fCompMax,
		ConVarChangeCallback callback)
	{
		Create(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax, bCompMin, fCompMin, bCompMax, fCompMax, callback);
	}


	virtual ~ConVar() = default;

	bool IsFlagSet(uint32_t flags)			const noexcept final { return (Parent->Flags & flags) == flags; }
	const char* GetHelpText()						const noexcept final { return Parent->HelpString; }
	bool IsRegistered()								const noexcept final { return Parent->Is_Registered; }
	const char* GetName()							const noexcept final { return Parent->Name; }
	void AddFlags(uint32_t flags)				  noexcept final { Parent->Flags |= flags; }
	bool IsCommand()								const noexcept final { return false; }

	void InstallChangeCallback(ConVarChangeCallback callback)
	{
		Parent->ChangeCallback = callback;
		if (callback)
			Parent->ChangeCallback(this, StringValue.get(), FloatValue);
	}

	// Retrieve value
	float GetFloat() const noexcept
	{
		return Parent->FloatValue;
	}
	int	GetInt() const noexcept
	{
		return Parent->IntValue;
	}
	bool GetBool() const noexcept
	{
		return GetInt() != 0;
	}
	const char* GetString() const noexcept
	{
		return Parent->StringValue.get();
	}

	// These just call into the IConCommandBaseAccessor to check flags and set the var (which ends up calling InternalSetValue).
	void SetValue(const char* value) override
	{
		Parent->InternalSetValue(value);
	}
	void SetValue(float value) override
	{
		Parent->InternalSetFloatValue(value);
	}
	void SetValue(int value) override
	{
		Parent->InternalSetIntValue(value);
	}

	void Revert()
	{
		Parent->SetValue(DefaultValue);
	}

	// True if it has a min/max setting
	bool GetMin(float& minVal) const noexcept
	{
		minVal = Parent->MinValue;
		return Parent->HasMin;
	}
	bool GetMax(float& maxVal) const noexcept
	{
		maxVal = Parent->MaxValue;
		return Parent->HasMax;
	}
	const char* GetDefault() const noexcept
	{
		return DefaultValue;
	}
		
	void SetDefault(const char* val) noexcept
	{
		DefaultValue = val ? val : "";
	}

	// True if it has a min/max competitive setting
	bool GetCompMin(float& minVal) const noexcept
	{
		minVal = Parent->CompMinValue;
		return Parent->HasCompMin;
	}
	bool GetCompMax(float& maxVal) const noexcept
	{
		maxVal = Parent->CompMaxValue;
		return Parent->HasCompMax;
	}

	ConVar(const ConVar&) = delete;
	ConVar& operator=(const ConVar&) = delete;
	ConVar(ConVar&&) = delete;
	ConVar& operator=(ConVar&&) = delete;

private:
	// Called by CCvar when the value of a var is changing.
	virtual void InternalSetValue(const char* value);
	// For CVARs marked FCVAR_NEVER_AS_STRING
	virtual void InternalSetFloatValue(float fNewValue, bool bForce = false);
	virtual void InternalSetIntValue(int nValue);

	virtual bool ClampValue(float& value);
	virtual void ChangeStringValue(const char* tempVal, float flOldValue);

	void Create(
		const char* pName, const char* pDefaultValue, MConVarFlag flags = MConVarFlag{ },
		const char* pHelpString = 0, bool bMin = false, float fMin = 0.0,
		bool bMax = false, float fMax = 0.0,
		bool bCompMin = false, float fCompMin = 0.0,
		bool bCompMax = false, float fCompMax = 0.0,
		ConVarChangeCallback callback = 0);


	// Used internally by OneTimeInit to initialize.
	void Init() override
	{
		ConCommandBase::Init();
	}
	bitmask::mask<ConVarFlag> GetFlags() { return Parent->Flags; }
public:

	// This either points to "this" or it points to the original declaration of a ConVar.
	// This allows ConVars to exist in separate modules, and they all use the first one to be declared.
	// m_pParent->m_pParent must equal m_pParent (ie: m_pParent must be the root, or original, ConVar).
	ConVar*						Parent;

	const char*					DefaultValue;

	std::unique_ptr<char[]>		StringValue;
	int							StringValueLen;

	float						FloatValue;
	int							IntValue;

	bool						HasMin;
	float						MinValue;
	bool						HasMax;
	float						MaxValue;

	bool						HasCompMin;
	float						CompMinValue;
	bool						HasCompMax;
	float						CompMaxValue;

	bool						IsCompetitiveRestrictions;


	ConVarChangeCallback		ChangeCallback;
};


struct CCmdCharacterSet
{
	char set[256];
};

class CCommand
{
public:
	int count() const noexcept
	{
		return ArgC;
	}

	bool has_count(int num) const noexcept
	{
		return num + 1 == ArgC;
	}
	
	bool has_atleast(int num) const noexcept
	{
		return num + 1 <= ArgC;
	}

	const char** argv() const noexcept
	{
		return ArgC ? (const char**)pArgV : nullptr;
	}

	// All args that occur after the 0th arg, in string form
	const char* str() const noexcept
	{
		return ArgV0Size ? &ArgSBuffer[ArgV0Size] : "";
	}

	// The entire command in string form, including the 0th arg
	const char* all() const noexcept
	{
		return ArgC ? ArgSBuffer : "";
	}

	// Gets at arguments
	const char* operator[](int index) const noexcept
	{
		return pArgV[index];
	}

private:
	enum
	{
		COMMAND_MAX_ARGC = 64,
		COMMAND_MAX_LENGTH = 512,
	};

	int		ArgC;
	int		ArgV0Size;
	char	ArgSBuffer[COMMAND_MAX_LENGTH];
	char	ArgVBuffer[COMMAND_MAX_LENGTH];

	const char* pArgV[COMMAND_MAX_ARGC];
};


#define CMD_TAG "m01_"

#define M01_CONCOMMAND(NAME, FUNCTION_CALLBACK, ...) \
	static ConCommand NAME##_ccmd(CMD_TAG#NAME, FUNCTION_CALLBACK, __VA_ARGS__)



template<typename... _Args>
inline void ReplyToCCmd(const color::u8rgba& clr, const std::string_view& fmt, const _Args&... args)
{
	std::string fmt_str = std::format(fmt, args...);
	Interfaces::CVar->ConsoleColorPrintf(clr, "%s\n", fmt_str.c_str());
}

