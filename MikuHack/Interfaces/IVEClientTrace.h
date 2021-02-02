#pragma once

#include "CBaseEntity.h"
#include <engine/IEngineTrace.h>
#include <gametrace.h>
#include <worldsize.h>
#include <engine/ivdebugoverlay.h>
#include <functional>

extern IEngineTrace* clienttrace;
extern IVDebugOverlay* debugoverlay;

using ShouldHitFn = std::function<bool(IHandleEntity*, int)>;

namespace Trace
{
	class ITraceFilterSimple: public CTraceFilter
	{
	public:
		IHandleEntity* pHndl;
		ShouldHitFn callback;

		ITraceFilterSimple(IHandleEntity* pHndl = NULL, ShouldHitFn extra = NULL) : pHndl(pHndl), callback(extra) { };
		ITraceFilterSimple(IClientShared* pEnt = NULL, ShouldHitFn extra = NULL) : pHndl(reinterpret_cast<IHandleEntity*>(pEnt)), callback(extra) { };
		bool ShouldHitEntity(IHandleEntity* pHndl, int mask) override;
	};

	class ILocalFilterSimple : public ITraceFilterSimple
	{
	public:
		ILocalFilterSimple(ShouldHitFn extra = NULL) : ITraceFilterSimple(reinterpret_cast<IHandleEntity*>(pLocalPlayer), extra) { };
	};

	void TraceLine(	const Vector& vec1,
					const Vector& vec2, 
					uint32_t mask, 
					trace_t* results, 
					ITraceFilterSimple* trace );
	

	void TraceHull(	const Vector& vec1,
					const Vector& vec2,
					const Vector& mins,
					const Vector& maxs,
					uint32_t mask,
					trace_t* results,
					ITraceFilterSimple* trace );

	inline bool VectorIsVisible(const Vector& start, 
								const Vector& end, 
								IClientShared* pIgnore,
								IClientShared* pEnt,
								uint32_t mask = MASK_SHOT)
	{
		static trace_t res;
		ITraceFilterSimple filter(pIgnore);
		TraceLine(start, end, mask, &res, &filter);

		return !res.DidHit() || res.m_pEnt == reinterpret_cast<CBaseEntity*>(pEnt);
	}

	inline bool VectorIsVisible(const Vector& start, 
								const Vector& end, 
								IClientShared* pEnt,
								uint32_t mask = MASK_SHOT )
	{
		static trace_t res;
		ILocalFilterSimple filter;
		TraceLine(start, end, mask, &res, &filter);

		return !res.DidHit() || res.m_pEnt == reinterpret_cast<CBaseEntity*>(pEnt);
	}
}

