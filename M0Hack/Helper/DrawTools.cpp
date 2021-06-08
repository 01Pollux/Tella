
#include "DrawTools.hpp"

#include "cdll_int.hpp"
#include "VGUI.hpp"
#include <iostream>

DrawTools::ScreenCoord DrawTools::ScreenPosition;

void DrawTools::Update()
{
	Interfaces::EngineClient->GetScreenSize(ScreenPosition.x, ScreenPosition.y);
}

bool DrawTools::IsVectorInHudSpace(const Vector& pos)
{
	const VMatrix& worldToScreen = Interfaces::EngineClient->WorldToScreenMatrix();
	const double w = worldToScreen[3][0] * pos[0] + worldToScreen[3][1] * pos[1] + worldToScreen[3][2] * pos[2] + worldToScreen[3][3];
	
	return w > 0.001;
}

static bool WorldToScreen(const Vector& origin, Vector& screen)
{
	const VMatrix& worldToScreen = Interfaces::EngineClient->WorldToScreenMatrix();
	const float w = worldToScreen[3][0] * origin[0] + worldToScreen[3][1] * origin[1] + worldToScreen[3][2] * origin[2] + worldToScreen[3][3];
	screen.z = 0;
	
	if (w > 0.001f)
	{
		screen.x = worldToScreen[0][0] * origin[0] + worldToScreen[0][1] * origin[1] + worldToScreen[0][2] * origin[2] + worldToScreen[0][3];
		screen.y = worldToScreen[1][0] * origin[0] + worldToScreen[1][1] * origin[1] + worldToScreen[1][2] * origin[2] + worldToScreen[1][3];

		screen.x *= 1/w;
		screen.y *= 1/w;

		return true;
	}
	else return false;
}

bool DrawTools::GetVectorInHudSpace(Vector pos, ScreenCoord& cord, const Vector& offset)
{
	Vector screen;
	pos += offset;

	if (WorldToScreen(pos, screen))
	{
		cord.x = static_cast<int>(0.5f * (1.0f + screen.x) * ScreenPosition.x);
		cord.y = static_cast<int>(0.5f * (1.0f - screen.y) * ScreenPosition.y);

		return true;
	}
	else return false;
}