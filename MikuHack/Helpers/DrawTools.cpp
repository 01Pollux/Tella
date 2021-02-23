
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "DrawTools.h"
#include "../Interfaces/VGUIS.h"
#include "../Interfaces/IVEngineClient.h"

#include "Main.h"
#include <codecvt>
#include <mathlib/vmatrix.h>

static bool make_close = false;

std::pair<int, int> DrawTools::m_ScreenSize { 1280, 720 };

struct SurfaceTools
{
	vgui::HFont Arial;
	uint iWhiteTexture;
	void Init()
	{
		this->Arial = surface->CreateFont();
		surface->SetFontGlyphSet(this->Arial, "Arial", DrawTools::m_iStringOffset, 500, 0, 0, vgui::ISurface::FONTFLAG_ANTIALIAS);

		iWhiteTexture = surface->CreateNewTextureID();
		const uint8_t colorBuffer[] { 255, 255, 255, 255 };
		surface->DrawSetTextureRGBA(iWhiteTexture, colorBuffer, 1, 1, false, true);
	}
};

static SurfaceTools surfacetools;

void DrawTools::InitSurface()
{
	surfacetools.Init();
}

void DrawTools::MarkForDeletion()
{
	Mmain.m_bIsActive = false;
	make_close = true;
}

bool DrawTools::IsMarkedForDeletion()
{
	return make_close;
}

void DrawTools::Update()
{
	engineclient->GetScreenSize(m_ScreenSize.first, m_ScreenSize.second);
}

bool DrawTools::WorldToScreen(const Vector& origin, Vector& screen)
{
	const VMatrix& worldToScreen = engineclient->WorldToScreenMatrix();
	float w = worldToScreen[3][0] * origin[0] + worldToScreen[3][1] * origin[1] + worldToScreen[3][2] * origin[2] + worldToScreen[3][3];
	screen.z = 0;

	if (w > 0.001) 
	{
		float fl1DBw = 1 / w;
		screen.x = (static_cast<const double>(m_ScreenSize.first) / 2) + (0.5 * ((	static_cast<const double>(worldToScreen[0][0]) * origin[0] +
																					static_cast<const double>(worldToScreen[0][1]) * origin[1] +
																					static_cast<const double>(worldToScreen[0][2]) * origin[2] +
																					static_cast<const double>(worldToScreen[0][3])) * fl1DBw) *
																					static_cast<const double>(m_ScreenSize.first) + 0.5);

		screen.y = (static_cast<const double>(m_ScreenSize.second) / 2) - (0.5 * ((	static_cast<const double>(worldToScreen[1][0]) * origin[0] +
																					static_cast<const double>(worldToScreen[1][1]) * origin[1] +
																					static_cast<const double>(worldToScreen[1][2]) * origin[2] +
																					static_cast<const double>(worldToScreen[1][3])) * fl1DBw) *
																					static_cast<const double>(m_ScreenSize.second) + 0.5);
		return true;
	}

	return false;
}

void DrawTools::DrawString(int x, int y, const Color& c, std::string& str)
{
	if (str.empty())
		return;
	
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	std::wstring ws = converter.from_bytes(str);

	surface->DrawSetTextPos(x, y);
	surface->DrawSetTextFont(surfacetools.Arial);
	surface->DrawSetTextColor(c);

	surface->DrawPrintText(ws.c_str(), ws.size());
}

void DrawTools::Line(int x, int y, int x1, int y1, const Color& c)
{
	surface->DrawSetColor(c);
	surface->DrawLine(x, y, x + x1, y + y1);
}

void DrawTools::Rect(int x, int y, int w, int h, const Color& clr)
{
	surface->DrawSetColor(clr);
	surface->DrawSetTexture(surfacetools.iWhiteTexture);

	vgui::Vertex_t vertices[]{
		{ { static_cast<float>(x),		static_cast<float>(y)		} },
		{ { static_cast<float>(x),		static_cast<float>(y + h)	} },
		{ { static_cast<float>(x + w),	static_cast<float>(y + h)	} },
		{ { static_cast<float>(x + w),	static_cast<float>(y)		} }
	};

	surface->DrawTexturedPolygon(4, vertices);
}

void DrawTools::OutlinedRect(int x, int y, int w, int h, const Color& clr)
{
	Rect(x, y, w, 1, clr);
	Rect(x, y, 1, h, clr);
	Rect(x + w - 1, y, 1, h, clr);
	Rect(x, y + h - 1, w, 1, clr);
}

void DrawTools::GradientRect(int x, int y, int x1, int y1, const  Color& top_clr, const Color& bottom_clr)
{
	surface->DrawSetColor(top_clr);
	surface->DrawFilledRectFade(x, y, x + x1, y + y1, 255, 255, false);
	surface->DrawSetColor(bottom_clr);
	surface->DrawFilledRectFade(x, y, x + x1, y + y1, 0, 255, false);
}

void DrawTools::OutlinedCircle(int x, int y, float radius, int segments, const Color& clr)
{
	float Step = 3.141 * 2.0 / segments;

	for (float a = 0; a < (3.141 * 2.0); a += Step) {
		float x1 = radius * cos(a) + x;
		float y1 = radius * sin(a) + y;
		float x2 = radius * cos(a + Step) + x;
		float y2 = radius * sin(a + Step) + y;
		Line(x1, y1, x2, y2, clr);
	}
}