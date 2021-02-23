#pragma once

#include "AutoList.h"
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_helper.h"

#include <mathlib/vector.h>
#include <Color.h>
#include <array>


namespace DrawTools
{
	namespace ColorTools
	{
#define CONST_COLOR \
		template<typename _Type> \
		constexpr std::array<_Type, 4>

		CONST_COLOR Black		{ 0, 0, 0, 255 };
		CONST_COLOR White		{ 255, 255, 255, 255 };
		CONST_COLOR Grey		{ 255, 255, 255, 125 };

		CONST_COLOR Red			{ 255, 0, 0, 255 };
		CONST_COLOR Green		{ 0, 255, 0, 255 };
		CONST_COLOR Blue		{ 0, 0, 255, 255 };

		CONST_COLOR Fuschia		{ 255, 0, 255, 255 };
		CONST_COLOR Pink		{ 228, 128, 255, 255 }:;

		CONST_COLOR Cyan		{ 0, 231, 231, 255 };
		CONST_COLOR Teal		{ 12, 165, 197, 255 };
		CONST_COLOR Turquoise	{ 12, 165, 197, 255 };
		CONST_COLOR Lime		{ 0, 255, 213, 255 };

		inline Color GetHealth(int cur, int max)
		{
			float ratio = static_cast<float>(cur) / static_cast<float>(max);

			if (ratio > 1)				return { 68,																128,		255,	255 };
			else if (ratio <= 0.5f)		return { 255,								static_cast<int>((2 * ratio) *  255),		0,		255 };
			else						return { static_cast<int>(1.0f - (2.0f * (ratio - 0.5f)) * 255),			255,		0,		255 };
		}

		inline Color GetScore(int min, int cur, int max, const std::array<int, 3>&& rgb)
		{
			float ratio = static_cast<float>(cur) / ((static_cast<float>(max) + static_cast<float>(min)) / 2);

			if (ratio > 1)				return { 68,														rgb[1],				255,	255 };
			else if (ratio <= 0.5f)		return { rgb[0],							  static_cast<int>((2 * rgb[1]) * 255),		rgb[2],	255 };
			else						return { static_cast<int>(1.0f - (2.0f * (rgb[0] - 0.5f)) * 255),	rgb[1],				rgb[2],	255 };
		}

		inline ImColor ToImColor(Color c)
		{
			ImColor i(c.r() / 255, c.g() / 255, c.b() / 255, c.a() / 255);

			return i;
		}

		inline Color FromImColor(ImColor i)
		{
			auto val = i.Value;

			Color c(static_cast<int>(val.x * 255.0f),
				static_cast<int>(val.y * 255.0f),
				static_cast<int>(val.z * 255.0f),
				static_cast<int>(val.w * 255.0f));

			return c;
		}

		inline const Color& FromArray(const std::array<char8_t, 4>& arr)
		{
			return *reinterpret_cast<const Color*>(arr.data());
		}
	}

	constexpr int m_iStringOffset = 21;
	extern std::pair<int, int> m_ScreenSize;
	void InitSurface();

	void MarkForDeletion();
	bool IsMarkedForDeletion();

	bool Init();
	void Destroy();

	void Update();

	void DrawString(int x, int y, const Color& c, std::string& str);
	void Line(int x, int y, int x1, int y1, const Color& c);
	void Rect(int x, int y, int w, int h, const Color& clr);
	void OutlinedRect(int x, int y, int w, int h, const Color& clr);
	void GradientRect(int x, int y, int x1, int y1, const Color& top_clr, const Color& bottom_clr);
	void OutlinedCircle(int x, int y, float radius, int segments, const Color& clr);

	bool WorldToScreen(const Vector& origin, Vector& screen);
};

