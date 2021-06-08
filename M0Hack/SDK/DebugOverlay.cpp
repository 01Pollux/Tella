#include "DebugOverlay.hpp"

#include "Entity/BasePlayer.hpp"
#include "Entity/BaseEntity.hpp"
#include "Trace.hpp"
#include "Const.hpp"

#include <algorithm>


#define	MAX_OVERLAY_DIST_SQR	90000000

//-----------------------------------------------------------------------------
// Purpose: Draw a box with no orientation
//-----------------------------------------------------------------------------
void NDebugOverlay::Box(const Vector& origin, const Vector& mins, const Vector& maxs, int r, int g, int b, int a, float flDuration)
{
	BoxAngles(origin, mins, maxs, NULL_VECTOR, r, g, b, a, flDuration);
}

//-----------------------------------------------------------------------------
// Purpose: Draw box oriented to a Vector direction
//-----------------------------------------------------------------------------
void NDebugOverlay::BoxDirection(const Vector& origin, const Vector& mins, const Vector& maxs, const Vector& orientation, int r, int g, int b, int a, float duration)
{
	// convert forward vector to angles
	QAngle f_angles;
	f_angles.y = VectorHelper::VecToYaw(orientation);

	BoxAngles(origin, mins, maxs, f_angles, r, g, b, a, duration);
}

//-----------------------------------------------------------------------------
// Purpose: Draw box oriented to a QAngle direction
//-----------------------------------------------------------------------------
void NDebugOverlay::BoxAngles(const Vector& origin, const Vector& mins, const Vector& maxs, const QAngle& angles, int r, int g, int b, int a, float duration)
{
	Interfaces::DebugOverlay->AddBoxOverlay(origin, mins, maxs, angles, r, g, b, a, duration);
}

//-----------------------------------------------------------------------------
// Purpose: Draws a swept box
//-----------------------------------------------------------------------------
void NDebugOverlay::SweptBox(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, const QAngle& angles, int r, int g, int b, int a, float flDuration)
{
	Interfaces::DebugOverlay->AddSweptBoxOverlay(start, end, mins, maxs, angles, r, g, b, a, flDuration);
}

//-----------------------------------------------------------------------------
// Purpose: Draws a box around an entity
//-----------------------------------------------------------------------------
void NDebugOverlay::EntityBounds(IBaseEntityInternal* pEntity, int r, int g, int b, int a, float flDuration)
{
	const ICollideable* pCollide = pEntity->CollisionProp.data();
	BoxAngles(pCollide->GetCollisionOrigin(), pCollide->OBBMins(), pCollide->OBBMaxs(), pCollide->GetCollisionAngles(), r, g, b, a, flDuration);
}

//-----------------------------------------------------------------------------
// Purpose: Draws a line from one position to another
//-----------------------------------------------------------------------------
void NDebugOverlay::Line(const Vector& origin, const Vector& target, int r, int g, int b, bool noDepthTest, float duration)
{
	// --------------------------------------------------------------
	// Clip the line before sending so we 
	// don't overflow the client message buffer
	// --------------------------------------------------------------
	ITFPlayer player(true);
	if (!player)
		return;

	const Vector& my_origin = player->GetAbsOrigin();

	// Clip line that is far away
	if (((my_origin - origin).LengthSqr() > MAX_OVERLAY_DIST_SQR) &&
		((my_origin - target).LengthSqr() > MAX_OVERLAY_DIST_SQR))
		return;

	// Clip line that is behind the client 
	Vector clientForward;
	player->EyeVectors(&clientForward);

	Vector toOrigin = origin - my_origin;
	Vector toTarget = target - my_origin;

	if (clientForward.Dot(toOrigin) < 0 && clientForward.Dot(toTarget) < 0)
		return;

	Interfaces::DebugOverlay->AddLineOverlay(origin, target, r, g, b, noDepthTest, duration);
}


