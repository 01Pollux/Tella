#pragma once

#include "SDK/Glow.hpp"
#include "Helper/Timer.hpp"
#include "BaseEntity.hpp"

class IAimbotGlowHelper
{
public:
	void allocate()
	{
		if (!GlowObj)
			GlowObj = std::make_unique<IGlowObject>();
	}

	void destroy() noexcept
	{
		GlowObj = nullptr;
	}

	void update(const IBaseEntity pEnt, const color::u8rgba& colors) noexcept
	{
		GlowObj->SetEntity(pEnt.get());
		GlowObj->SetColor(colors);
	}

	void invalidate() noexcept
	{
		if (GlowObj)
			GlowObj->SetEntity(nullptr);
	}

private:
	std::unique_ptr<IGlowObject> GlowObj;
};

class IAimbotParticleHelper
{
public:
	void update(IBaseEntity pEnt, const std::string& colors);

	void delete_one(IBaseEntity pEnt)
	{
		if (const auto iter = find(pEnt.get());
			iter != particles.end())
		{
			auto id = iter->second;
			particles.erase(iter);
			ITimerSys::DeleteFuture(id, true);
		}
	}

	void delete_all()
	{
		for (auto& [ent, id] : particles)
			ITimerSys::DeleteFuture(id, true);
	}

	size_t query_size() const noexcept
	{
		return particles.size();
	}

private:
	using DataPair = std::pair<IBaseEntityInternal*, TimerID>;

	std::vector<DataPair>::iterator find(const IBaseEntityInternal* pEnt)
	{
		return std::find_if(
			particles.begin(),
			particles.end(),
			[pEnt](const DataPair& data)
			{
				return data.first == pEnt;
			}
		);
	}

	std::vector<DataPair> particles;
};
