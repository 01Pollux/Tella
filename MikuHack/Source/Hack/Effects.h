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
	Vector m_vOrigin;
	Vector m_vStart;
	Vector m_vNormal;
	QAngle m_vAngles;
	int		m_fFlags;
	IBaseHandle m_hEntity;
	float	m_flScale;
	float	m_flMagnitude;
	float	m_flRadius;
	int		m_nAttachmentIndex;
	short	m_nSurfaceProp;

	// Some TF2 specific things
	int		m_nMaterial;
	int		m_nDamageType;
	int		m_nHitBox;

	unsigned char	m_nColor;

	bool							m_bCustomColors;
	te_tf_particle_effects_colors_t	m_CustomColors;

	bool									m_bControlPoint1;
	te_tf_particle_effects_control_point_t	m_ControlPoint1;

	int m_iEffectName;

	CEffectData()
	{
		m_vOrigin.Init();
		m_vStart.Init();
		m_vNormal.Init();
		m_vAngles.Init();

		m_fFlags = 0;
		m_hEntity = INVALID_EHANDLE_INDEX;
		m_flScale = 1.f;
		m_nAttachmentIndex = 0;
		m_nSurfaceProp = 0;

		m_flMagnitude = 0.0f;
		m_flRadius = 0.0f;

		m_nMaterial = 0;
		m_nDamageType = 0;
		m_nHitBox = 0;

		m_nColor = 0;

		m_bCustomColors = false;
		m_CustomColors.m_vecColor1.Init();
		m_CustomColors.m_vecColor2.Init();

		m_bControlPoint1 = false;
		m_ControlPoint1.m_eParticleAttachment = PATTACH_ABSORIGIN;
		m_ControlPoint1.m_vecOffset.Init();
	}
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
	static CTFParticleFactory* ParticleProp(IClientShared* pEnt = pLocalPlayer)
	{
		if (!pEnt)
			return nullptr;
		return pEnt->GetEntProp<CTFParticleFactory>("m_flElasticity", -32);
	}

	CTFParticle* Create(const char* pszParticleName, ParticleAttachment_t iAttachType, int iAttachmentPoint = 0, Vector vecOriginOffset = vec3_origin)
	{
		union {
			CTFParticle* (CTFParticleFactory::*fn)(const char*, ParticleAttachment_t, int, Vector);
			void* ptr = reinterpret_cast<void*>(Library::clientlib.FindPattern("CreateParticle"));
		} static u;

		return (this->*u.fn)(pszParticleName, iAttachType, iAttachmentPoint, vecOriginOffset);
	}

	void StopEmission(CTFParticle* pEffect = NULL, bool bWakeOnStop = false, bool bDestroyAsleepSystems = false)
	{
		union {
			CTFParticle* (CTFParticleFactory::* fn)(CTFParticle*, bool, bool);
			void* ptr = reinterpret_cast<void*>(Library::clientlib.FindPattern("StopParticleEmission"));
		} static u;

		(this->*u.fn)(pEffect, bWakeOnStop, bDestroyAsleepSystems);
	}
};