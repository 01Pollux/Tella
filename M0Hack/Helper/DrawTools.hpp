#pragma once

#include "ImGui/imgui.h"
#include "ImGui/imgui_helper.h"

#include "MathLib/Vector3D.hpp"
#include "Color.hpp"
#include <array>


namespace DrawTools
{
	inline color::u8rgba GetHealth(int cur, int max)
	{
		const float ratio = static_cast<float>(cur) / static_cast<float>(max);
		constexpr auto lm = 2 * 255;
		if (ratio > 1)
			return {
			68,
			128,
			255,
			255
		};
		else if (ratio <= 0.5f)
			return {
			255,
			static_cast<char8_t>(ratio * 510),
			0,
			255
		};
		else
			return {
			static_cast<char8_t>(1.0f - (510.f * (ratio - 0.5f))),
			255,
			0,
			255
		};
	}

	inline color::u8rgba GetScore(int min, int cur, int max, const color::u8rgba& rgb)
	{
		const float ratio = static_cast<float>(cur) / ((static_cast<float>(max) + static_cast<float>(min)) / 2);

		if (ratio > 1)
			return {
			68,
			rgb.g,
			255,
			255
		};
		else if (ratio <= 0.5f)
			return {
			rgb.r,
			static_cast<char8_t>(ratio * 510),
			rgb.b,
			255
		};
		else
			return {
			static_cast<char8_t>(1.0f - (510.f * (ratio - 0.5f))),
			rgb.g,
			rgb.b,
			255
		};
	}

	struct ScreenCoord
	{
		int x{ };
		int y{ };
	};
	extern ScreenCoord ScreenPosition;

	void Update();

	bool IsVectorInHudSpace(const Vector& pos);
	bool GetVectorInHudSpace(Vector pos, ScreenCoord& cord, const Vector& offset = NULL_VECTOR);
};