//-----------------------------------------------------------------------------
// Purpose: Draw triangle
//-----------------------------------------------------------------------------
void NDebugOverlay::Triangle(const Vector& p1, const Vector& p2, const Vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration)
{
	ITFPlayer player(true);
	if (!player)
		return;

	// Clip triangles that are far away
	Vector to1 = p1 - player->GetAbsOrigin();
	Vector to2 = p2 - player->GetAbsOrigin();
	Vector to3 = p3 - player->GetAbsOrigin();

	if ((to1.LengthSqr() > MAX_OVERLAY_DIST_SQR) &&
		(to2.LengthSqr() > MAX_OVERLAY_DIST_SQR) &&
		(to3.LengthSqr() > MAX_OVERLAY_DIST_SQR))
	{
		return;
	}

	// Clip triangles that are behind the client 
	Vector clientForward;
	player->EyeVectors(&clientForward);

	if (clientForward.Dot(to1) < 0 &&
		clientForward.Dot(to2) < 0 &&
		clientForward.Dot(to3) < 0)
		return;

	Interfaces::DebugOverlay->AddTriangleOverlay(p1, p2, p3, r, g, b, a, noDepthTest, duration);
}

//-----------------------------------------------------------------------------
// Purpose: Draw entity text overlay
//-----------------------------------------------------------------------------
void NDebugOverlay::EntityText(int entityID, int text_offset, const char* text, float duration, int r, int g, int b, int a)
{
	Interfaces::DebugOverlay->AddEntityTextOverlay(
		entityID, 
		text_offset, 
		duration,
		static_cast<int>(std::clamp(r * 255.f, 0.f, 255.f)),
		static_cast<int>(std::clamp(g * 255.f, 0.f, 255.f)),
		static_cast<int>(std::clamp(b * 255.f, 0.f, 255.f)),
		static_cast<int>(std::clamp(a * 255.f, 0.f, 255.f)),
		text);
}

//-----------------------------------------------------------------------------
// Purpose: Draw entity text overlay at a specific position
//-----------------------------------------------------------------------------
void NDebugOverlay::EntityTextAtPosition(const Vector& origin, int text_offset, const char* text, float duration, int r, int g, int b, int a)
{
	Interfaces::DebugOverlay->AddTextOverlayRGB(origin, text_offset, duration, r, g, b, a, "%s", text);
}

//-----------------------------------------------------------------------------
// Purpose: Add grid overlay 
//-----------------------------------------------------------------------------
void NDebugOverlay::Grid(const Vector& vPosition)
{
	Interfaces::DebugOverlay->AddGridOverlay(vPosition);
}

//-----------------------------------------------------------------------------
// Purpose: Draw debug text at a position
//-----------------------------------------------------------------------------
void NDebugOverlay::Text(const Vector& origin, const char* text, float duration)
{
	ITFPlayer player(true);
	if (!player)
		return;

	// Clip text that is far away
	if ((player->GetAbsOrigin() - origin).LengthSqr() > MAX_OVERLAY_DIST_SQR)
		return;

	// Clip text that is behind the client 
	Vector clientForward;
	player->EyeVectors(&clientForward);

	Vector toText = origin - player->GetAbsOrigin();

	if (clientForward.Dot(toText) < 0)
		return;

	Interfaces::DebugOverlay->AddTextOverlay(origin, duration, "%s", text);
}

//-----------------------------------------------------------------------------
// Purpose: Add debug overlay text with screen position
//-----------------------------------------------------------------------------
void NDebugOverlay::ScreenText(float flXpos, float flYpos, const char* text, int r, int g, int b, int a, float duration)
{
	Interfaces::DebugOverlay->AddScreenTextOverlay(flXpos, flYpos, duration, r, g, b, a, text);
}

//-----------------------------------------------------------------------------
// Purpose: Draw a colored 3D cross of the given hull size at the given position
//-----------------------------------------------------------------------------
void NDebugOverlay::Cross3D(const Vector& position, const Vector& mins, const Vector& maxs, int r, int g, int b, bool noDepthTest, float fDuration)
{
	Vector start = mins + position;
	Vector end = maxs + position;
	Line(start, end, r, g, b, noDepthTest, fDuration);

	start.x += (maxs.x - mins.x);
	end.x -= (maxs.x - mins.x);
	Line(start, end, r, g, b, noDepthTest, fDuration);

	start.y += (maxs.y - mins.y);
	end.y -= (maxs.y - mins.y);
	Line(start, end, r, g, b, noDepthTest, fDuration);

	start.x -= (maxs.x - mins.x);
	end.x += (maxs.x - mins.x);
	Line(start, end, r, g, b, noDepthTest, fDuration);
}

