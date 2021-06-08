#pragma once

#define _USE_MATH_DEFINES

#include "Vector2D.hpp"
#include <math.h>
#include <xmmintrin.h>

class Vector
{
public:
	using vec_t = float;
	vec_t x = 0.f, y = 0.f, z = 0.f;

	// Construction/destruction:
	Vector() = default;
	constexpr Vector(vec_t x, vec_t y, vec_t z) : x(x), y(y), z(z) { };

	// Initialization
	void Init(vec_t x = 0.0f, vec_t y = 0.0f, vec_t z = 0.0f)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	bool IsValid() const noexcept
	{
		return !std::isnan(x) && std::isnan(y);
	}

	vec_t operator[](int i) const noexcept
	{
		return Base()[i];
	}
	vec_t& operator[](int i) noexcept
	{
		return Base()[i];
	}

	vec_t* Base() noexcept { return reinterpret_cast<vec_t*>(this); }
	const vec_t* Base() const noexcept { return reinterpret_cast<const vec_t*>(this); }

	Vector2D& AsVector2D() noexcept
	{
		return *reinterpret_cast<Vector2D*>(this);
	}
	const Vector2D& AsVector2D() const noexcept
	{
		return *reinterpret_cast<const Vector2D*>(this);
	}

	auto operator<=>(const Vector&) const = default;

	// arithmetic operations
	Vector& operator+=(const Vector& v) noexcept;
	Vector& operator-=(const Vector& v) noexcept;
	Vector& operator*=(const Vector& v) noexcept;
	Vector& operator*=(float s) noexcept;
	Vector& operator/=(const Vector& v) noexcept;
	Vector& operator/=(float s) noexcept;
	Vector& operator=(const float s) noexcept
	{
		x = y = z = s;
		return *this;
	}

	void Negate() noexcept
	{
		x = -x;
		y = -y;
		z = -z;
	}

	// Get the vector's magnitude.
	double Length() const noexcept
	{
		return std::sqrt(LengthSqr());
	}

	// Get the vector's magnitude squared.
	constexpr double LengthSqr() const noexcept
	{
		return static_cast<double>(x) * x + static_cast<double>(y) * y + static_cast<double>(z) * z;
	}

	// return true if this vector is (0,0,0) within tolerance
	constexpr bool IsZero(float tolerance = 0.01f) const noexcept
	{
		return (x > -tolerance && x < tolerance&&
			y > -tolerance && y < tolerance&&
			z > -tolerance && z < tolerance);
	}

	double NormalizeInPlace() noexcept;

	constexpr bool IsLengthGreaterThan(float val) const noexcept
	{
		return LengthSqr() > val * val;
	}
	constexpr bool IsLengthLessThan(float val) const noexcept
	{
		return !IsLengthGreaterThan(val);
	}

	// check if a vector is within the box defined by two other vectors
	constexpr bool WithinAABox(Vector const& boxmin, Vector const& boxmax) const noexcept
	{
		return (
			(x >= boxmin.x) && (x <= boxmax.x) &&
			(y >= boxmin.y) && (y <= boxmax.y) &&
			(z >= boxmin.z) && (z <= boxmax.z)
			);
	}

	// Get the distance from this vector to the other one.
	double DistTo(const Vector& vOther) const noexcept;

	// Get the distance from this vector to the other one squared.
	// NJS: note, VC wasn't inlining it correctly in several deeply nested inlines due to being an 'out of line' inline.  
	// may be able to tidy this up after switching to VC7
	constexpr double DistToSqr(const Vector& other) const noexcept
	{
		return (*this - other).LengthSqr();
	}

	// Copy
	void CopyToArray(float* rgfl) const noexcept
	{
		rgfl[0] = x;
		rgfl[1] = y;
		rgfl[2] = z;
	}

	// Multiply, add, and assign to this (ie: *this = a + b * scalar). This
	// is about 12% faster than the actual vector equation (because it's done per-component
	// rather than per-vector).
	void MulAdd(const Vector& a, const Vector& b, float scalar) noexcept;

	// Dot product.
	constexpr double Dot(const Vector& v) const noexcept
	{
		return static_cast<double>(x) * v.x + static_cast<double>(y) * v.y + static_cast<double>(z) * v.z;
	}

	double Length2D() const noexcept
	{
		return std::sqrt(Length2DSqr());
	}

	constexpr double Length2DSqr() const noexcept
	{
		return x * x + y * y + z * z;
	}

