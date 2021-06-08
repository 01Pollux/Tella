#include "Trace.hpp"
#include "DebugOverlay.hpp"
#include <iostream>

namespace Trace
{
	void DrawRay(
		const GameRay& ray,
		float duration
	)
	{
		if (ray.Extents.x != 0.0f || ray.Extents.y != 0.0f || ray.Extents.z != 0.0f)
			NDebugOverlay::Line(ray.Start, ray.Start + ray.Delta, 255, 0, 0, true, duration);
		else
			NDebugOverlay::Line(ray.Start, ray.Start + ray.Delta, 255, 255, 0, true, duration);
	}

	void TraceLine(
		const Vector& vec1,
		const Vector& vec2,
		int mask,
		GameTrace* results,
		ITraceFilter* trace
	)
	{
		GameRay ray(vec1, vec2);
		Interfaces::ClientTrace->TraceRay(ray, mask, trace, results);
	}

	void TraceHull(
		const Vector& vec1,
		const Vector& vec2,
		const Vector& mins,
		const Vector& maxs,
		int mask,
		GameTrace* results,
		ITraceFilter* trace
	)
	{
		GameRay ray(vec1, vec2, mins, maxs);
		Interfaces::ClientTrace->TraceRay(ray, mask, trace, results);
	}
}