//-----------------------------------------------------------------------------
// Purpose: Draw a colored 3D cross of the given size at the given position
//-----------------------------------------------------------------------------
void NDebugOverlay::Cross3D(const Vector& position, float size, int r, int g, int b, bool noDepthTest, float flDuration)
{
	Line(position + Vector(size, 0, 0), position - Vector(size, 0, 0), r, g, b, noDepthTest, flDuration);
	Line(position + Vector(0, size, 0), position - Vector(0, size, 0), r, g, b, noDepthTest, flDuration);
	Line(position + Vector(0, 0, size), position - Vector(0, 0, size), r, g, b, noDepthTest, flDuration);
}

//-----------------------------------------------------------------------------
// Purpose: Draw an oriented, colored 3D cross of the given size at the given position (via a vector)
//-----------------------------------------------------------------------------
void NDebugOverlay::Cross3DOriented(const Vector& position, const QAngle& angles, float size, int r, int g, int b, bool noDepthTest, float flDuration)
{
	Vector forward, right, up;
	VectorHelper::AngleVectors(angles, &forward, &right, &up);

	forward *= size;
	right *= size;
	up *= size;

	Line(position + right, position - right, r, g, b, noDepthTest, flDuration);
	Line(position + forward, position - forward, r, g, b, noDepthTest, flDuration);
	Line(position + up, position - up, r, g, b, noDepthTest, flDuration);
}

//--------------------------------------------------------------------------------
// Purpose : Draw tick marks between start and end position of the given distance
//			 with text every tickTextDist steps apart. 
//--------------------------------------------------------------------------------
void NDebugOverlay::DrawTickMarkedLine(const Vector& startPos, const Vector& endPos, float tickDist, int tickTextDist, int r, int g, int b, bool noDepthTest, float duration)
{
	ITFPlayer player(true);
	if (!player)
		return;

	Vector	lineDir = (endPos - startPos);
	double	lineDist = lineDir.NormalizeInPlace();
	int		numTicks = static_cast<int>(lineDist / tickDist);
	Vector	vBodyDir;
	
	VectorHelper::AngleVectors(player->EyeAngles, &vBodyDir);

	Vector  upVec = vBodyDir * 4;
	Vector	tickPos = startPos;
	int		tickTextCnt = 0;

	Vector	sideDir
	{
		lineDir.y * upVec.z - lineDir.z * upVec.y,
		lineDir.z * upVec.x - lineDir.x * upVec.z,
		lineDir.x * upVec.y - lineDir.y * upVec.z
	};

	// First draw the line
	Line(startPos, endPos, r, g, b, noDepthTest, duration);

	// Now draw the ticks
	for (int i = 0; i < numTicks + 1; i++)
	{
		// Draw tick mark
		Vector tickLeft = tickPos - sideDir;
		Vector tickRight = tickPos + sideDir;

		// Draw tick mark text
		if (tickTextCnt == tickTextDist)
		{
			char text[25];
			snprintf(text, sizeof(text), "%i", i);
			Vector textPos = tickLeft + Vector(0, 0, 8);
			Line(tickLeft, tickRight, 255, 255, 255, noDepthTest, duration);
			Text(textPos, text, 0.f);
			tickTextCnt = 0;
		}
		else
		{
			Line(tickLeft, tickRight, r, g, b, noDepthTest, duration);
		}

		tickTextCnt++;

		tickPos = tickPos + (lineDir * tickDist);
	}
}

