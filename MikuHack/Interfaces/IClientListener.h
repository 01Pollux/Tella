#pragma once

#include <forward_list>
#include "IClientEntityList.h"


class IClientEntityListener;
class IClientShared;

inline CUtlVector<IClientEntityListener*>* FindEntityListener();

class IClientEntityListener
{
public:
	virtual void OnEntityCreated(IClientShared* pEnt) { };
	virtual void OnEntityDeleted(IClientShared* pEnt) { };

public:
	void AddEntityListener();
	void RemoveEntityListener();

protected:
	~IClientEntityListener() = default;
};


enum class EntFlag : char8_t
{
	Invalid,
	Player,
	Building,
	Extra
};

struct MyClientCacheList
{
	IClientShared*	pEnt;
	EntFlag			flag;
};


class IEntityCached : public IClientEntityListener
{
	bool m_bLoaded{ };

public:
	const std::forward_list<MyClientCacheList>& GetInfos() const noexcept { return m_EntInfos; }
	void reset() noexcept { m_EntInfos.clear(); }
	void insert(MyClientCacheList&& info) { m_EntInfos.push_front(info); }

public:	//IClientEntityListener
	void OnEntityCreated(IClientShared* pEnt) noexcept override;
	void OnEntityDeleted(IClientShared* pEnt) noexcept override;

private:
	std::forward_list<MyClientCacheList> m_EntInfos;
};

extern IEntityCached ent_infos;