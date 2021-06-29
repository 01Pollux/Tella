#include "KeyValues.hpp"
#include "Library/Lib.hpp"
#include "Helper/Format.hpp"

#include "GlobalHook/listener.hpp"

class KeyValues_SetLookupTable
{
public:
	KeyValues_SetLookupTable()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[](M0EventData*)
			{
				KeyValues::GetSymbolForStringFn = static_cast<decltype(KeyValues::GetSymbolForStringFn)>(M0Library{ M0ENGINE_DLL }.FindPattern("KeyValues::GetSymbolForString"));
				KeyValues::GetStringForSymbolFn = static_cast<decltype(KeyValues::GetStringForSymbolFn)>(M0Library{ M0ENGINE_DLL }.FindPattern("KeyValues::GetStringForSymbol"));
			},
			EVENT_NULL_NAME
		);
	}
} static dummy_kvlookup;


KeyValues::KeyValues(const char* setName)
{
	Init();
	SetName(setName);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, const char* firstValue)
{
	Init();
	SetName(setName);
	SetString(firstKey, firstValue);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, const wchar_t* firstValue)
{
	Init();
	SetName(setName);
	SetWString(firstKey, firstValue);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, int firstValue)
{
	Init();
	SetName(setName);
	SetInt(firstKey, firstValue);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, const char* firstValue, const char* secondKey, const char* secondValue)
{
	Init();
	SetName(setName);
	SetString(firstKey, firstValue);
	SetString(secondKey, secondValue);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, int firstValue, const char* secondKey, int secondValue)
{
	Init();
	SetName(setName);
	SetInt(firstKey, firstValue);
	SetInt(secondKey, secondValue);
}

void KeyValues::Init()
{
	KeyName = -1;
	DataType = KeyValuesType::None;

	SubKV = PeerKV = ChainKV = nullptr;

	StringValue = nullptr;
	WStringValue = nullptr;
	PtrValue = nullptr;

	HasEscapeSequences = false;
	EvaluateConditionals = true;
}


bool KeyValues::LoadFromFile(const char* fileName, const char* pathID, bool refreshCache)
{
	static IMemberFuncThunk<bool, const char*, const char*, bool> LoadFromFile(M0Library{ M0ENGINE_DLL }.FindPattern("KeyValues::LoadFromBuffer"));
	return LoadFromFile(this, fileName, pathID, refreshCache);
}

const KeyValues* KeyValues::FindKey(int keySymbol) const noexcept
{
	for (KeyValues* dat = SubKV; dat != NULL; dat = dat->PeerKV)
	{
		if (dat->KeyName == keySymbol)
			return dat;
	}

	return nullptr;
}

KeyValues* KeyValues::FindKey(int keySymbol) noexcept
{
	for (KeyValues* dat = SubKV; dat != NULL; dat = dat->PeerKV)
	{
		if (dat->KeyName == keySymbol)
			return dat;
	}

	return nullptr;
}


KeyValues* KeyValues::FindKey(const char* keyName, bool bCreate)
{
	if (!keyName || !keyName[0])
		return this;

	// look for '/' characters deliminating sub fields
	char szBuf[256];
	const char* subStr = strchr(keyName, '/');
	const char* searchStr = keyName;

	// pull out the substring if it exists
	if (subStr)
	{
		int size = subStr - keyName;
		memcpy(szBuf, keyName, size);
		szBuf[size] = 0;
		searchStr = szBuf;
	}

	// lookup the symbol for the search string
	int iSearchStr = GetSymbolForStringFn(searchStr, bCreate);

	if (iSearchStr == -1)
		return nullptr;

	KeyValues* lastItem = NULL;
	KeyValues* dat;

	for (dat = SubKV; dat != nullptr; dat = dat->PeerKV)
	{
		lastItem = dat;	
		if (dat->KeyName == iSearchStr)
			break;
	}

	if (!dat && ChainKV)
		dat = ChainKV->FindKey(keyName, false);

	// make sure a key was found
	if (!dat)
	{
		if (bCreate)
		{
			dat = new KeyValues(searchStr);

			dat->HasEscapeSequences = HasEscapeSequences != 0;	// use same format as parent
			dat->EvaluateConditionals = EvaluateConditionals != 0;

			// insert new key at end of list
			if (lastItem)
				lastItem->PeerKV = dat;
			else
				SubKV = dat;
			dat->PeerKV = nullptr;

			DataType = KeyValuesType::None;
		}
		else
			return nullptr;
	}

	// if we've still got a subStr we need to keep looking deeper in the tree
	if (subStr)
	{
		// recursively chain down through the paths in the string
		return dat->FindKey(subStr + 1, bCreate);
	}

	return dat;
}