	// arithmetic operations
	constexpr Vector operator-() const noexcept
	{
		return { -x, -y, -z };
	}

	// Returns a vector with the min or max in X, Y, and Z.
	constexpr Vector Min(const Vector& v) const noexcept
	{
		return { x < v.x ? x : v.x, y < v.y ? y : v.y, z < v.z ? z : v.z };
	}

	constexpr Vector Max(const Vector& v) const noexcept
	{
		return { x > v.x ? x : v.x, y > v.y ? y : v.y, z > v.z ? z : v.z };
	}

public:
	Vector(const Vector&) = default;
	Vector& operator=(const Vector&) = default;
	Vector(Vector&&) = default;
	Vector& operator=(Vector&&) = default;

public:
	constexpr Vector operator+(const Vector& v) const noexcept
	{
		return { x + v.x, y + v.y, z + v.z };
	}
	constexpr Vector operator-(const Vector& v) const noexcept
	{
		return { x - v.x, y - v.y, z - v.z };
	}
	constexpr Vector operator*(const Vector& v) const noexcept
	{
		return { x * v.x, y * v.y, z * v.z };
	}
	constexpr Vector operator/(const Vector& v) const noexcept
	{
		return { x / v.x, y / v.y, z / v.z };
	}
	constexpr Vector operator*(float fl) const noexcept
	{
		return { x * fl, y * fl, z * fl };
	}

	constexpr Vector operator/(float fl) const noexcept
	{
		return { x / fl, y / fl, z / fl };
	}
};

extern const Vector NULL_VECTOR;


__declspec(align(16))
class VectorAligned : public Vector
{
public:
	VectorAligned() = default;
	constexpr VectorAligned(vec_t x, vec_t y, vec_t z) noexcept : Vector(x, y, z) { };

	VectorAligned(const VectorAligned&) = delete;
	VectorAligned& operator=(const VectorAligned&) = default;
	VectorAligned(VectorAligned&&) = default;
	VectorAligned& operator=(VectorAligned&&) = default;

	VectorAligned(Vector&& o) noexcept
	{
		x = o.x; 
		y = o.y;
		z = o.z;
	}

	VectorAligned(const Vector& o) noexcept
	{
		x = o.x; 
		y = o.y;
		z = o.z;
	}

	VectorAligned& operator=(const float s) noexcept
	{
		x = y = z = w = s;
		return *this;
	}

	float w;
};

struct Quaternion
{
	float x;
	float y;
	float z;
	float w;
};

struct Quaternion48
{
	unsigned short x : 16;
	unsigned short y : 16;
	unsigned short z : 15;
	unsigned short wneg : 1;
};

struct Quaternion64
{
	unsigned __int64 x : 21;
	unsigned __int64 y : 21;
	unsigned __int64 z : 21;
	unsigned __int64 wneg : 1;
};

using QAngle = Vector;
using Vector4D = Quaternion;
using RadianEuler = Vector;


inline double Vector::NormalizeInPlace() noexcept
{
	double l = Length();
	if (l)
		this->operator/=(static_cast<vec_t>(l));
	else
		x = y = z = 0.f;
	return l;
}

inline double Vector::DistTo(const Vector& other) const noexcept
{
	return (other - *this).Length();
}

inline void Vector::MulAdd(const Vector& a, const Vector& b, float scalar) noexcept
{
	x = a.x + b.x * scalar;
	y = a.y + b.y * scalar;
	z = a.z + b.z * scalar;
}


inline Vector& Vector::operator+=(const Vector& v) noexcept
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

inline Vector& Vector::operator-=(const Vector& v) noexcept
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

inline Vector& Vector::operator*=(const Vector& v) noexcept
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

inline Vector& Vector::operator*=(float s)  noexcept
{
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

inline Vector& Vector::operator/=(const Vector& v) noexcept
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}

inline Vector& Vector::operator/=(float s)  noexcept
{
	x /= s;
	y /= s;
	z /= s;
	return *this;
}


struct Matrix3x4;

namespace VectorHelper
{
	template<typename FloatingPoint, typename = std::enable_if_t<std::is_floating_point_v<FloatingPoint>>>
	FloatingPoint DegToRad(const FloatingPoint& n)
	{
		return n * static_cast<FloatingPoint>(M_PI / 180.0);
	}

	template<typename FloatingPoint, typename = std::enable_if_t<std::is_floating_point_v<FloatingPoint>>>
	FloatingPoint RadToDeg(const FloatingPoint& n)
	{
		return n * static_cast<FloatingPoint>(180.0 / M_PI);
	}