//------------------------------------------------------------------------------
// Purpose : Draw crosshair on ground where player is looking
//------------------------------------------------------------------------------
void NDebugOverlay::DrawGroundCrossHairOverlay(void)
{
	ITFPlayer player(true);
	if (!player)
		return;

	// Trace a line to where player is looking
	Vector vForward;
	Vector vSource = player->EyePosition();
	player->EyeVectors(&vForward);

	GameTrace tr;
	Trace::FilterSimple filter(player);

	Trace::TraceLine(vSource, vSource + vForward * 2048, MASK_SOLID, &tr, &filter);
	float dotPr = static_cast<float>(tr.Plane.Normal.Dot({ 0, 0, 1 }));

	if (tr.Fraction != 1.0 && dotPr > 0.5)
	{
		tr.EndPos.z += 1;
		float	scale = 6.f;
		Vector	startPos = tr.EndPos + Vector{ -scale, 0, 0};
		Vector	endPos = tr.EndPos + Vector{ scale, 0, 0 };
		Line(startPos, endPos, 255, 0, 0, false, 0);

		startPos = tr.EndPos + Vector{ 0, -scale, 0 };
		endPos = tr.EndPos + Vector{ 0, scale, 0 };
		Line(startPos, endPos, 255, 0, 0, false, 0);
	}
}

//--------------------------------------------------------------------------------
// Purpose : Draw a horizontal arrow pointing in the specified direction
//--------------------------------------------------------------------------------
void NDebugOverlay::HorzArrow(const Vector& startPos, const Vector& endPos, float width, int r, int g, int b, int a, bool noDepthTest, float flDuration)
{
	Vector	lineDir = (endPos - startPos);
	lineDir.NormalizeInPlace();
	Vector  upVec = Vector(0, 0, 1);
	Vector	sideDir;
	float   radius = width / 2.0f;

	VectorHelper::CrossProduct(&lineDir, &upVec, &sideDir);

	Vector p1 = startPos - sideDir * radius;
	Vector p2 = endPos - lineDir * width - sideDir * radius;
	Vector p3 = endPos - lineDir * width - sideDir * width;
	Vector p4 = endPos;
	Vector p5 = endPos - lineDir * width + sideDir * width;
	Vector p6 = endPos - lineDir * width + sideDir * radius;
	Vector p7 = startPos + sideDir * radius;

	// Outline the arrow
	Line(p1, p2, r, g, b, noDepthTest, flDuration);
	Line(p2, p3, r, g, b, noDepthTest, flDuration);
	Line(p3, p4, r, g, b, noDepthTest, flDuration);
	Line(p4, p5, r, g, b, noDepthTest, flDuration);
	Line(p5, p6, r, g, b, noDepthTest, flDuration);
	Line(p6, p7, r, g, b, noDepthTest, flDuration);

	if (a > 0)
	{
		// Fill us in with triangles
		Triangle(p5, p4, p3, r, g, b, a, noDepthTest, flDuration); // Tip
		Triangle(p1, p7, p6, r, g, b, a, noDepthTest, flDuration); // Shaft
		Triangle(p6, p2, p1, r, g, b, a, noDepthTest, flDuration);

		// And backfaces
		Triangle(p3, p4, p5, r, g, b, a, noDepthTest, flDuration); // Tip
		Triangle(p6, p7, p1, r, g, b, a, noDepthTest, flDuration); // Shaft
		Triangle(p1, p2, p6, r, g, b, a, noDepthTest, flDuration);
	}
}

//-----------------------------------------------------------------------------
// Purpose : Draw a horizontal arrow pointing in the specified direction by yaw value
//-----------------------------------------------------------------------------
void NDebugOverlay::YawArrow(const Vector& startPos, float yaw, float length, float width, int r, int g, int b, int a, bool noDepthTest, float flDuration)
{
	Vector forward = VectorHelper::YawToVector(yaw);

	HorzArrow(startPos, startPos + forward * length, width, r, g, b, a, noDepthTest, flDuration);
}

