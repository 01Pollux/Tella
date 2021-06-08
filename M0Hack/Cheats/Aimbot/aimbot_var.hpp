#pragma once

#define AIMBOT_SECTION(STR) "Aimbot." ## STR

enum class AimTargetPriority
{
	Fov,
	FovAndDist,

	Highest_Score,
	Lowest_Score,

	Highest_Health,
	Lowest_Health,

	Closest,
	Furthest
};

enum class HitboxPriority
{
	Head,
	Nearest,
	Smart
};

M0CONFIG_BEGIN;

class AimPriority final : public M0Config::Custom<AimTargetPriority, false>
{
	M0CONFIG_INHERIT_FROM(AimPriority, AimTargetPriority, M0Config::Custom<AimTargetPriority, false>);

	M0CONFIG_IMPL_MAP(AimTargetPriority)
	{
		return 
		{
			{ "Fov", AimTargetPriority::Fov },
			{ "Fov And Distance", AimTargetPriority::FovAndDist },

			{ "Highest Score", AimTargetPriority::Highest_Score },
			{ "Lowest Score", AimTargetPriority::Lowest_Score },

			{ "Highest Health", AimTargetPriority::Highest_Health },
			{ "Lowest Health", AimTargetPriority::Lowest_Health },

			{ "Closest", AimTargetPriority::Closest },
			{ "Furthest", AimTargetPriority::Furthest }
		};
	}
};

class HBPriority : public M0Config::Custom<HitboxPriority, false>
{
	M0CONFIG_INHERIT_FROM(HBPriority, HitboxPriority, M0Config::Custom<HitboxPriority, false>);

	M0CONFIG_IMPL_MAP(HitboxPriority)
	{
		return
		{
			{ "Head", HitboxPriority::Head },
			{ "Smart", HitboxPriority::Smart },
			{ "Nearest", HitboxPriority::Nearest }
		};
	}
};

M0CONFIG_END;
