#pragma once

#include <cmath>

class Vector2D
{
public:
	using vec_t = float;
	vec_t x = 0.f, y = 0.f;

	// Construction/destruction
	Vector2D() = default;
	Vector2D(vec_t x, vec_t y) : x(x), y(y) { };
	Vector2D(const float* pFloat) : x(pFloat[0]), y(pFloat[1]) { };

	void Init(vec_t x = 0.0f, vec_t y = 0.0f)
	{
		this->x = x;
		this->y = y;
	}

	// Got any nasty NAN's?
	bool IsValid() const noexcept;

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

	// equality
	bool operator==(const Vector2D& v) const noexcept = default;
	bool operator!=(const Vector2D& v) const noexcept = default;

	// arithmetic operations
	Vector2D& operator+=(const Vector2D& v) noexcept;
	Vector2D& operator-=(const Vector2D& v) noexcept;
	Vector2D& operator*=(const Vector2D& v) noexcept;
	Vector2D& operator*=(float s)  noexcept;
	Vector2D& operator/=(const Vector2D& v) noexcept;
	Vector2D& operator/=(float s)  noexcept;

	void Negate() noexcept;

	// Get the Vector2D's magnitude.
	vec_t Length() const noexcept;

	// Get the Vector2D's magnitude squared.
	vec_t LengthSqr() const noexcept;

	// return true if this vector is (0,0) within tolerance
	bool IsZero(float tolerance = 0.01f) const noexcept
	{
		return (x > -tolerance && x < tolerance&&
			y > -tolerance && y < tolerance);
	}

	// Normalize in place and return the old length.
	vec_t NormalizeInPlace() noexcept;

	// Compare length.
	bool IsLengthGreaterThan(float val) const noexcept;
	bool IsLengthLessThan(float val) const noexcept;

	// Get the distance from this Vector2D to the other one.
	vec_t	DistTo(const Vector2D& vOther) const noexcept;
	// Get the distance from this Vector2D to the other one squared.
	vec_t	DistToSqr(const Vector2D& vOther) const noexcept;

	void CopyToArray(float* rgfl) const noexcept
	{
		rgfl[0] = x;
		rgfl[1] = y;
	}

	// Multiply, add, and assign to this (ie: *this = a + b * scalar). This
	// is about 12% faster than the actual Vector2D equation (because it's done per-component
	// rather than per-Vector2D).
	void MulAdd(const Vector2D& a, const Vector2D& b, float scalar) noexcept;
	
	// Dot product.
	vec_t Dot(const Vector2D& v) const noexcept
	{
		return x * v.x + y * v.y;
	}

	// arithmetic operations
	Vector2D operator-() const noexcept
	{
		return { -x, -y };
	}

	// Returns a Vector2D with the min or max in X, Y, and Z.
	Vector2D Min(const Vector2D& v) const noexcept
	{
		return { x < v.x ? x : v.x,  y < v.y ? y : v.y };
	}

	Vector2D Max(const Vector2D& v) const noexcept
	{
		return { x > v.x ? x : v.x,  y > v.y ? y : v.y };
	}

public:
	Vector2D(const Vector2D&) = default;
	Vector2D& operator=(const Vector2D&) = default;
	Vector2D(Vector2D&&) = default;
	Vector2D& operator=(Vector2D&&) = default;

public:
	Vector2D operator+(const Vector2D& v) const noexcept
	{
		return { x + v.x, y + v.y };
	}
	Vector2D operator-(const Vector2D& v) const noexcept
	{
		return { x - v.x, y - v.y };
	}
	Vector2D operator*(const Vector2D& v) const noexcept
	{
		return { x * v.x, y * v.y };
	}
	Vector2D operator/(const Vector2D& v) const noexcept
	{
		return { x / v.x, y / v.y };
	}
	Vector2D operator*(float fl) const noexcept
	{
		return { x / fl, y / fl };
	}
	Vector2D operator/(float fl) const noexcept
	{
		return { x / fl, y / fl };
	}
};

inline bool Vector2D::IsValid() const noexcept
{
	return !std::isnan(x) && std::isnan(y);
}

inline void Vector2D::Negate() noexcept
{
	x = -x;
	y = -y;
}

inline float Vector2D::LengthSqr() const noexcept
{
	return x * x + y * y;
}

inline float Vector2D::Length() const noexcept
{
	return std::sqrtf(LengthSqr());
}

inline float Vector2D::NormalizeInPlace() noexcept
{
	vec_t l = Length();
	if (l)
		this->operator/=(l);
	else
		x = y = 0.f;
	return l;
}

inline bool Vector2D::IsLengthGreaterThan(float val) const noexcept
{
	return LengthSqr() > val * val;
}

inline bool Vector2D::IsLengthLessThan(float val) const noexcept
{
	return !IsLengthGreaterThan(val);
}

inline float Vector2D::DistTo(const Vector2D& other) const noexcept
{
	return (other - *this).Length();
}

inline float Vector2D::DistToSqr(const Vector2D& other) const noexcept
{
	return (other - *this).LengthSqr();
}

inline void Vector2D::MulAdd(const Vector2D& a, const Vector2D& b, float scalar) noexcept
{
	x = a.x + b.x * scalar;
	y = a.y + b.y * scalar;
}


inline Vector2D& Vector2D::operator+=(const Vector2D& v) noexcept
{
	x += v.x;
	y += v.y;
}
inline Vector2D& Vector2D::operator-=(const Vector2D& v) noexcept
{
	x -= v.x;
	y -= v.y;
}
inline Vector2D& Vector2D::operator*=(const Vector2D& v) noexcept
{
	x *= v.x;
	y *= v.y;
}
inline Vector2D& Vector2D::operator*=(float s)  noexcept
{
	x *= s;
	y *= s;
}
inline Vector2D& Vector2D::operator/=(const Vector2D& v) noexcept
{
	x /= v.x;
	y /= v.y;
}
inline Vector2D& Vector2D::operator/=(float s)  noexcept
{
	x /= s;
	y /= s;
}