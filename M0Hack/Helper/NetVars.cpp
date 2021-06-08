
#include "GameProp.hpp"

#include <iomanip>
#include <fstream>

#include "Helper/Debug.hpp"
#include "cdll_int.hpp"

#include "ConVar.hpp"
#include "BasePlayer.hpp"

#include "Profiler/mprofiler.hpp"

#include "GlobalHook/listener.hpp"


bool IGamePropHelper::FindInRecvTable(const RecvTable* pTable, const char* target_name, CachedRecvInfo* info)
{
	for (int i = 0; i < pTable->NumProps; i++)
	{
		RecvProp* prop = &pTable->Props[i];
		const char* name = prop->VarName;

		if (name && !strcmp(name, target_name))
		{
			info->Prop = prop;
			info->Offset = prop->Offset;
			return true;
		}

		if (prop->DataTable)
		{
			if (!FindInRecvTable(prop->DataTable, target_name, info))
				continue;

			info->Offset += prop->Offset;
			return true;
		}
	}
	return false;
}

bool IGamePropHelper::FindRecvProp(const ClientClass* pCls, const char* prop_name, CachedRecvInfo* info)
{
	if (!FindInRecvTable(pCls->RecvTable, prop_name, info))
	{
		M0Logger::Err("Failed to Find RecvProp {}::{}"sv, pCls->NetworkName, prop_name);
		return false;
	}

	return true;
}

bool IGamePropHelper::FindRecvProp(const char* class_name, const char* prop_name, CachedRecvInfo* info)
{
	ClientClass* pCls{ FindClientClass(class_name) };
	if (!pCls)
		return false;

	return FindInRecvTable(pCls->RecvTable, prop_name, info);
}


ClientClass* IGamePropHelper::FindClientClass(const char* class_name) const noexcept
{
	for (ClientClass* cc = Interfaces::ClientDLL->GetAllClasses(); cc; cc = cc->NextClass)
	{
		if (!strcmp(cc->NetworkName, class_name))
			return cc;
	}

	M0Logger::Err("Failed to Find Client Class {}"sv, class_name);
	return nullptr;
}


void NetVarHook::init(const char* class_name, const char* prop_name, RecvVarProxyFn callback)
{
	IGamePropHelper prop_finder;
	CachedRecvInfo info;
	if (!prop_finder.FindRecvProp(class_name, prop_name, &info))
		throw std::runtime_error(std::format("Cannot NetVar Hook {}::{}"sv, class_name, prop_name));

	this->init(info.Prop, callback);
}


static std::string UTIL_SendFlagsToString(int flags, SendPropType type)
{
	std::string str;

	if (flags & SPROP_COORD)
		str = type == SendPropType::Int ? "VarInt|" : "Normal|";

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

	for (const auto& i : flagsname)
	{
		if (flags & i.flag)
			str += i.name;
	}

	const size_t size = str.size();
	if (size)
		str[size - 1] = '\0';

	return str;
}

static const char* GetDTTypeName(SendPropType type)
{
	constexpr const char* types[]{
		"integer",
		"float",
		"vector",
		"vector-xy",
		"string",
		"array",
		"data-table",
	};
	static_assert(sizeof(types) / sizeof(const char*) == static_cast<size_t>(SendPropType::Count));
	return types[static_cast<std::underlying_type_t<SendPropType>>(type)];
}

static bool DumpRecvTable(std::fstream& file, RecvTable* pTable, uint32_t offset)
{
	file << std::setw(offset);
	for (int i = 0; i < pTable->NumProps; i++)
	{
		RecvProp* pProp = &pTable->Props[i];
		if (pProp->DataTable)
		{
			file << std::format("Table: {} (offset: {}) (type: {})\n", pProp->VarName, pProp->Offset, pProp->DataTable->NetTableName);

			DumpRecvTable(file, pProp->DataTable, offset + 1);
		}
		else 
		{
			const char* type = GetDTTypeName(pProp->RecvType);

			if (type)
			{
				file << std::format(
					"Member: {0} (offset: {1}) (type: {2}) (stride: {3}) ({4})\n", 
					pProp->VarName,											//0
					pProp->Offset,											//1
					type,													//2
					pProp->ElementStride,									//3
					UTIL_SendFlagsToString(pProp->Flags, pProp->RecvType)	//4
				);
			}
			else
			{
				file << std::format(
					"Member: {0} (offset: {1}) (type: {2}) ({3})\n",
					pProp->VarName,											//0
					pProp->Offset,											//1
					static_cast<int>(pProp->RecvType),						//2
					UTIL_SendFlagsToString(pProp->Flags, pProp->RecvType)	//3
				);
			}
		}
	}
	return false;
}

static void DumpAllRecvTables()
{
	IFormatterSV file_fmt(".Miku/Infos/NetVars/{}.txt"sv);
	for (ClientClass* cc = Interfaces::ClientDLL->GetAllClasses(); cc != nullptr; cc = cc->NextClass)
	{
		std::fstream file(file_fmt(cc->NetworkName));
		if (!file)
			break;

		file << "Dump for: \"" << cc->NetworkName << "\"\n\n\n";
		DumpRecvTable(file, cc->RecvTable, 0);
		file << std::flush;
	}
}
M01_CONCOMMAND(dump_recvtable, DumpAllRecvTables, "DumpDataTable");


class LoadGameProps
{
public:
	LoadGameProps()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[](M0EventData*)
			{
				IGamePropHelper prop_finder;

				for (auto& prop : IGameRecvProp::GetEntries())
					prop->LoadOffset(&prop_finder);

				IGameRecvProp::RemoveAll();
			},
			EVENT_NULL_NAME
		);
	}
} static gamepropsmgr;

void IGameRecvProp::LoadOffset(IGamePropHelper* PropMgr)
{
	CachedRecvInfo info;
	if (!PropMgr->FindRecvProp(ClassName, PropName, &info))
		M0Logger::Err("Failed to Find RecvProp {}::{}"sv, ClassName, PropName);
	else
	{
		Offset = info.Offset;
		Prop = info.Prop;
	}
}