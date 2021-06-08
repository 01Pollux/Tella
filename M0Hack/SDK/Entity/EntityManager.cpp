
#include "UtlVector.hpp"
#include "GlobalHook/listener.hpp"
#include "Helper/Offsets.hpp"
#include "Library/Lib.hpp"

#include "BaseEntity.hpp"

class IBaseEntityInternal;

class IClientEntityListener
{
public:
	static ValveUtlVector<IClientEntityListener*>* FindEntityListener();

	virtual void OnEntityCreated(IBaseEntityInternal* pEnt) { };
	virtual void OnEntityDeleted(IBaseEntityInternal* pEnt) { };

public:
	void AddEntityListener() noexcept;
	void RemoveEntityListener() noexcept;

	IClientEntityListener() = default;
	IClientEntityListener(const IClientEntityListener&) = default;
	IClientEntityListener& operator=(const IClientEntityListener&) = default;
	IClientEntityListener(IClientEntityListener&&) = default;
	IClientEntityListener& operator=(IClientEntityListener&&) = default;

protected:
	~IClientEntityListener() { };
};


class DummyForwardListener : public IClientEntityListener
{
public:
	DummyForwardListener()
	{
		M0EventManager::AddListener(EVENT_KEY_LOAD_DLL, std::bind(&DummyForwardListener::LoadDLL, this), EVENT_NULL_NAME);
		M0EventManager::AddListener(EVENT_KEY_UNLOAD_DLL, std::bind(&DummyForwardListener::UnloadDLL, this), EVENT_NULL_NAME);
	}

	void LoadDLL()
	{
		AddEntityListener();
	}
	
	void UnloadDLL()
	{
		RemoveEntityListener();
	}

	void OnEntityCreated(IBaseEntityInternal* pEnt) final
	{
		if (!pEnt)
			return;

		ClientClass* pCls = pEnt->GetClientClass();
		if (pCls && pCls->ClassID != EntClassID::CSceneEntity)
		{
			if (auto pEvent = M0EventManager::Find(EVENT_KEY_ENTITY_CREATED))
				pEvent(reinterpret_cast<M0EventData*>(pEnt));
		}
	};
	
	void OnEntityDeleted(IBaseEntityInternal* pEnt) final
	{
		if (!pEnt)
			return;

		ClientClass* pCls = pEnt->GetClientClass();
		if (pCls && pCls->ClassID != EntClassID::CSceneEntity)
		{
			if (auto pEvent = M0EventManager::Find(EVENT_KEY_ENTITY_DELETED))
				pEvent(reinterpret_cast<M0EventData*>(pEnt));
		}
	};
} static dummy_entitylistener;


inline ValveUtlVector<IClientEntityListener*>* IClientEntityListener::FindEntityListener()
{
	uintptr_t pEntListener = reinterpret_cast<uintptr_t>(M0Libraries::Client->FindPattern("pCEntityListPtr")) + Offsets::ClientDLL::ClientEntList::EntListenerVec;
	return reinterpret_cast<ValveUtlVector<IClientEntityListener*>*>(pEntListener);
}

void IClientEntityListener::AddEntityListener() noexcept
{
	auto listener = FindEntityListener();
	if (listener->Find(this) == -1)
		listener->AddToTail(this);
}

void IClientEntityListener::RemoveEntityListener() noexcept
{
	auto listener = FindEntityListener();

	if (int pos = listener->Find(this); pos != -1)
		listener->FastRemove(pos);
}
