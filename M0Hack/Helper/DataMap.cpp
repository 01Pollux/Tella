
#include "GameProp.hpp"

#include "Offsets.hpp"
#include "Library/Lib.hpp"

#include "Debug.hpp"

bool IGamePropHelper::FindInDataMap(const EntityDataMap* map, const char* name, CachedDataMapInfo* info)
{
	while (map)
	{
		for (int i = 0; i < map->DataNumFields; i++)
		{
			typedescription_t* desc = &map->DataDesc[i];
			const char* field_name = desc->FieldName;
			if (!field_name)
				continue;

			if (!strcmp(field_name, name))
			{
				info->TypeDesc = desc;
				info->Offset = desc->Offset[TD_OFFSET_NORMAL];
				return true;
			}

			EntityDataMap* nested = desc->DataDesc;
			if (!nested || !FindInDataMap(nested, name, info))
				continue;

			info->Offset += desc->Offset[TD_OFFSET_NORMAL];
			return true;
		}

		map = map->BaseMap;
	}
	return false;
}

bool IGamePropHelper::FindDataMap(const EntityDataMap* map, const char* name, CachedDataMapInfo* info)
{
	if (!FindInDataMap(map, name, info))
	{
		M0Logger::Err("DATAMAP: Failed to find: {}", name);
		return false;
	}

	return true;
}



