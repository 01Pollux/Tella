#pragma once

#include "MathLib/Vector3D.hpp"
#include "Helper/Color.hpp"

class IVDebugOverlay;
class IBaseEntityInternal;

namespace Interfaces
{
	extern IVDebugOverlay* DebugOverlay;
	constexpr const char* DebugOverlayName = "VDebugOverlay003";
}

struct OverlayLine
{
	Vector			Origin;
	Vector			Dest;
	int				R;
	int				G;
	int				B;
	bool			NoDepthTest;
	bool			Draw;
};

class IVDebugOverlay
{
public:
	virtual void AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
	virtual void AddBoxOverlay(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration) = 0;
	virtual void AddTriangleOverlay(const Vector& p1, const Vector& p2, const Vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
	virtual void AddLineOverlay(const Vector& origin, const Vector& dest, int r, int g, int b, bool noDepthTest, float duration) = 0;
	virtual void AddTextOverlay(const Vector& origin, float duration, const char* format, ...) = 0;
	virtual void AddTextOverlay(const Vector& origin, int line_offset, float duration, const char* format, ...) = 0;
	virtual void AddScreenTextOverlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text) = 0;
	virtual void AddSweptBoxOverlay(const Vector& start, const Vector& end, const Vector& mins, const Vector& max, const QAngle& angles, int r, int g, int b, int a, float flDuration) = 0;
	virtual void AddGridOverlay(const Vector& origin) = 0;
	virtual int ScreenPosition(const Vector& point, Vector& screen) = 0;
	virtual int ScreenPosition(float flXPos, float flYPos, Vector& screen) = 0;

	virtual OverlayLine* GetFirst() = 0;
	virtual OverlayLine* GetNext(OverlayLine* current) = 0;
	virtual void ClearDeadOverlays() = 0;
	virtual void ClearAllOverlays() = 0;

	virtual void AddTextOverlayRGB(const Vector& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
	virtual void AddTextOverlayRGB(const Vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;

	virtual void AddLineOverlayAlpha(const Vector& origin, const Vector& dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
	virtual void AddBoxOverlay2(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, const color::u8rgba& faceColor, const color::u8rgba& edgeColor, float duration) = 0;

private:
	inline void AddTextOverlay(const Vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) {} 
};

namespace NDebugOverlay
{
	void	Box(const Vector& origin, const Vector& mins, const Vector& maxs, int r, int g, int b, int a, float flDuration);
	void	BoxDirection(const Vector& origin, const Vector& mins, const Vector& maxs, const Vector& forward, int r, int g, int b, int a, float flDuration);
	void	BoxAngles(const Vector& origin, const Vector& mins, const Vector& maxs, const QAngle& angles, int r, int g, int b, int a, float flDuration);
	void	SweptBox(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, const QAngle& angles, int r, int g, int b, int a, float flDuration);
	void	EntityBounds(IBaseEntityInternal* pEntity, int r, int g, int b, int a, float flDuration);
	void	Line(const Vector& origin, const Vector& target, int r, int g, int b, bool noDepthTest, float flDuration);
	void	Triangle(const Vector& p1, const Vector& p2, const Vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration);
	void	EntityText(int entityID, int text_offset, const char* text, float flDuration, int r = 255, int g = 255, int b = 255, int a = 255);
	void	EntityTextAtPosition(const Vector& origin, int text_offset, const char* text, float flDuration, int r = 255, int g = 255, int b = 255, int a = 255);
	void	Grid(const Vector& vPosition);
	void	Text(const Vector& origin, const char* text, float flDuration);
	void	ScreenText(float fXpos, float fYpos, const char* text, int r, int g, int b, int a, float flDuration);
	void	Cross3D(const Vector& position, const Vector& mins, const Vector& maxs, int r, int g, int b, bool noDepthTest, float flDuration);
	void	Cross3D(const Vector& position, float size, int r, int g, int b, bool noDepthTest, float flDuration);
	void	Cross3DOriented(const Vector& position, const QAngle& angles, float size, int r, int g, int b, bool noDepthTest, float flDuration);
	void	DrawTickMarkedLine(const Vector& startPos, const Vector& endPos, float tickDist, int tickTextDist, int r, int g, int b, bool noDepthTest, float flDuration);
	void	DrawGroundCrossHairOverlay();
	void	HorzArrow(const Vector& startPos, const Vector& endPos, float width, int r, int g, int b, int a, bool noDepthTest, float flDuration);
	void	YawArrow(const Vector& startPos, float yaw, float length, float width, int r, int g, int b, int a, bool noDepthTest, float flDuration);
	void	VertArrow(const Vector& startPos, const Vector& endPos, float width, int r, int g, int b, int a, bool noDepthTest, float flDuration);
	void	Axis(const Vector& position, const QAngle& angles, float size, bool noDepthTest, float flDuration);
	void	Sphere(const Vector& center, float radius, int r, int g, int b, bool noDepthTest, float flDuration);
	void	Circle(const Vector& position, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration);
	void	Circle(const Vector& position, const QAngle& angles, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration);
	void	Circle(const Vector& position, const Vector& xAxis, const Vector& yAxis, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration);
	void	Sphere(const Vector& position, const QAngle& angles, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration);
};