const KeyValues* KeyValues::FindKey(const char* keyName) const
{
	if (!keyName || !keyName[0])
		return this;

	// look for '/' characters deliminating sub fields
	char szBuf[256];
	const char* subStr = strchr(keyName, '/');
	const char* searchStr = keyName;

	// pull out the substring if it exists
	if (subStr)
	{
		int size = subStr - keyName;
		memcpy(szBuf, keyName, size);
		szBuf[size] = 0;
		searchStr = szBuf;
	}

	// lookup the symbol for the search string
	int iSearchStr = GetSymbolForStringFn(searchStr, false);

	if (iSearchStr == -1)
		return nullptr;

	KeyValues* lastItem = NULL;
	KeyValues* dat;

	for (dat = SubKV; dat != nullptr; dat = dat->PeerKV)
	{
		lastItem = dat;	
		if (dat->KeyName == iSearchStr)
			break;
	}

	if (!dat && ChainKV)
		dat = ChainKV->FindKey(keyName, false);

	// if we've still got a subStr we need to keep looking deeper in the tree
	if (dat && subStr)
	{
		// recursively chain down through the paths in the string
		return dat->FindKey(subStr + 1);
	}

	return dat;
}


void KeyValues::AddSubKey(KeyValues* pSubkey) noexcept
{
	// add into subkey list
	if (!SubKV)
		SubKV = pSubkey;
	else
	{
		KeyValues* pTempDat = SubKV;
		while (pTempDat->GetNextKey() != NULL)
			pTempDat = pTempDat->GetNextKey();

		pTempDat->SetNextKey(pSubkey);
	}
}

void KeyValues::RemoveSubKey(KeyValues* subKey) noexcept
{
	if (!subKey)
		return;

	// check the list pointer
	if (SubKV == subKey)
	{
		SubKV = subKey->PeerKV;
	}
	else
	{
		// look through the list
		KeyValues* kv = SubKV;
		while (kv->PeerKV)
		{
			if (kv->PeerKV == subKey)
			{
				kv->PeerKV = subKey->PeerKV;
				break;
			}

			kv = kv->PeerKV;
		}
	}

	subKey->PeerKV = nullptr;
}


const KeyValues* KeyValues::GetFirstTrueSubKey() const noexcept
{
	KeyValues* pRet = SubKV;
	while (pRet && pRet->DataType != KeyValuesType::None)
		pRet = pRet->PeerKV;

	return pRet;
}

KeyValues* KeyValues::GetFirstTrueSubKey() noexcept
{
	KeyValues* pRet = SubKV;
	while (pRet && pRet->DataType != KeyValuesType::None)
		pRet = pRet->PeerKV;

	return pRet;
}

const KeyValues* KeyValues::GetNextTrueSubKey() const noexcept
{
	KeyValues* pRet = PeerKV;
	while (pRet && pRet->DataType != KeyValuesType::None)
		pRet = pRet->PeerKV;

	return pRet;
}

KeyValues* KeyValues::GetNextTrueSubKey() noexcept
{
	KeyValues* pRet = PeerKV;
	while (pRet && pRet->DataType != KeyValuesType::None)
		pRet = pRet->PeerKV;

	return pRet;
}


const KeyValues* KeyValues::GetFirstValue() const noexcept
{
	KeyValues* pRet = SubKV;
	while (pRet && pRet->DataType == KeyValuesType::None)
		pRet = pRet->PeerKV;

	return pRet;
}

