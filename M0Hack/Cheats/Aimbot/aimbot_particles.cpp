#include "aimbot.hpp"
#include "Cheats/Extras/Effects.hpp"


void IAimbotParticleHelper::update(IBaseEntity pEnt, const std::string& name)
{
    if (const auto iter = find(pEnt.get());
        iter != particles.end())
	{
		ITimerSys::RewindBack(iter->second);
	}
	else
	{
        ITFParticleData* paricle = new ITFParticleData(pEnt.get(), name.c_str(), ParticleAttachment::AbsOriginFollow);

        TimerID particle_timer = ITimerSys::CreateFuture(150ms, TimerFlags::ExecuteOnMapEnd,
            [this](TimerRawData data)
            {
                ITFUniqueParticle pInfo{ static_cast<ITFParticleData*>(data) };

                const auto iter = find(pInfo->GetOwner());
                if (iter != particles.end())
                    particles.erase(iter);
            },
            paricle
        );

        particles.emplace_back(pEnt.get(), particle_timer);
	}
}
