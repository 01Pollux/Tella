#pragma once

#include "../Source/Debug.h"

#include "Library.h"
#include "Timer.h"
#include "NetVars.h"
#include "Offsets.h"

#include "../Interfaces/CBaseEntity.h"
#include "../Interfaces/HatCommand.h"

#include "../Interfaces/IVEngineClient.h"
#include "../Interfaces/IClientEntityList.h"

#include <random>


template<typename T, int size>
constexpr int SizeOfArray(T(&)[size])
{
	return size;
}


namespace Random
{
	template<typename RandomEngine = std::default_random_engine>
	[[nodiscard]] bool Bool()
	{
		std::bernoulli_distribution r(0.5);
		RandomEngine _engine;
		auto b  = r(_engine);
		return r(_engine) ? true:false;
	}

	template<typename RandomEngine = std::default_random_engine>
	[[nodiscard]] int Int(int min, int max)
	{
		std::uniform_int_distribution<int> r(min, max);
		RandomEngine _engine;
		return r(_engine);
	}

	template<typename RandomEngine = std::default_random_engine>
	[[nodiscard]] float Float(float min, float max)
	{
		std::uniform_real_distribution<float> r(min, max);
		RandomEngine _engine;
		return r(_engine);
	}

	template<typename RandomEngine = std::default_random_engine>
	[[nodiscard]] double Double(double min, double max)
	{
		std::uniform_real_distribution<double> r(min, max);
		RandomEngine _engine;
		return r(_engine);
	}
}


namespace
{
	inline IClientShared* GetIClientEntity(int index)
	{
		return reinterpret_cast<IClientShared*>(clientlist->GetClientEntity(index));
	}

	inline IClientShared* GetIClientEntity(IBaseHandle& hndl)
	{
		return reinterpret_cast<IClientShared*>(clientlist->GetClientEntityFromHandle(hndl));
	}

	inline IBaseObject* GetIBaseObject(int index)
	{
		return reinterpret_cast<IBaseObject*>(clientlist->GetClientEntity(index));
	}

	inline IBaseObject* GetIBaseObject(IBaseHandle& hndl)
	{
		return reinterpret_cast<IBaseObject*>(clientlist->GetClientEntityFromHandle(hndl));
	}

	inline ITFPlayer* GetITFPlayer(int index)
	{
		return reinterpret_cast<ITFPlayer*>(clientlist->GetClientEntity(index));
	}

	inline ITFPlayer* GetITFPlayer(IBaseHandle& hndl)
	{
		return reinterpret_cast<ITFPlayer*>(clientlist->GetClientEntityFromHandle(hndl));
	}


	inline int ILocalIdx()
	{
		return engineclient->GetLocalPlayer();
	}

	inline ITFPlayer* ILocalPtr()
	{
		return GetITFPlayer(ILocalIdx());
	}

	inline IBaseObject* ILocalWpn()
	{
		return GetIBaseObject(ILocalPtr()->GetActiveWeapon());
	}


	inline bool BadEntity(IClientShared* pPlayer)
	{
		return !pPlayer || pPlayer->IsDormant();
	}

	inline bool BadLocal()
	{
		return !engineclient->IsInGame() || BadEntity(ILocalPtr());
	}
}