//--------------------------------------------------------------------------------
// Purpose : Draw a vertical arrow at a position
//--------------------------------------------------------------------------------
void NDebugOverlay::VertArrow(const Vector& startPos, const Vector& endPos, float width, int r, int g, int b, int a, bool noDepthTest, float flDuration)
{
	Vector	lineDir = (endPos - startPos);
	lineDir.NormalizeInPlace();
	Vector  upVec;
	Vector	sideDir;
	float   radius = width / 2.0f;

	VectorHelper::VectorVectors(lineDir, sideDir, upVec);

	Vector p1 = startPos - upVec * radius;
	Vector p2 = endPos - lineDir * width - upVec * radius;
	Vector p3 = endPos - lineDir * width - upVec * width;
	Vector p4 = endPos;
	Vector p5 = endPos - lineDir * width + upVec * width;
	Vector p6 = endPos - lineDir * width + upVec * radius;
	Vector p7 = startPos + upVec * radius;

	// Outline the arrow
	Line(p1, p2, r, g, b, noDepthTest, flDuration);
	Line(p2, p3, r, g, b, noDepthTest, flDuration);
	Line(p3, p4, r, g, b, noDepthTest, flDuration);
	Line(p4, p5, r, g, b, noDepthTest, flDuration);
	Line(p5, p6, r, g, b, noDepthTest, flDuration);
	Line(p6, p7, r, g, b, noDepthTest, flDuration);

	if (a > 0)
	{
		// Fill us in with triangles
		Triangle(p5, p4, p3, r, g, b, a, noDepthTest, flDuration); // Tip
		Triangle(p1, p7, p6, r, g, b, a, noDepthTest, flDuration); // Shaft
		Triangle(p6, p2, p1, r, g, b, a, noDepthTest, flDuration);

		// And backfaces
		Triangle(p3, p4, p5, r, g, b, a, noDepthTest, flDuration); // Tip
		Triangle(p6, p7, p1, r, g, b, a, noDepthTest, flDuration); // Shaft
		Triangle(p1, p2, p6, r, g, b, a, noDepthTest, flDuration);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw an axis
//-----------------------------------------------------------------------------
void NDebugOverlay::Axis(const Vector& position, const QAngle& angles, float size, bool noDepthTest, float flDuration)
{
	Vector xvec, yvec, zvec;
	VectorHelper::AngleVectors(angles, &xvec, &yvec, &zvec);

	xvec = position + (xvec * size);
	yvec = position - (yvec * size); // Left is positive
	zvec = position + (zvec * size);

	Line(position, xvec, 255, 0, 0, noDepthTest, flDuration);
	Line(position, yvec, 0, 255, 0, noDepthTest, flDuration);
	Line(position, zvec, 0, 0, 255, noDepthTest, flDuration);
}

//-----------------------------------------------------------------------------
// Purpose: Draw circles to suggest a sphere
//-----------------------------------------------------------------------------
void NDebugOverlay::Sphere(const Vector& center, float radius, int r, int g, int b, bool noDepthTest, float flDuration)
{
	Vector edge, lastEdge;

	float axisSize = radius;
	Line(center + Vector(0, 0, -axisSize), center + Vector(0, 0, axisSize), r, g, b, noDepthTest, flDuration);
	Line(center + Vector(0, -axisSize, 0), center + Vector(0, axisSize, 0), r, g, b, noDepthTest, flDuration);
	Line(center + Vector(-axisSize, 0, 0), center + Vector(axisSize, 0, 0), r, g, b, noDepthTest, flDuration);

	lastEdge = Vector(radius + center.x, center.y, center.z);
	float angle;
	for (angle = 0.0f; angle <= 360.0f; angle += 22.5f)
	{
		edge.x = radius * cosf(angle / 180.0f * float(M_PI)) + center.x;
		edge.y = center.y;
		edge.z = radius * sinf(angle / 180.0f * float(M_PI)) + center.z;

		Line(edge, lastEdge, r, g, b, noDepthTest, flDuration);

		lastEdge = edge;
	}

	lastEdge = Vector(center.x, radius + center.y, center.z);
	for (angle = 0.0f; angle <= 360.0f; angle += 22.5f)
	{
		edge.x = center.x;
		edge.y = radius * cosf(angle / 180.0f * float(M_PI)) + center.y;
		edge.z = radius * sinf(angle / 180.0f * float(M_PI)) + center.z;

		Line(edge, lastEdge, r, g, b, noDepthTest, flDuration);

		lastEdge = edge;
	}

	lastEdge = Vector(center.x, radius + center.y, center.z);
	for (angle = 0.0f; angle <= 360.0f; angle += 22.5f)
	{
		edge.x = radius * cosf(angle / 180.0f * float(M_PI)) + center.x;
		edge.y = radius * sinf(angle / 180.0f * float(M_PI)) + center.y;
		edge.z = center.z;

		Line(edge, lastEdge, r, g, b, noDepthTest, flDuration);

		lastEdge = edge;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw a circle whose center is at a position, facing the camera
//-----------------------------------------------------------------------------
void NDebugOverlay::Circle(const Vector& position, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration)
{
	ITFPlayer player(true);
	if (!player)
		return;

	Vector clientForward;
	player->EyeVectors(&clientForward);

	QAngle vecAngles;
	VectorHelper::VectorAngles(clientForward, vecAngles);

	Circle(position, vecAngles, radius, r, g, b, a, bNoDepthTest, flDuration);
}


//-----------------------------------------------------------------------------
// Purpose: Draw a circle whose center is at a position and is facing a specified direction
//-----------------------------------------------------------------------------
void NDebugOverlay::Circle(const Vector& position, const QAngle& angles, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration)
{
	// Setup our transform matrix
	Matrix3x4 xform;
	VectorHelper::AngleMatrix(angles, position, xform);
	Vector xAxis, yAxis;
	// default draws circle in the y/z plane
	VectorHelper::MatrixGetColumn(xform, 2, xAxis);
	VectorHelper::MatrixGetColumn(xform, 1, yAxis);
	Circle(position, xAxis, yAxis, radius, r, g, b, a, bNoDepthTest, flDuration);
}

void NDebugOverlay::Circle(const Vector& position, const Vector& xAxis, const Vector& yAxis, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration)
{
	constexpr unsigned int nSegments = 16;
	const float flRadStep = (float(M_PI) * 2.0f) / (float)nSegments;

	// Find our first position
	// Retained for triangle fanning
	Vector vecStart = position + xAxis * radius;
	Vector vecLastPosition = vecStart;
	Vector vecPosition = position + (xAxis * radius * 0.923879f) + (yAxis * radius * 0.382683f);

	// Draw out each segment (fanning triangles if we have an alpha amount)
	for (int i = 2; i <= nSegments; i++)
	{
		// Store off our last position
		vecLastPosition = vecPosition;

		// Calculate the new one
		float flSin, flCos;
		VectorHelper::SinCos(flRadStep * i, flSin, flCos);
		vecPosition = position + (xAxis * flCos * radius) + (yAxis * flSin * radius);

		// Draw the line
		Line(vecLastPosition, vecPosition, r, g, b, bNoDepthTest, flDuration);

		Interfaces::DebugOverlay->AddTriangleOverlay(vecStart, vecLastPosition, vecPosition, r, g, b, a, bNoDepthTest, flDuration);
	}
}

void NDebugOverlay::Sphere(const Vector& position, const QAngle& angles, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration)
{
	// Setup our transform matrix
	Matrix3x4 xform;
	VectorHelper::AngleMatrix(angles, position, xform);
	Vector xAxis, yAxis, zAxis;
	// default draws circle in the y/z plane
	VectorHelper::MatrixGetColumn(xform, 0, xAxis);
	VectorHelper::MatrixGetColumn(xform, 1, yAxis);
	VectorHelper::MatrixGetColumn(xform, 2, zAxis);
	Circle(position, xAxis, yAxis, radius, r, g, b, a, bNoDepthTest, flDuration);	// xy plane
	Circle(position, yAxis, zAxis, radius, r, g, b, a, bNoDepthTest, flDuration);	// yz plane
	Circle(position, xAxis, zAxis, radius, r, g, b, a, bNoDepthTest, flDuration);	// xz plane
}