KeyValues* KeyValues::GetFirstValue() noexcept
{
	KeyValues* pRet = SubKV;
	while (pRet && pRet->DataType == KeyValuesType::None)
		pRet = pRet->PeerKV;

	return pRet;
}

const KeyValues* KeyValues::GetNextValue() const noexcept
{
	KeyValues* pRet = PeerKV;
	while (pRet && pRet->DataType == KeyValuesType::None)
		pRet = pRet->PeerKV;

	return pRet;
}

KeyValues* KeyValues::GetNextValue() noexcept
{
	KeyValues* pRet = PeerKV;
	while (pRet && pRet->DataType == KeyValuesType::None)
		pRet = pRet->PeerKV;

	return pRet;
}


int KeyValues::GetInt(const char* keyName, int defaultValue) const noexcept
{
	const KeyValues* dat = FindKey(keyName);
	if (dat)
	{
		switch (dat->DataType)
		{
		case KeyValuesType::String:
			return atoi(dat->StringValue);
		case KeyValuesType::WString:
			return _wtoi(dat->WStringValue);
		case KeyValuesType::Float:
			return static_cast<int>(dat->FloatValue);
		case KeyValuesType::UInt64:
			return 0;
		case KeyValuesType::Int:
		case KeyValuesType::Pointer:
		default:
			return dat->IntValue;
		};
	}
	return defaultValue;
}


uint64_t KeyValues::GetUint64(const char* keyName, uint64_t defaultValue) const noexcept
{
	const KeyValues* dat = FindKey(keyName);
	if (dat)
	{
		switch (dat->DataType)
		{
		case KeyValuesType::String:
			return atoll(dat->StringValue);
		case KeyValuesType::WString:
			return _wtoi64(dat->WStringValue);
		case KeyValuesType::Float:
			return static_cast<uint64_t>(static_cast<int>(dat->FloatValue));
		case KeyValuesType::UInt64:
			return *(reinterpret_cast<uint64_t*>(dat->StringValue));
		case KeyValuesType::Int:
		case KeyValuesType::Pointer:
		default:
			return dat->IntValue;
		};
	}
	return defaultValue;
}


void* KeyValues::GetPtr(const char* keyName, void* defaultValue) const noexcept
{
	const KeyValues* dat = FindKey(keyName);
	if (dat)
	{
		switch (dat->DataType)
		{
		case KeyValuesType::Pointer:
			return dat->PtrValue;

		case KeyValuesType::WString:
		case KeyValuesType::String:
		case KeyValuesType::Float:
		case KeyValuesType::Int:
		case KeyValuesType::UInt64:
		default:
			return nullptr;
		};
	}
	return defaultValue;
}


float KeyValues::GetFloat(const char* keyName, float defaultValue) const noexcept
{
	const KeyValues* dat = FindKey(keyName);
	if (dat)
	{
		switch (dat->DataType)
		{
		case KeyValuesType::String:
			return static_cast<float>(atof(dat->StringValue));
		case KeyValuesType::WString:
			return static_cast<float>(_wtof(dat->WStringValue));
		case KeyValuesType::Float:
			return dat->FloatValue;
		case KeyValuesType::Int:
			return static_cast<float>(dat->IntValue);
		case KeyValuesType::UInt64:
			return static_cast<float>(*reinterpret_cast<uint64_t*>(dat->StringValue));
		case KeyValuesType::Pointer:
		default:
			return 0.0f;
		};
	}
	return defaultValue;
}


