#pragma once

#include "IVEClientTrace.h"

static std::mutex trace_mutex;

namespace Trace
{
	bool ITraceFilterSimple::ShouldHitEntity(IHandleEntity* pHndl, int mask)
	{
		if (pHndl && this->pHndl == pHndl)
			return false;

		if (callback && !callback(pHndl, mask))
			return false;

		return true;
	}

	void TraceLine( const Vector& vec1, 
					const Vector& vec2, 
					uint32_t mask, 
					trace_t* results, 
					ITraceFilterSimple* trace)
	{
		static Ray_t ray;
		ray.Init(vec1, vec2);

		std::lock_guard<std::mutex> protect(trace_mutex);
		clienttrace->TraceRay(ray, mask, trace, results);
	}

	void TraceHull(	const Vector& vec1,
					const Vector& vec2,
					const Vector& mins,
					const Vector& maxs,
					uint32_t mask,
					trace_t* results,
					ITraceFilterSimple* trace)
	{
		static Ray_t ray;
		ray.Init(vec1, vec2, mins, maxs);

		std::lock_guard<std::mutex> protect(trace_mutex);
		clienttrace->TraceRay(ray, mask, trace, results);
	}
}