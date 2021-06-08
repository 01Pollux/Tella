#pragma once

#include "Library/Lib.hpp"
#include "MathLib/Vector3D.hpp"
#include "Entity/HandleEntity.hpp"
#include "Entity/BaseEntity.hpp"


enum class ParticleAttachment
{
	AbsOrigin = 0,			// Create at absorigin, but don't follow
	AbsOriginFollow,		// Create at absorigin, and update to follow the entity
	CustomOrigin,			// Create at a custom origin, but don't follow
	Point,					// Create on attachment point, but don't follow
	PointFollow,			// Create on attachment point, and update to follow the entity

	WorldOrigin,			// Used for control points that don't attach to an entity

	RootBoneFollow,			// Create at the root bone of the entity, and update to follow

	Count,
};

#define PARTICLE_DISPATCH_FROM_ENTITY		(1<<0)
#define PARTICLE_DISPATCH_RESET_PARTICLES	(1<<1)


struct TFParticleEffectColor
{
	Vector Color1;
	Vector Color2;
};

struct TFParticleEffectPoint
{
	ParticleAttachment AttachmentType;
	Vector Offset;
};

class TFEffectData
{
public:
	Vector		Origin{ };
	Vector		Start{ };
	Vector		Normal{ };
	QAngle		Angles{ };
	int			Flags{ };
	IBaseHandle Entity;
	float		Scale{ 1.f };
	float		Magnitude{ };
	float		Radius{ };
	int			Attachement_Index{ };
	short		SurfaceProp{ };

	int			Matriel{ };
	int			DamageType{ };
	int			Hitbox{ };

	unsigned char	Color{ };

	bool						HasCustomColors{ };
	TFParticleEffectColor		CustomColors{ };

	bool						HasCustomPoint{ };
	TFParticleEffectPoint		CustomPoint{ };

	int			EffectName;
};

void DispatchParticleEffect(const IBaseEntity entity, const char* particle, const Vector origins, const QAngle angles);


class ITFParticle;
class ITFParticleFactory
{
public:
	ITFParticle*	Create(const char* particle_name, ParticleAttachment attach_type, int attach_point = 0, Vector attach_position = NULL_VECTOR) noexcept;
	void			StopEmission(ITFParticle* pEffect = nullptr, bool bWakeOnStop = false, bool bDestroyAsleepSystems = false) noexcept;
};


class ITFParticleData
{
public:
	ITFParticleData(IBaseEntityInternal* pEnt, const char* particle_name, ParticleAttachment attach_type, int attach_pt = 0, Vector offset = NULL_VECTOR) noexcept :
			Owner(pEnt)
	{
		if (pEnt)
			Particle = pEnt->ParticleProp->Create(particle_name, attach_type, attach_pt, offset);
	}

	~ITFParticleData() noexcept
	{
		if (Owner && Particle)
		{
			Owner->ParticleProp->StopEmission(Particle);
		}
	}

	IBaseEntityInternal* GetOwner() const noexcept
	{
		return Owner;
	}

public:
	ITFParticleData(const ITFParticleData&)				= delete;
	ITFParticleData& operator=(const ITFParticleData&)	= delete;
	ITFParticleData(ITFParticleData&&)					= default;
	ITFParticleData& operator=(ITFParticleData&&)		= default;

private:
	ITFParticle*			Particle{ };
	IBaseEntityInternal*	Owner;
};

using ITFUniqueParticle = std::unique_ptr<ITFParticleData>;