#include "Matrix3x4.hpp"
#include "BasePlayer.hpp"

const Vector NULL_VECTOR{ };

namespace VectorHelper
{
	inline void MatrixTranspose(const Matrix3x4& src, Matrix3x4& dst)
	{
		if (&src == &dst)
		{
			auto Swap = [](float& a, float& b)
			{
				float tmp = a;
				a = b;
				b = tmp;
			};

			Swap(dst[0][1], dst[1][0]);
			Swap(dst[0][2], dst[2][0]);
			Swap(dst[0][3], dst[3][0]);
			Swap(dst[1][2], dst[2][1]);
			Swap(dst[1][3], dst[3][1]);
			Swap(dst[2][3], dst[3][2]);
		}
		else
		{
			dst[0][0] = src[0][0]; dst[0][1] = src[1][0]; dst[0][2] = src[2][0]; dst[0][3] = src[3][0];
			dst[1][0] = src[0][1]; dst[1][1] = src[1][1]; dst[1][2] = src[2][1]; dst[1][3] = src[3][1];
			dst[2][0] = src[0][2]; dst[2][1] = src[1][2]; dst[2][2] = src[2][2]; dst[2][3] = src[3][2];
			dst[3][0] = src[0][3]; dst[3][1] = src[1][3]; dst[3][2] = src[2][3]; dst[3][3] = src[3][3];
		}
	}

	void AngleMatrix(const QAngle& angles, Matrix3x4& matrix)
	{
		float sr, sp, sy, cr, cp, cy;

		SinCos(DegToRad(angles[0]), sp, cp);
		SinCos(DegToRad(angles[1]), sy, cy);
		SinCos(DegToRad(angles[2]), sr, cr);

		// matrix = (YAW * PITCH) * ROLL
		matrix[0][0] = cp * cy;
		matrix[1][0] = cp * sy;
		matrix[2][0] = -sp;

		float crcy = cr * cy;
		float crsy = cr * sy;
		float srcy = sr * cy;
		float srsy = sr * sy;
		matrix[0][1] = sp * srcy - crsy;
		matrix[1][1] = sp * srsy + crcy;
		matrix[2][1] = sr * cp;

		matrix[0][2] = (sp * crcy + srsy);
		matrix[1][2] = (sp * crsy - srcy);
		matrix[2][2] = cr * cp;

		matrix[0][3] = 0.0f;
		matrix[1][3] = 0.0f;
		matrix[2][3] = 0.0f;
	}

	void MatrixAngles(const Matrix3x4& matrix, float* angles)
	{
		float forward[3];
		float left[3];
		float up[3];

		//
		// Extract the basis vectors from the matrix. Since we only need the Z
		// component of the up vector, we don't get X and Y.
		//
		forward[0] = matrix[0][0];
		forward[1] = matrix[1][0];
		forward[2] = matrix[2][0];
		left[0] = matrix[0][1];
		left[1] = matrix[1][1];
		left[2] = matrix[2][1];
		up[2] = matrix[2][2];

		float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

		// enough here to get angles?
		if (xyDist > 0.001f)
		{
			// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
			angles[1] = RadToDeg(atan2f(forward[1], forward[0]));

			// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
			angles[0] = RadToDeg(atan2f(-forward[2], xyDist));

			// (roll)	z = ATAN( left.z, up.z );
			angles[2] = RadToDeg(atan2f(left[2], up[2]));
		}
		else	// forward is mostly Z, gimbal lock-
		{
			// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
			angles[1] = RadToDeg(atan2f(-left[0], left[1]));

			// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
			angles[0] = RadToDeg(atan2f(-forward[2], xyDist));

			// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
			angles[2] = 0;
		}
	}

	float VecToYaw(const Vector& vec)
	{
		if (vec.y == 0 && vec.x == 0)
			return 0;

		float yaw = atan2(vec.y, vec.x);

		yaw = RadToDeg(yaw);

		if (yaw < 0)
			yaw += 360;

		return yaw;
	}


	float VecToPitch(const Vector& vec)
	{
		if (vec.y == 0 && vec.x == 0)
		{
			if (vec.z < 0)
				return 180.0;
			else
				return -180.0;
		}

		float dist = static_cast<float>(vec.Length2D());
		float pitch = atan2f(-vec.z, dist);

		pitch = RadToDeg(pitch);

		return pitch;
	}

	float VecToYaw(const Matrix3x4& matrix, const Vector& vec)
	{
		Vector tmp = vec;
		tmp.NormalizeInPlace();

		float x = matrix[0][0] * tmp.x + matrix[1][0] * tmp.y + matrix[2][0] * tmp.z;
		float y = matrix[0][1] * tmp.x + matrix[1][1] * tmp.y + matrix[2][1] * tmp.z;

		if (x == 0.0f && y == 0.0f)
			return 0.0f;

		float yaw = atan2(-y, x);

		yaw = RadToDeg(yaw);

		if (yaw < 0)
			yaw += 360;

		return yaw;
	}

