#pragma once

#include <array>
#include <mathlib/vector.h>
#include <mathlib/mathlib.h>

class IClientShared;
struct IBoneCache;
struct mstudiobbox_t;

struct IBoneCache
{
	Vector min;
	Vector max;
	Vector center;

	mstudiobbox_t* bbox = nullptr;

	std::array<matrix3x4_t, 128	/* MAXSTUDIOBONES */> bones;

	static bool GetHitbox(IClientShared*, int, IBoneCache*);
};
