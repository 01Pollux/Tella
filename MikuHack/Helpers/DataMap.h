#pragma once


#include <unordered_map>
#include <datamap.h>



enum DataMapType : char8_t
{
	DataMap,
	PredictionMap
};

struct datamap_info_t
{
	typedescription_t*	desc { };
	uint32_t			offset{ };
	bool				valid{ false };
};

class IClientShared;
bool LookupDataMap(IClientShared* pClass, const char* offset, datamap_info_t* info, DataMapType type);
