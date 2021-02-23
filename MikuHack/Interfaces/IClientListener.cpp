
#include "../Helpers/Commons.h"
#include "../GlobalHook/vhook.h"
#include "../GlobalHook/load_routine.h"

#include "IClientListener.h"

class ILisntenerLoadRoutine: public IMainRoutine
{
public:
	void OnLoadDLL() override;
	void OnUnloadDLL() override;
} static dummy_listener;
IEntityCached ent_infos;

inline CUtlVector<IClientEntityListener*>* FindEntityListener()
{
	uintptr_t pEntListener = Library::clientlib.FindPattern("pCEntityListPtr") + Offsets::ClientDLL::To_EntListenerVec;
	return reinterpret_cast<CUtlVector<IClientEntityListener*>*>(pEntListener);
}

void IClientEntityListener::AddEntityListener()
{
	auto listener = FindEntityListener();
	if (listener->Find(this) == listener->InvalidIndex())
		listener->AddToTail(this);
}

void IClientEntityListener::RemoveEntityListener()
{
	auto listener = FindEntityListener();
	listener->FindAndRemove(this);
}



void IEntityCached::OnEntityCreated(IClientShared* pEnt) noexcept
{
	if (!pEnt || !pEnt->GetClientNetworkable())
		return;

	int ent_index = pEnt->entindex();
	if (ent_index < 0)
		return;

	MyClientCacheList cache;
	EntFlag flag;

	if (ent_index <= gpGlobals->maxClients)
		flag = EntFlag::Player;
	else
	{
		ClientClass* cls = pEnt->GetClientClass();
		if (!cls)
			flag = EntFlag::Invalid;
		else {
			switch (cls->m_ClassID)
			{
			case ClassID::ClassID_CObjectSentrygun:
			case ClassID::ClassID_CObjectTeleporter:
			case ClassID::ClassID_CObjectDispenser:
				flag = EntFlag::Building;
				break;

			default:
				flag = EntFlag::Extra;
				break;
			}
		}
	}

	cache.pEnt = pEnt;
	cache.flag = flag;

	m_EntInfos.push_front(cache);
	//	NBoneCache::AddEntity(pEnt);
}

void IEntityCached::OnEntityDeleted(IClientShared* pEnt) noexcept
{
	if (!pEnt || !pEnt->GetClientNetworkable())
		return;
	if (m_EntInfos.remove_if([&pEnt](const MyClientCacheList& cache) -> bool { return cache.pEnt == pEnt; }))
		pEnt->DestroyVirtualGlowObject();
}


void ILisntenerLoadRoutine::OnLoadDLL()
{
	{
		auto level_init = IGlobalVHook<void, const char*>::QueryHook("LevelInit");
		level_init->AddPostHook(HookCall::Any, [](const char*) { ent_infos.reset(); return HookRes::Continue; });

		auto level_shutdown = IGlobalVHook<void>::QueryHook("LevelShutdown");
		level_shutdown->AddPostHook(HookCall::Any, []() { ent_infos.reset(); return HookRes::Continue; });
	}

	ent_infos.AddEntityListener();
	ent_infos.reset();

	if (BadLocal())
		return;

	EntFlag flag;

	int maxClients = gpGlobals->maxClients;
	int maxEntities = clientlist->GetHighestEntityIndex();

	for (int i = 1; i <= maxEntities; i++)
	{
		IClientShared* pEnt = GetIClientEntity(i);
		if (!pEnt)
			continue;

		if (i <= maxClients)
			flag = EntFlag::Player;
		else
		{
			ClientClass* cls = pEnt->GetClientClass();
			if (!cls)
				flag = EntFlag::Invalid;
			else {
				switch (cls->m_ClassID)
				{
				case ClassID::ClassID_CObjectSentrygun:
				case ClassID::ClassID_CObjectTeleporter:
				case ClassID::ClassID_CObjectDispenser:
					flag = EntFlag::Building;
					break;

				default:
					flag = EntFlag::Extra;
					break;
				}
			}
		}

		ent_infos.insert({ pEnt, flag });
	}
}

void ILisntenerLoadRoutine::OnUnloadDLL()
{
	ent_infos.RemoveEntityListener();
	ent_infos.reset();
}
