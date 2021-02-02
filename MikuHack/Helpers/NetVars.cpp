#include "sdk.h"
#include "NetVars.h"
#include "../Interfaces/IBaseClientDLL.h"
#include "../Interfaces/HatCommand.h"

using namespace std;

using ClassHash = unordered_map<const char*, ClientClass*>;
ClassHash m_pClasses;
using RecvPropHash = unordered_map<string, recvprop_info_t>;
RecvPropHash m_pRecvHash;

ClientClass* LookupClientClassHash(const char* classname)
{
	auto iter = m_pClasses.find(classname);
	if (iter == m_pClasses.end())
	{
		ClientClass* pClass;
		pClass = clientdll->GetAllClasses();
		while (pClass)
		{
			if (!strcmp(classname, pClass->m_pNetworkName))
			{
				m_pClasses.insert(iter, make_pair(classname, pClass));
				return pClass;
			}
			pClass = pClass->m_pNext;
		}
		m_pClasses.insert(iter, make_pair(classname, static_cast<ClientClass*>(NULL)));
		return NULL;
	}

	return iter->second;
}

bool FindInRecvTable(RecvTable* pTable, const char* szName, recvprop_info_t* info, uint offset)
{
	const char* name;
	int props = pTable->m_nProps;
	RecvProp* pProp;

	for (int i = 0; i < props; i++)
	{
		pProp = &pTable->m_pProps[i];
		name = pProp->m_pVarName;

		if (name && !strcmp(name, szName))
		{
			info->offset = offset + pProp->m_Offset;
			info->pProp = pProp;
			return true;
		}

		if (pProp->m_pDataTable)
		{
			if (FindInRecvTable(pProp->m_pDataTable, szName, info, offset + pProp->m_Offset))
			{
				return true;
			}
		}
	}
	return false;
}


bool LookupRecvPropC(ClientClass* pClass, const char* offset, recvprop_info_t* info)
{
	if (!pClass || !pClass->m_pNetworkName)
		return false;

	string actual = pClass->m_pNetworkName + string("::") + offset;

	auto iter = m_pRecvHash.find(actual);
	if (iter == m_pRecvHash.end())
	{
		recvprop_info_t temp;
		if (!FindInRecvTable(pClass->m_pRecvTable, offset, &temp, 0))
		{
			temp.valid = false;
			Msg("RECVTABLE: Failed to find %s\n", actual.c_str());
		}

		m_pRecvHash.insert(std::make_pair(actual, temp));

		*info = temp;
		return temp.valid;
	}

	*info = iter->second;
	return info->valid;
}

bool LookupRecvProp(const char* classname, const char* offset, recvprop_info_t* info)
{
	ClientClass* pClass = LookupClientClassHash(classname);
	return LookupRecvPropC(pClass, offset, info);
#if 0
	if (!pClass)
		return false;

	string actual = string(classname) + "::" + string(offset);

	auto iter = m_pRecvHash.find(actual);
	if (iter == m_pRecvHash.end())
	{
		recvprop_info_t temp;
		if (!FindInRecvTable(pClass->m_pRecvTable, offset, &temp, 0))
		{
			Warning("RECVTABLE: Failed to find %s\n", actual.c_str());
			temp.valid = false;
		}

		m_pRecvHash.insert(std::make_pair(actual, temp));

		*info = temp;
		return temp.valid;
	}

	*info = iter->second;
	return info->valid;
#endif
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

	static unordered_map<int, const char*> flagsname = {
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
		if (flags & i.first)
		{
			str += i.second;
		}
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

static bool Dump_RecvTable(FILE* file, RecvTable* pTable, uint offset)
{
	int props = pTable->m_nProps;
	RecvProp* pProp;
	const char* type;

	for (int i = 0; i < props; i++)
	{
		pProp = &pTable->m_pProps[i];
		if (pProp->m_pDataTable)
		{
			fprintf(file, "%*sTable: %s (offset %d) (type %s)\n",
				offset, "",
				pProp->m_pVarName,
				pProp->m_Offset,
				pProp->m_pDataTable->m_pNetTableName);

			Dump_RecvTable(file, pProp->m_pDataTable, offset + 1);
		}
		else 
		{
			type = GetDTTypeName(pProp->m_RecvType);

			if (type != NULL)
			{
				fprintf(file, "%*sMember: %s (offset %d) (type %s) (bits %d) (%s)\n",
					offset, "",
					pProp->m_pVarName,
					pProp->m_Offset,
					type,
					pProp->m_ElementStride,
					UTIL_SendFlagsToString(pProp->m_Flags, pProp->m_RecvType));
			}
			else
			{
				fprintf(file, "%*sMember: %s (offset %d) (type %d) (bits %d) (%s)\n",
					offset, "",
					pProp->m_pVarName,
					pProp->m_Offset,
					pProp->m_RecvType,
					pProp->m_ElementStride,
					UTIL_SendFlagsToString(pProp->m_Flags, pProp->m_RecvType));
			}
		}
	}
	return false;
}

static void DumpCBaseEntityTable()
{
	const char* path = "";
	if (!path)
		return;
	char actual[MAX_PATH];
	FILE* file;

	for (ClientClass* cc = clientdll->GetAllClasses(); cc != nullptr; cc = cc->m_pNext)
	{
		snprintf(actual, sizeof(actual), "%s\\%s.txt", path, cc->m_pNetworkName);
		file = fopen(actual, "wt");
		if (!file)
		{
			printf("Invalid File Path: \"%s\"\n", actual);
			break;
		}

		printf("Dumping: \"%s\"\n", cc->m_pNetworkName);
		fprintf(file, "// Dump for \"%s\"\n\n\n", cc->m_pNetworkName);
		Dump_RecvTable(file, cc->m_pRecvTable, 0);
		fclose(file);
	}
}


void InitializeRecvProp()
{
	CleanupRecvProp();
}

void CleanupRecvProp()
{
	m_pClasses.clear();
	m_pRecvHash.clear();
}