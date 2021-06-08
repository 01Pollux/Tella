#pragma once

#include <map>

#include "Interfaces.hpp"
#include "vtable.hpp"

class M0HookStorage
{
public:
	virtual ~M0HookStorage() = default;

	static inline std::multimap<const char*, M0HookStorage*> _Hooks;
};

template<typename, typename = void>
class _M0Hook_Storage;

template<typename RetType>
class _M0Hook_Storage<RetType, std::enable_if_t<!std::is_void_v<RetType>>> : public M0HookStorage
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
class _M0Hook_Storage<RetType, std::enable_if_t<std::is_void_v<RetType>>> : public M0HookStorage { };

namespace M0HookManager
{
	template<typename HookType>
	HookType* AddHook(const char* name)
	{
		if (HookType* hook = FindHook<HookType>(name))
		{
			return hook;
		}
		else
		{
			hook = new HookType;
			M0HookStorage::_Hooks.insert(std::make_pair(name, hook));
			return hook;
		}
	}

	inline void RemoveHook(const char* name) noexcept
	{
		const auto iter = M0HookStorage::_Hooks.find(name);
		if (iter != M0HookStorage::_Hooks.end())
		{
			delete iter->second;
			M0HookStorage::_Hooks.erase(iter);
		}
	}

	template<typename HookType>
	HookType* FindHook(const char* name) noexcept
	{
		const auto iter = M0HookStorage::_Hooks.find(name);
		return iter != M0HookStorage::_Hooks.end() ? static_cast<HookType*>(iter->second) : nullptr;
	}

	namespace Policy
	{
#define MAKE_HOOK_POLICY_(NAME, CUSTOM_NAME, ...) \
		struct NAME \
		{ \
		public: \
			using Hook = __VA_ARGS__; \
			static constexpr const char* Name = CUSTOM_NAME; \
		\
			NAME(bool auto_init = false) { if (auto_init) init(); } \
			void init() { HookMgr = FindHook<Hook>(CUSTOM_NAME); }; \
		\
			Hook* operator->() noexcept { return HookMgr; } \
			operator bool() const noexcept { return HookMgr != nullptr; } \
		\
		private: \
			Hook* HookMgr; \
		}
	}

#define MAKE_VHOOK_POLICY(NAME, CUSTOM_NAME, ...) MAKE_HOOK_POLICY_(NAME, CUSTOM_NAME, IGlobalVHook<__VA_ARGS__>)

#define MAKE_DHOOK_POLICY(NAME, CUSTOM_NAME, ...) MAKE_HOOK_POLICY_(NAME, CUSTOM_NAME, IGlobalDHook<__VA_ARGS__>)

};