const char* KeyValues::GetString(const char* keyName, const char* defaultValue) const noexcept
{
	const KeyValues* dat = FindKey(keyName);
	if (dat)
	{
		switch (dat->DataType)
		{
		case KeyValuesType::Float:
		{
			std::string buf = std::format("{:6f}", dat->FloatValue);
			const_cast<KeyValues*>(this)->SetString(keyName, buf.c_str());
			break;
		}
		case KeyValuesType::Pointer:
		{
			std::string buf = std::format("{:d}", static_cast<int64_t>(reinterpret_cast<size_t>(dat->PtrValue)));
			const_cast<KeyValues*>(this)->SetString(keyName, buf.c_str());
			break;
		}
		case KeyValuesType::Int:
		{
			std::string buf = std::format("{:d}", dat->IntValue);
			const_cast<KeyValues*>(this)->SetString(keyName, buf.c_str());
			break;
		}
		case KeyValuesType::UInt64:
		{
			std::string buf = std::format("{:d}", *reinterpret_cast<uint64_t*>(dat->StringValue));
			const_cast<KeyValues*>(this)->SetString(keyName, buf.c_str());
			break;
		}

		case KeyValuesType::WString:
		{
			std::string buf = StringTransform<std::string>(dat->WStringValue);
			if (buf.size())
				const_cast<KeyValues*>(this)->SetString(keyName, buf.c_str());
			else
				return defaultValue;
			break;
		}
		case KeyValuesType::String: break;
		default: return defaultValue;
		};

		return dat->StringValue;
	}
	return defaultValue;
}


const wchar_t* KeyValues::GetWString(const char* keyName, const wchar_t* defaultValue) const noexcept
{
	const KeyValues* dat = FindKey(keyName);
	if (dat)
	{
		switch (dat->DataType)
		{
		case KeyValuesType::Float:
		{
			std::wstring buf = std::format(L"{:6f}"sv, dat->FloatValue);
			const_cast<KeyValues*>(this)->SetWString(keyName, buf.c_str());
			break;
		}
		case KeyValuesType::Pointer:
		{
			std::wstring buf = std::format(L"{:d}"sv, static_cast<int64_t>(reinterpret_cast<size_t>(dat->PtrValue)));
			const_cast<KeyValues*>(this)->SetWString(keyName, buf.c_str());
			break;
		}
		case KeyValuesType::Int:
		{
			std::wstring buf = std::format(L"{:d}"sv, dat->IntValue);
			const_cast<KeyValues*>(this)->SetWString(keyName, buf.c_str());
			break;
		}
		case KeyValuesType::UInt64:
		{
			std::wstring buf = std::format(L"{:d}"sv, *reinterpret_cast<uint64_t*>(dat->StringValue));
			const_cast<KeyValues*>(this)->SetWString(keyName, buf.c_str());
			break;
		}

		case KeyValuesType::WString: break;
		case KeyValuesType::String:
		{
			std::wstring buf = StringTransform<std::wstring>(dat->StringValue);
			if (buf.size())
				const_cast<KeyValues*>(this)->SetWString(keyName, buf.c_str());
			else
				return defaultValue;
			break;
		}
		default: return defaultValue;
		};

		return dat->WStringValue;
	}
	return defaultValue;
}


bool KeyValues::GetBool(const char* keyName, bool defaultValue, bool* optGotDefault) const noexcept
{
	if (FindKey(keyName))
	{
		if (optGotDefault)
			(*optGotDefault) = false;
		return GetInt(keyName, 0) != 0;
	}

	if (optGotDefault)
		(*optGotDefault) = true;

	return defaultValue;
}


color::u8rgba KeyValues::GetColor(const char* keyName) const noexcept
{
	color::u8rgba color;
	const KeyValues* dat = FindKey(keyName);
	if (dat)
	{
		switch (dat->DataType)
		{
		case KeyValuesType::Color:
		{
			color[0] = dat->ColorValue[0];
			color[1] = dat->ColorValue[1];
			color[2] = dat->ColorValue[2];
			color[3] = dat->ColorValue[3];
			break;
		}
		case KeyValuesType::Float:
		case KeyValuesType::Int:
		{
			color[0] = static_cast<char8_t>(dat->FloatValue);
			break;
		}

		case KeyValuesType::String:
		{
			float a = 0.0f, b = 0.0f, c = 0.0f, d = 0.0f;
			sscanf_s(dat->StringValue, "%f %f %f %f", &a, &b, &c, &d);

			color[0] = static_cast<char8_t>(a);
			color[1] = static_cast<char8_t>(b);
			color[2] = static_cast<char8_t>(c);
			color[3] = static_cast<char8_t>(d);
			break;
		}
		}
	}
	return color;
}