	inline void VectorRotate(const Vector& in1, const QAngle& in2, Vector& out);
	void VectorRotate(const Vector& in1, const Matrix3x4& in2, Vector& out);
	void AngleMatrix(const QAngle& angles, Matrix3x4& matrix);

	inline float DotProduct(const float* v1, const float* v2)
	{
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}

	inline void VectorSubtract(const float* a, const float* b, float* c)
	{
		c[0] = a[0] - b[0];
		c[1] = a[1] - b[1];
		c[2] = a[2] - b[2];
	}
	inline void VectorAdd(const float* a, const float* b, float* c)
	{
		c[0] = a[0] + b[0];
		c[1] = a[1] + b[1];
		c[2] = a[2] + b[2];
	}
	inline void VectorCopy(const float* a, float* b)
	{
		b[0] = a[0];
		b[1] = a[1];
		b[2] = a[2];
	}

	inline void VectorClear(float* a)
	{
		a[0] = a[1] = a[2] = 0;
	}

	inline float VectorMaximum(const float* v)
	{
		return std::max(v[0], std::max(v[1], v[2]));
	}

	inline float VectorMaximum(const Vector& v)
	{
		return std::max(v.x, std::max(v.y, v.z));
	}

	inline void VectorScale(const float* in, float scale, float* out)
	{
		out[0] = in[0] * scale;
		out[1] = in[1] * scale;
		out[2] = in[2] * scale;
	}

	inline void VectorFill(float* a, float b)
	{
		a[0] = a[1] = a[2] = b;
	}

	inline void VectorNegate(float* a)
	{
		a[0] = -a[0];
		a[1] = -a[1];
		a[2] = -a[2];
	}

	inline void SinCos(const float f, float& sval, float& cval)
	{
		sval = sin(f);
		cval = cos(f);
	}

	inline void CrossProduct(const float* v1, const float* v2, float* cross)
	{
		cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
		cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
		cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
	}

	inline void CrossProduct(const Vector* a, const Vector* b, Vector* out)
	{
		CrossProduct(a->Base(), b->Base(), out->Base());
	}

	inline void VectorRotate(const float* in1, const Matrix3x4& in2, float* out);	

	inline void VectorRotate(const Vector* in1, const Matrix3x4& in2, float* out)
	{
		VectorRotate(reinterpret_cast<const float*>(in1), in2, out);
	}

	inline void VectorRotate(const Vector& in1, const Matrix3x4& in2, Vector& out)
	{
		VectorRotate(&in1, in2, &out.x);
	}

	template<typename Type>
	inline Type Positive(const Type& a)
	{
		return a < 0 ? -a : a;
	}

	void VectorVectors(const Vector& fwd, Vector& origin, Vector& up);
	void VectorAngles(const Vector& forward, QAngle& angles);
	void AngleVectors(const QAngle& angles, Vector* forward, Vector* right = nullptr, Vector* up = nullptr);

	float VecToYaw(const Vector& vec);
	float VecToPitch(const Vector& vec);
	float VecToYaw(const Matrix3x4& matrix, const Vector& vec);
	float VecToPitch(const Matrix3x4& matrix, const Vector& vec);
	Vector YawToVector(float yaw);

	inline float AngMod(float a)
	{
		return (360.f / 65536) * ((int)(a * (65536.f / 360.0f)) & 65535);
	}

	float ApproachAngle(float target, float value, float speed);
	float AngleDiff(float destAngle, float srcAngle);
	float AngleDistance(float next, float cur);

	QAngle GetAimAngle(const Vector& start, const Vector& end, bool useLocalPunchAng = false);
	float GetFOV(const Vector& start, const Vector& end, const QAngle& angle);

	inline float AngleNormalize(float angle)
	{
		angle = fmodf(angle, 360.0f);
		if (angle > 180)
			angle -= 360;
		else if (angle < -180)
			angle += 360;
		return angle;
	}

	inline void ClampAngle(QAngle& ang)
	{
		ang.x = AngleNormalize(ang.x);
		ang.y = AngleNormalize(ang.y);
		ang.z = 0;
	}

	inline void SmoothAngle(const QAngle& input, QAngle& output, const float rate)
	{
		const QAngle delta = (input - output) * rate;

		output.x = VectorHelper::ApproachAngle(output.x, input.x, delta.x);
		output.y = VectorHelper::ApproachAngle(output.y, input.y, delta.y);

		ClampAngle(output);
	}
}