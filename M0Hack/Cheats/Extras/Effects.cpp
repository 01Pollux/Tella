#include "Effects.hpp"

void DispatchParticleEffect(const IBaseEntity entity, const char* particle, Vector origins, QAngle angles)
{
	static IMemberFuncThunk<void, const char*, Vector, QAngle> dispatch_particle(M0Libraries::Client->FindPattern("DispatchParticleEffect"));
	dispatch_particle(entity.get(), particle, origins, angles);
}


ITFParticle* ITFParticleFactory::Create(const char* particle_name, ParticleAttachment attach_type, int attach_point, Vector attach_position) noexcept
{
	static IMemberFuncThunk<ITFParticle*, const char*, ParticleAttachment, int, Vector> create_particle(M0Libraries::Client->FindPattern("CreateParticle"));
	return create_particle(
		this,
		particle_name,
		attach_type,
		attach_point,
		attach_position
	);
}

void ITFParticleFactory::StopEmission(ITFParticle* pEffect, bool bWakeOnStop, bool bDestroyAsleepSystems) noexcept
{
	static IMemberFuncThunk<void, ITFParticle*, bool, bool> stop_emission(M0Libraries::Client->FindPattern("StopParticleEmission"));
	return stop_emission(
		this,
		pEffect,
		bWakeOnStop,
		bDestroyAsleepSystems
	);
}