void KeyValues::SetColor(const char* keyName, color::u8rgba value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->DataType = KeyValuesType::Color;

		dat->ColorValue[0] = value[0];
		dat->ColorValue[1] = value[1];
		dat->ColorValue[2] = value[2];
		dat->ColorValue[3] = value[3];
	}
}


void KeyValues::SetStringValue(char const* strValue)
{
	delete[] StringValue;
	delete[] WStringValue;
	WStringValue = nullptr;

	if (!strValue)
		strValue = "";

	size_t len = ::strlen(strValue) + 1;
	StringValue = new char[len];
	memcpy(StringValue, strValue, len);

	DataType = KeyValuesType::String;
}


void KeyValues::SetString(const char* keyName, const char* value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		if (dat->DataType == KeyValuesType::String && dat->StringValue == value)
			return;

		delete[] dat->StringValue;
		delete[] dat->WStringValue;
		dat->WStringValue = nullptr;

		if (!value)
			value = "";

		size_t len = ::strlen(value) + 1;
		dat->StringValue = new char[len];
		memcpy(dat->StringValue, value, len);

		dat->DataType = KeyValuesType::String;
	}
}


void KeyValues::SetWString(const char* keyName, const wchar_t* value)
{
	KeyValues* dat = FindKey(keyName, true);
	if (dat)
	{
		delete[] dat->WStringValue;
		delete[] dat->StringValue;
		dat->StringValue = nullptr;

		if (!value)
			value = L"";

		size_t len = ::lstrlenW(value) + 1;
		dat->WStringValue = new wchar_t[len];
		memcpy(dat->WStringValue, value, len * sizeof(wchar_t));

		dat->DataType = KeyValuesType::WString;
	}
}


void KeyValues::SetInt(const char* keyName, int value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->IntValue = value;
		dat->DataType = KeyValuesType::Int;
	}
}


void KeyValues::SetUint64(const char* keyName, uint64_t value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		delete[] dat->StringValue;
		delete[] dat->WStringValue;
		dat->WStringValue = nullptr;

		dat->StringValue = new char[sizeof(uint64_t)];
		*reinterpret_cast<uint64_t*>(dat->StringValue) = value;
		dat->DataType = KeyValuesType::UInt64;
	}
}


void KeyValues::SetFloat(const char* keyName, float value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->FloatValue = value;
		dat->DataType = KeyValuesType::Float;
	}
}


void KeyValues::SetPtr(const char* keyName, void* value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->PtrValue = value;
		dat->DataType = KeyValuesType::Pointer;
	}
}


bool KeyValues::IsEmpty(const char* keyName) const noexcept
{
	const KeyValues* dat = FindKey(keyName);
	if (!dat)
		return true;

	return dat->DataType == KeyValuesType::None && dat->SubKV == nullptr;
}


void KeyValues::Clear()
{
	delete SubKV;
	SubKV = NULL;
	DataType = KeyValuesType::None;
}


KeyValues* KeyValues::MakeCopy() const
{
	KeyValues* newKeyValue = new KeyValues(GetName());

	newKeyValue->HasEscapeSequences = HasEscapeSequences != 0;
	newKeyValue->EvaluateConditionals = EvaluateConditionals != 0;

	newKeyValue->DataType = DataType;
	switch (DataType)
	{
	case KeyValuesType::String:
	{
		if (StringValue)
		{
			int len = strlen(StringValue) + 1;
			newKeyValue->StringValue = new char[len];
			memcpy(newKeyValue->StringValue, StringValue, len);
		}
	}
	break;
	case KeyValuesType::WString:
	{
		if (WStringValue)
		{
			int len = lstrlenW(WStringValue) + 1;
			newKeyValue->WStringValue = new wchar_t[len];
			memcpy(newKeyValue->WStringValue, WStringValue, len * sizeof(wchar_t));
		}
	}
	break;

	case KeyValuesType::Int:
		newKeyValue->IntValue = IntValue;
		break;

	case KeyValuesType::Float:
		newKeyValue->FloatValue = FloatValue;
		break;

	case KeyValuesType::Pointer:
		newKeyValue->PtrValue = PtrValue;
		break;

	case KeyValuesType::Color:
		newKeyValue->ColorValue[0] = ColorValue[0];
		newKeyValue->ColorValue[1] = ColorValue[1];
		newKeyValue->ColorValue[2] = ColorValue[2];
		newKeyValue->ColorValue[3] = ColorValue[3];
		break;

	case KeyValuesType::UInt64:
		newKeyValue->StringValue = new char[sizeof(uint64_t)];
		memcpy(newKeyValue->StringValue, StringValue, sizeof(uint64_t));
		break;
	};

	// recursively copy subkeys
	CopySubkeys(newKeyValue);
	return newKeyValue;
}


