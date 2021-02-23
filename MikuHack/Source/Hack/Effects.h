#pragma once

#include <mathlib/vector.h>
#include <basehandle.h>

enum ParticleAttachment_t
{
	PATTACH_ABSORIGIN = 0,			// Create at absorigin, but don't follow
	PATTACH_ABSORIGIN_FOLLOW,		// Create at absorigin, and update to follow the entity
	PATTACH_CUSTOMORIGIN,			// Create at a custom origin, but don't follow
	PATTACH_POINT,					// Create on attachment point, but don't follow
	PATTACH_POINT_FOLLOW,			// Create on attachment point, and update to follow the entity

	PATTACH_WORLDORIGIN,			// Used for control points that don't attach to an entity

	PATTACH_ROOTBONE_FOLLOW,		// Create at the root bone of the entity, and update to follow

	MAX_PATTACH_TYPES,
};

#define PARTICLE_DISPATCH_FROM_ENTITY		(1<<0)
#define PARTICLE_DISPATCH_RESET_PARTICLES	(1<<1)

struct te_tf_particle_effects_colors_t
{
	Vector m_vecColor1;
	Vector m_vecColor2;
};

struct te_tf_particle_effects_control_point_t
{
	ParticleAttachment_t m_eParticleAttachment;
	Vector m_vecOffset;
};

class CEffectData
{
public:
	Vector		origin{ };
	Vector		start{ };
	Vector		normal{ };
	QAngle		angles{ };
	int			flags{ };
	IBaseHandle entity{ INVALID_EHANDLE_INDEX };
	float		scale{ 1.f };
	float		magnitude{ };
	float		radius{ };
	int			attachement_index{ };
	short		surface_prop{ };

	int			matriel{ };
	int			dmg_type{ };
	int			hitbox{ };

	unsigned char	color{ };

	bool							has_custom_colors{ };
	te_tf_particle_effects_colors_t	custom_colors{ };

	bool									has_control_point{ };
	te_tf_particle_effects_control_point_t	control_point{ };

	int			effect_name;
};

inline void DispatchParticleEffect(IClientShared* pEnt, const char* particle, Vector vecOrigin, QAngle vecAngles)
{
	using DispatchParticleEffectFn = void(*)(const char*, Vector, QAngle, C_BaseEntity*);
	static DispatchParticleEffectFn DispatchParticle = reinterpret_cast<DispatchParticleEffectFn>(Library::clientlib.FindPattern("DispatchParticleEffect"));
	DispatchParticle(particle, vecOrigin, vecAngles, pEnt->GetBaseEntity());
}


class CTFParticle;
class CTFParticleFactory
{
public:
	static CTFParticleFactory* ParticleProp(IClientShared* pEnt)
	{
		if (!pEnt)
			return nullptr;
		return pEnt->GetEntProp<CTFParticleFactory, PropType::Recv>("m_flElasticity", -32);
	}

	CTFParticle* Create(const char* pszParticleName, ParticleAttachment_t iAttachType, int iAttachmentPoint = 0, Vector vecOriginOffset = vec3_origin)
	{
		union {
			CTFParticle* (CTFParticleFactory::*fn)(const char*, ParticleAttachment_t, int, Vector);
			void* ptr;
		} static u{ .ptr = reinterpret_cast<void*>(Library::clientlib.FindPattern("CreateParticle")) };

		return (this->*u.fn)(pszParticleName, iAttachType, iAttachmentPoint, vecOriginOffset);
	}

	void StopEmission(CTFParticle* pEffect = NULL, bool bWakeOnStop = false, bool bDestroyAsleepSystems = false) noexcept(false)
	{
		union {
			CTFParticle* (CTFParticleFactory::* fn)(CTFParticle*, bool, bool);
			void* ptr;
		} static u{ .ptr = reinterpret_cast<void*>(Library::clientlib.FindPattern("StopParticleEmission")) };

		(this->*u.fn)(pEffect, bWakeOnStop, bDestroyAsleepSystems);
	}
};

class ITFParticleData
{
	CTFParticleFactory* owner;
	CTFParticle* particle;

public:
	explicit ITFParticleData(ITFPlayer* pPlayer, const char* particle_name, ParticleAttachment_t attach_type, int attach_pt = 0, Vector offset = vec3_origin)
	{
		owner = CTFParticleFactory::ParticleProp(pPlayer);
		if (owner)
			particle = owner->Create(particle_name, attach_type, attach_pt, offset);
	}

	~ITFParticleData()
	{
		try
		{
			if (owner)
				owner->StopEmission(particle);
			owner = nullptr;
		}
		catch (...)
		{
			owner = nullptr;
		}
	}

	ITFParticleData(const ITFParticleData&)				= delete;
	ITFParticleData& operator=(const ITFParticleData&)	= delete;
	ITFParticleData(ITFParticleData&&)					= delete;
	ITFParticleData& operator=(ITFParticleData&&)		= delete;
};