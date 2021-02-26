
#include "../Interfaces/CBaseEntity.h"

#include "DataMap.h"
#include "NetVars.h"

#include "Offsets.h"
#include "String.h"

#include "../Helpers/Commons.h"
#include "../Interfaces/IBaseClientDLL.h"
#include "../Interfaces/HatCommand.h"

using std::string;
using DataMapHash = std::unordered_map<string, datamap_info_t>;
DataMapHash m_DataMapHash;


static bool _FindInDataMap(datamap_t* map, const char* name, datamap_info_t* info)
{
	while (map)
	{
		for (int i = 0; i < map->dataNumFields; i++)
		{
			typedescription_t* desc = &map->dataDesc[i];
			const char* field_name = desc->fieldName;
			if (!field_name)
				continue;

			if (!strcmp(field_name, name))
			{
				info->desc = desc;
				info->offset = desc->fieldOffset[TD_OFFSET_NORMAL];
				return true;
			}

			datamap_t* nested = desc->td;
			if (!nested || !_FindInDataMap(nested, name, info))
				continue;

			info->offset += desc->fieldOffset[TD_OFFSET_NORMAL];
			return true;
		}

		map = map->baseMap;
	}
	return false;
}

bool LookupDataMap(IClientShared* pEnt, const char* name, datamap_info_t* info, DataMapType type)
{
	ClientClass* cls = pEnt->GetClientClass();
	if (!cls || !cls->m_pNetworkName)
		return false;

	string actual = cls->m_pNetworkName + string(type == DataMapType::DataMap ? ":DM:":":PM:") + name;
	auto iter = m_DataMapHash.find(actual);
	if (iter == m_DataMapHash.end())
	{
		uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(pEnt);
		union {
			datamap_t* (IClientShared::* fn)();
			uintptr_t ptr;
		} u{ .ptr = vtable[Offsets::IBaseEntity::VTIdx_GetDataMapDesc] };

		datamap_t* map = (pEnt->*u.fn)();
		if (!_FindInDataMap(map, name, info))
		{
			info->valid = false;
			MIKUDebug::LogCritical(fmt::format("DATAMAP: Failed to find: {}", actual));
		}
		else info->valid = true;
		m_DataMapHash.insert(std::make_pair(actual, *info));

	} else *info = iter->second;

	return info->valid;
}

#include "../Interfaces/IVEClientTrace.h"

HAT_COMMAND(set_datamap, "Set DataMap, _01_set_datamap @aim/classname prop_name prop_type prop_value")
{
	if (BadLocal())
		REPLY_TO_TARGET(return, "Command must be used in-game");

	IClientShared* pTarget = nullptr;
	const char* target = args[0];
	if (!strcmp(target, "@aim"))
	{
		ITFPlayer* pMe = ::ILocalPtr();
		
		Vector begin = pMe->EyePosition();
		
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

	enum Prop_Type_
	{
		Prop_Float,
		Prop_Int,
		Prop_String,
		Prop_EHANDLE
	};

	const char* prop_name = args[1];
	Prop_Type_ type = static_cast<Prop_Type_>(atoi(args[2]));
	const char* value = args[3];
	bool get_instead = !*value;

	
	auto prop_validate = 
		[&](auto* prop)
		{
			if (!prop)
				REPLY_TO_TARGET(return false, "Invalid DataMap Prop \"%s\"", prop_name);
			else return true;
		};

	switch (type)
	{
	case Prop_Float:
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
	case Prop_Int:
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
	case Prop_String:
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
	case Prop_EHANDLE:
	{
		IBaseHandle* prop = pTarget->GetEntProp<IBaseHandle, PropType::Data>(prop_name);
		if (prop_validate(prop))
		{
			IBaseHandle& val = *prop;
			REPLY_TO_TARGET(return, "Prop \"%s\" = \"%s\"", prop_name, val.GetEntryIndex());
		}
		break;
	}
	}
}