	float VecToPitch(const Matrix3x4& matrix, const Vector& vec)
	{
		Vector tmp = vec;
		tmp.NormalizeInPlace();

		float x = matrix[0][0] * tmp.x + matrix[1][0] * tmp.y + matrix[2][0] * tmp.z;
		float z = matrix[0][2] * tmp.x + matrix[1][2] * tmp.y + matrix[2][2] * tmp.z;

		if (x == 0.0f && z == 0.0f)
			return 0.0f;

		float pitch = atan2(z, x);

		pitch = RadToDeg(pitch);

		if (pitch < 0)
			pitch += 360;

		return pitch;
	}

	Vector YawToVector(float yaw)
	{
		float angle = RadToDeg(yaw);
		return { cos(angle), sin(angle), 0.f };
	}

	inline void VectorRotate(const float* in1, const Matrix3x4& in2, float* out)
	{
		out[0] = DotProduct(in1, in2[0]);
		out[1] = DotProduct(in1, in2[1]);
		out[2] = DotProduct(in1, in2[2]);
	}

	inline void VectorRotate(const Vector& in1, const QAngle& in2, Vector& out)
	{
		Matrix3x4 matRotate;
		AngleMatrix(in2, matRotate);
		VectorRotate(in1, matRotate, out);
	}

	float ApproachAngle(float target, float value, float speed)
	{
		target = AngMod(target);
		value = AngMod(value);

		float delta = target - value;

		// Speed is assumed to be positive
		if (speed < 0)
			speed = -speed;

		if (delta < -180)
			delta += 360;
		else if (delta > 180)
			delta -= 360;

		if (delta > speed)
			value += speed;
		else if (delta < -speed)
			value -= speed;
		else
			value = target;

		return value;
	}

	float AngleDiff(float destAngle, float srcAngle)
	{
		float delta;

		delta = fmodf(destAngle - srcAngle, 360.0f);
		if (destAngle > srcAngle)
		{
			if (delta >= 180)
				delta -= 360;
		}
		else
		{
			if (delta <= -180)
				delta += 360;
		}
		return delta;
	}


	float AngleDistance(float next, float cur)
	{
		float delta = next - cur;

		if (delta < -180)
			delta += 360;
		else if (delta > 180)
			delta -= 360;

		return delta;
	}

	QAngle GetAimAngle(const Vector& start, const Vector& end, bool useLocalPunchAng)
	{
		QAngle res;
		const Vector to_target = end - start;
		VectorAngles(to_target, res);

		if (useLocalPunchAng)
			res -= ITFPlayer(true)->PunchAngle;

		ClampAngle(res);
		return res;
	}

	float GetFOV(const Vector& start, const Vector& end, const QAngle& angle)
	{
		Vector vecfwd;
		AngleVectors(angle, &vecfwd);
		vecfwd.NormalizeInPlace();

		Vector delta = end - start;
		delta.NormalizeInPlace();

		const float dot = static_cast<float>(vecfwd.Dot(delta));
		const float pow_len = powf(static_cast<float>(Positive(vecfwd.Length())), 2.f);

		return RadToDeg(acosf(dot / (pow_len)));
	}

	void VectorVectors(const Vector& forward, Vector& right, Vector& up)
	{
		if (!forward[0] && !forward[1])
		{
			// pitch 90 degrees up/down from identity
			right[0] = 0;
			right[1] = -1;
			right[2] = 0;
			up[0] = -forward[2];
			up[1] = 0;
			up[2] = 0;
		}
		else
		{
			Vector tmp{ 0, 0, 1.0f };
			CrossProduct(&forward, &tmp, &right);
			right.NormalizeInPlace();
			CrossProduct(&right, &forward, &up);
			up.NormalizeInPlace();
		}
	}

	void VectorAngles(const Vector& forward, QAngle& angles)
	{
		float tmp, yaw, pitch;

		if (!forward[0] && !forward[1])
		{
			yaw = 0;
			if (forward[2] > 0)
				pitch = 270;
			else
				pitch = 90;
		}
		else
		{
			yaw = (atan2(forward[1], forward[0]) * 180.f / static_cast<float>(M_PI));
			if (yaw < 0)
				yaw += 360;

			tmp = std::sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
			pitch = (atan2(-forward[2], tmp) * 180 / static_cast<float>(M_PI));
			if (pitch < 0)
				pitch += 360;
		}

		angles.Init(pitch, yaw);
	}

	void AngleVectors(const QAngle& angles, Vector* forward, Vector* right, Vector* up)
	{
		float sr, sp, sy, cr, cp, cy;

		SinCos(DegToRad(angles[0]), sp, cp);
		SinCos(DegToRad(angles[1]), sy, cy);
		SinCos(DegToRad(angles[2]), sr, cr);

		if (forward)
		{
			forward->x = cp * cy;
			forward->y = cp * sy;
			forward->z = -sp;
		}

		if (right)
		{
			right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
			right->y = (-1 * sr * sp * sy + -1 * cr * cy);
			right->z = -1 * sr * cp;
		}

		if (up)
		{
			up->x = (cr * sp * cy + -sr * -sy);
			up->y = (cr * sp * sy + -sr * cy);
			up->z = cr * cp;
		}
	}
}