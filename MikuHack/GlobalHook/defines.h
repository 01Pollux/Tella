#pragma once

#include <list>
#include <unordered_map>
#include <functional>

#include "../Helpers/VTable.h"


using M0HOOK_INSTANCE	= void*;
using M0HOOK_TAG_NAME	= const char*;

using M0HOOK_STORAGE	= std::unordered_multimap<M0HOOK_TAG_NAME, M0HOOK_INSTANCE>;

inline M0HOOK_STORAGE* GetStorage() noexcept;


template<typename RetType, typename = void>
class M0Hook_Storage;

template<typename RetType>
class M0Hook_Storage<RetType, std::enable_if_t<!std::is_void_v<RetType>>>
{
public:
	RetType LastRet{ };

	const RetType& GetReturnInfo() noexcept
	{
		return LastRet;
	}

	void SetReturnInfo(const RetType& ret) noexcept
	{
		LastRet = ret;
	}
};

template<typename RetType>
class M0Hook_Storage<RetType, std::enable_if_t<std::is_void_v<RetType>>>
{

};