/*#include "../Interfaces/IClientListener.h"
#include <unordered_set>
#include <fstream>
#include <iomanip>


static void DumpDataMap(std::ofstream& file, datamap_t* map, int offset = 0, int deep = 0)
{
	const auto flags_to_string =
	[](int flags, std::string& output)
	{
		constexpr std::pair<int, const char*> NameFlags[]{
			{ FTYPEDESC_GLOBAL, "Global" },
			{ FTYPEDESC_SAVE, "Save" },
			{ FTYPEDESC_KEY, "Key" },
			{ FTYPEDESC_INPUT, "Input" },
			{ FTYPEDESC_OUTPUT, "Output" },
			{ FTYPEDESC_FUNCTIONTABLE, "FunctionTable" },
			{ FTYPEDESC_PTR, "Pointer" },
			{ FTYPEDESC_OVERRIDE, "Override" },
		};

		for (auto& key_name : NameFlags)
		{
			if (flags & key_name.first)
				output += key_name.second + '|';
		}

		if (const auto beg = output.begin(), end = output.end();
			beg != end)
			output.erase(end - 1);
	};

	file << std::setw(10 * deep) << std::setfill('\t');

	while (map)
	{
		for (int i = 0; i < map->dataNumFields; i++)
		{
			typedescription_t* desc = &map->dataDesc[i];
			const char* field_name = desc->fieldName;
			if (!field_name)
				continue;

			datamap_t* nested = desc->td;
			if (nested)
			{
				file << "Table: " << field_name << " - "
					 << nested->dataClassName << '\n';

				DumpDataMap(file, nested, offset + desc->fieldOffset[TD_OFFSET_NORMAL], deep + 1);
			}
			else
			{
				const char* extrenal_name = desc->externalName ? desc->externalName:"";

				std::string flags;
				flags_to_string(desc->flags, flags);

				file << field_name
					 << " - Offset: " << offset + desc->fieldOffset[TD_OFFSET_NORMAL]
					 << " (" << flags << ") "
					 << " - Size: " << desc->fieldSizeInBytes 
					 << extrenal_name << '\n';
			}
		}

		map = map->baseMap;
	}
}

static void DumpEntityDataMap(IClientShared* pEnt, const char* name)
{
	std::ofstream file(fmt::format(".\\Miku\\Infos\\Datamap\\{}.txt", name));
	if (!file)
		return;

	auto cls = pEnt->GetClientClass();
	name = cls ? cls->m_pNetworkName : "UNKNOWN";
	file << "Dump for: \"" << name << "\"\n\n\n";

	datamap_t* map = CallVirtualFunction<datamap_t*>(pEnt, Offsets::IBaseEntity::VTIdx_GetDataMapDesc);
	DumpDataMap(file, map, 0);

	file << std::flush;

	static  bool once = false;
	if (!once && strstr(name, "Sentry") != nullptr)
	{
		once = true;
		datamap_info_t info;
		LookupDataMap(pEnt, "m_flNextAttack", &info, DataMapType::DataMap);
	}

	Msg("Dumping: %s\n", name);
}

#include "../GlobalHook/load_routine.h"

class IDumpListener : public IClientEntityListener, public IMainRoutine
{
	std::unordered_set<std::string> printed_datas;

public:
	void OnEntityCreated(IClientShared* pEnt) final
	{
		if (!pEnt)
			return;

		if (!clienttools)
			return;

		const char* name = CallMemberFunction<const char*>(pEnt, Library::clientlib.FindPattern("CBaseEntity::GetClassName"));

		if (!name)
			return;

		if (printed_datas.find(name) != printed_datas.end())
			return;

		printed_datas.insert(name);

		DumpEntityDataMap(pEnt, name);
	}

	void OnLoadDLL() final
	{
		AddEntityListener();
	}

	void OnUnloadDLL() final
	{
		RemoveEntityListener();
	}
} ;
 
#include "../Interfaces/IVEClientTrace.h"
#include <worldsize.h>

HAT_COMMAND(set_ent_datamap, "Set DataMap, _01_set_datamap @aim/classname prop_name prop_type prop_value")
{
	if (BadLocal())
		REPLY_TO_TARGET(return, "Command must be used in-game");

	IClientShared* pTarget = nullptr;
	const char* target = args[0];
	if (!strcmp(target, "@aim"))
	{
		ITFPlayer* pMe = ::ILocalPtr();
		
		const Vector begin = pMe->EyePosition();

		Vector end;
		AngleVectors(pMe->GetAbsAngles(), &end);
		end *= MAX_TRACE_LENGTH;
		end += begin;

		trace_t tr;
		Trace::ITraceFilterSimple filter(pMe);
		Trace::TraceLine(begin, end, MASK_VISIBLE, &tr, &filter);

		pTarget = reinterpret_cast<IClientShared*>(tr.m_pEnt);
	}
	else {
		for (int i = 0; i < clientlist->GetHighestEntityIndex(); i++)
		{
			IClientShared* pEnt = ::GetIClientEntity(i);
			if (!pEnt)
				continue;

			ClientClass* pCls = pEnt->GetClientClass();
			if (!pCls)
				continue;

			const char* name = clienttools->GetClassname(reinterpret_cast<HTOOLHANDLE>(pEnt));
			if (name && !strcmp(target, name))
			{
				pTarget = pEnt;
				break;
			}
		}
	}

	if (!pTarget)
		REPLY_TO_TARGET(return, "Invalid entity with target \"%s\"", target);

	enum class Prop_Type_
	{
		Prop_Float,
		Prop_Int,
		Prop_String,
		Prop_EHANDLE
	};

	const char* prop_name = args[1];
	const Prop_Type_ type = static_cast<const Prop_Type_>(atoi(args[2]));
	const char* value = args[3];
	const bool get_instead = !*value;
	
	const auto prop_validate = 
		[&](auto* prop)
		{
			if (!prop)
				REPLY_TO_TARGET(return false, "Invalid DataMap Prop \"%s\"", prop_name);
			else return true;
		};

	switch (type)
	{
	case Prop_Type_::Prop_Float:
	{
		float* prop = pTarget->GetEntProp<float, PropType::Data>(prop_name);
		if (prop_validate(prop))
		{
			float& val = *prop;
			if (get_instead)
				REPLY_TO_TARGET(return, "Prop \"%s\" = \"%s\"", prop_name, val);
			else val = atof(value);
		}
		break;
	}
	case Prop_Type_::Prop_Int:
	{
		int* prop = pTarget->GetEntProp<int, PropType::Data>(prop_name);
		if (prop_validate(prop))
		{
			int& val = *prop;
			if (get_instead)
				REPLY_TO_TARGET(return, "Prop \"%s\" = \"%s\"", prop_name, val);
			else val = atoi(value);
		}
		break;
	}
	case Prop_Type_::Prop_String:
	{
		const char** prop = pTarget->GetEntProp<const char*, PropType::Data>(prop_name);
		if (prop_validate(prop))
		{
			const char*& val = *prop;
			if (get_instead)
				REPLY_TO_TARGET(return, "Prop \"%s\" = \"%s\"", prop_name, val);
			else val = value;
		}
		break;
	}
	case Prop_Type_::Prop_EHANDLE:
	{
		IBaseHandle* prop = pTarget->GetEntProp<IBaseHandle, PropType::Data>(prop_name);
		if (prop_validate(prop))
		{
			const IBaseHandle& val = *prop;
			REPLY_TO_TARGET(return, "Prop \"%s\" = \"%s\"", prop_name, val.GetEntryIndex());
		}
		break;
	}
	}
}
*/