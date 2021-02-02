#pragma once

#include "../Interfaces/IClientEntityList.h"
#include <array>

class IClientShared;
struct IBoneCache
{
	Vector min = vec3_origin;
	Vector max = vec3_origin;
	Vector center = vec3_origin;

	mstudiobbox_t* bbox = nullptr;

	std::array<matrix3x4_t, MAXSTUDIOBONES> bones;

	static bool GetHitbox(IClientShared*, int, IBoneCache*);
};
