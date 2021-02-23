
#include "NetVars.h"
#include "../Interfaces/IBaseClientDLL.h"
#include "../Interfaces/HatCommand.h"
#include "../Source/Debug.h"

#include <iomanip>
#include <fstream>

using namespace std;

using RecvPropHash = unordered_map<string, recvprop_info_t>;
RecvPropHash m_RecvHash;


static bool _FindInRecvTable(RecvTable* pTable, const char* target_name, recvprop_info_t* info)
{
	for (int i = 0; i < pTable->m_nProps; i++)
	{
		RecvProp* prop = &pTable->m_pProps[i];
		const char* name = prop->m_pVarName;

		if (name && !strcmp(name, target_name))
		{
			info->pProp = prop;
			info->offset = prop->m_Offset;
			return true;
		}

		if (prop->m_pDataTable)
		{
			if (!_FindInRecvTable(prop->m_pDataTable, target_name, info))
				continue;

			info->offset += prop->m_Offset;
			return true;
		}
	}
	return false;
}

bool LookupRecvPropC(ClientClass* pClass, const char* name, recvprop_info_t* info)
{
	if (!pClass || !pClass->m_pNetworkName)
		return false;

	string actual = pClass->m_pNetworkName + string("::") + name;

	auto iter = m_RecvHash.find(actual);
	if (iter == m_RecvHash.end())
	{
		if (!_FindInRecvTable(pClass->m_pRecvTable, name, info))
			info->valid = false;
		else info->valid = true;

		m_RecvHash.insert(std::make_pair(actual, *info));
	}
	else *info = iter->second;

	return info->valid;
}

static bool LookupRecvProp(const char* class_name, const char* prop_name, recvprop_info_t* info)
{
	for (ClientClass* cc = clientdll->GetAllClasses(); cc != nullptr; cc = cc->m_pNext)
	{
		if (!strcmp(class_name, cc->m_pNetworkName))
			return LookupRecvPropC(cc, prop_name, info);
	}
	return false;
}


NetVarHook::NetVarHook(const char* class_name, const char* prop_name, RecvVarProxyFn callback)
{
	recvprop_info_t info;
	if (!LookupRecvProp(class_name, prop_name, &info))
	{
		std::string str = "Cannot NetVar Hook " + std::string(class_name) + "::" + prop_name;
		throw std::runtime_error(str);
	}
	this->Init(info.pProp, callback);
}


static char* UTIL_SendFlagsToString(int flags, SendPropType type)
{
	static string str;
	str.clear();

	if (flags & SPROP_COORD)
	{
		if (type == SendPropType::DPT_Int)
		{
			str += "VarInt|";
		}
		else
		{
			str += "Normal|";
		}
	}

	class FlagAndName {
	public:
		int flag;
		std::string_view name;

		constexpr FlagAndName(int flag, std::string_view name): flag(flag), name(name) { }
	};
	constexpr FlagAndName flagsname[] = {
		{SPROP_UNSIGNED,			"Unsigned|"},
		{SPROP_COORD,				"Coord|"},
		{SPROP_NOSCALE,				"NoScale|"},
		{SPROP_ROUNDDOWN,			"RoundDown|"},
		{SPROP_ROUNDUP,				"RoundUp|"},
		{SPROP_EXCLUDE,				"Exclude|"},
		{SPROP_XYZE,				"XYZE|"},
		{SPROP_INSIDEARRAY,			"InsideArray|"},
		{SPROP_PROXY_ALWAYS_YES,	"AlwaysProxy|"},
		{SPROP_CHANGES_OFTEN,		"ChangesOften|"},
		{SPROP_IS_A_VECTOR_ELEM,	"VectorElem|"},
		{SPROP_COLLAPSIBLE,			"Collapsible|"},
		{SPROP_COORD_MP,			"CoordMP|"},
		{SPROP_COORD_MP_LOWPRECISION,"CoordMPLowPrec|"},
		{SPROP_COORD_MP_intEGRAL,	"CoordMpIntegral|"},
	};

	for (auto i : flagsname)
	{
		if (flags & i.flag)
			str += i.name;
	}

	return (char*)str.c_str();
}

static const char* GetDTTypeName(SendPropType type)
{
	static unordered_map<SendPropType, const char*> types = {
		{SendPropType::DPT_Int,			"integer"},
		{SendPropType::DPT_Float,		"float"},
		{SendPropType::DPT_Vector,		"vector"},
		{SendPropType::DPT_String,		"string"},
		{SendPropType::DPT_Array,		"array"},
		{SendPropType::DPT_DataTable,	"datatable"},
		{SendPropType::DPT_String,		"string"},
	};
	auto i = types.find(type);
	if (i == types.end())
	{
		return NULL;
	}
	return i->second;
}

static bool DumpRecvTable(std::fstream& file, RecvTable* pTable, uint32_t offset)
{
	for (int i = 0; i < pTable->m_nProps; i++)
	{
		RecvProp* pProp = &pTable->m_pProps[i];
		if (pProp->m_pDataTable)
		{
			file << std::setw(offset) << "Table: " << pProp->m_pVarName << 
										" (offset " << pProp->m_Offset << 
										") (type " << pProp->m_pDataTable->m_pNetTableName << ")\n";

			DumpRecvTable(file, pProp->m_pDataTable, offset + 1);
		}
		else 
		{
			const char* type = GetDTTypeName(pProp->m_RecvType);

			if (type)
			{
				file << std::setw(offset) << "Member: " << pProp->m_pVarName << 
											" (offset "<< pProp->m_Offset << 
											") (type " << type << 
											") (stride " << pProp->m_ElementStride << ") (" 
										  << UTIL_SendFlagsToString(pProp->m_Flags, pProp->m_RecvType) <<")\n";
			}
			else
			{
				file << std::setw(offset) << "Member: " << pProp->m_pVarName << 
											" (offset " << pProp->m_Offset << 
											") (type " << static_cast<int>(pProp->m_RecvType) << ") (" 
										  << UTIL_SendFlagsToString(pProp->m_Flags, pProp->m_RecvType) << ")\n";
			}
		}
	}
	return false;
}

static void DumpAllRecvTables(const char* path0)
{
	for (ClientClass* cc = clientdll->GetAllClasses(); cc != nullptr; cc = cc->m_pNext)
	{
		std::string path = path0 + std::string(cc->m_pNetworkName) + ".txt";
		std::fstream file(path);
		if (!file)
			break;

		file << "Dump for: \"" << cc->m_pNetworkName << "\"\n\n\n";
		DumpRecvTable(file, cc->m_pRecvTable, 0);
		file << std::flush;
	}
}

HAT_COMMAND(dump_recvtable, "DumpDataTable")
{
	DumpAllRecvTables(args.ArgS());
}