KeyValues* KeyValues::MakeCopy(bool copySiblings) const
{
	KeyValues* rootDest = MakeCopy();
	if (!copySiblings)
		return rootDest;

	const KeyValues* curSrc = GetNextKey();
	KeyValues* curDest = rootDest;
	while (curSrc) {
		curDest->SetNextKey(curSrc->MakeCopy());
		curDest = curDest->GetNextKey();
		curSrc = curSrc->GetNextKey();
	}

	return rootDest;
}


KeyValuesType KeyValues::GetDataType(const char* keyName) const noexcept
{
	const KeyValues* dat = FindKey(keyName);
	return dat ? dat->DataType : KeyValuesType::None;
}


bool KeyValues::Dump(IKeyValuesDumpContext* pDump, int indentlvl, bool sorted) const
{
	if (!pDump->OnBeginKVSection(this, indentlvl))
		return false;

	if (sorted)
	{
		std::multimap<std::string, const KeyValues*> sortedKeys;

		for (const KeyValues* val = GetFirstValue(); val; val = val->GetNextValue())
			sortedKeys.insert(std::make_pair(std::string(val->GetName()), val));

		for (auto& kvs : sortedKeys)
		{
			if (!pDump->OnKVSection(kvs.second, indentlvl + 1))
				return false;
		}

		sortedKeys.clear();

		for (const KeyValues* sub = GetFirstTrueSubKey(); sub; sub = sub->GetNextTrueSubKey())
			sortedKeys.insert(std::make_pair(std::string(sub->GetName()), sub));

		for (auto& kvs : sortedKeys)
		{
			if (!kvs.second->Dump(pDump, indentlvl + 1, sorted))
				return false;
		}
	}
	else
	{
		for (const KeyValues* val = GetFirstValue(); val; val = val->GetNextValue())
		{
			if (!pDump->OnKVSection(val, indentlvl + 1))
				return false;
		}

		for (const KeyValues* sub = GetFirstTrueSubKey(); sub; sub = sub->GetNextTrueSubKey())
		{
			if (!sub->Dump(pDump, indentlvl + 1))
				return false;
		}
	}

	return pDump->OnEndKVSection(this, indentlvl);
}


void KeyValues::CopySubkeys(KeyValues* pParent) const
{
	KeyValues* pPrev = nullptr;
	for (KeyValues* sub = SubKV; sub; sub = sub->PeerKV)
	{
		KeyValues* dat = sub->MakeCopy();

		if (pPrev)
			pPrev->PeerKV = dat;
		else
			pParent->SubKV = dat;

		dat->PeerKV = nullptr;
		pPrev = dat;
	}
}


KeyValues::~KeyValues()
{
	KeyValues* dat;
	KeyValues* datNext = NULL;
	for (dat = SubKV; dat; dat = datNext)
	{
		datNext = dat->PeerKV;
		dat->PeerKV = nullptr;
		delete dat;
	}

	for (dat = PeerKV; dat && dat != this; dat = datNext)
	{
		datNext = dat->PeerKV;
		dat->PeerKV = nullptr;
		delete dat;
	}

	delete[] StringValue;
	StringValue = NULL;
	delete[] WStringValue;
	WStringValue = NULL;
}