#pragma once

#include "Vector3D.hpp"
#include "Matrix3x4.hpp"

class VMatrix
{
public:
	using vec_t = float;
	vec_t m[4][4]{ };

	// Construction/destruction:
	VMatrix() = default;
	VMatrix(const Vector& fwd, const Vector& left, const Vector& up)
	{
		Init(fwd.x, left.x, up.x, 0.0f,
			 fwd.y, left.y, up.y, 0.0f,
			 fwd.z, left.z, up.z, 0.0f,
			 0.0f, 0.0f, 0.0f, 1.0f);
	}

	// Initialization
	void Init(
		vec_t m00, vec_t m01, vec_t m02, vec_t m03,
		vec_t m10, vec_t m11, vec_t m12, vec_t m13,
		vec_t m20, vec_t m21, vec_t m22, vec_t m23,
		vec_t m30, vec_t m31, vec_t m32, vec_t m33
		) noexcept
	{
		m[0][0] = m00;
		m[0][1] = m01;
		m[0][2] = m02;
		m[0][3] = m03;

		m[1][0] = m10;
		m[1][1] = m11;
		m[1][2] = m12;
		m[1][3] = m13;

		m[2][0] = m20;
		m[2][1] = m21;
		m[2][2] = m22;
		m[2][3] = m23;

		m[3][0] = m30;
		m[3][1] = m31;
		m[3][2] = m32;
		m[3][3] = m33;
	}

	inline float* operator[](int i) noexcept
	{
		return m[i];
	}

	inline const float* operator[](int i) const noexcept
	{
		return m[i];
	}

	inline float* Base() noexcept { return reinterpret_cast<float*>(this); }
	inline const float* Base() const noexcept { return reinterpret_cast<const float*>(this); }

	// equality
	bool operator==(const VMatrix& v) const noexcept = default;
	bool operator!=(const VMatrix& v) const noexcept
	{
		return *this != v;
	}


public:
	VMatrix(const VMatrix&) = default;
	VMatrix& operator=(const VMatrix&) = default;
	VMatrix(VMatrix&&) = default;
	VMatrix& operator=(VMatrix&&) = default;

public:
	VMatrix operator+(const VMatrix& v) const noexcept
	{
		VMatrix ret;
		for (int i = 0; i < 16; i++)
			ret.Base()[i] = Base()[i] + v.Base()[i];
		return ret;
	}
	VMatrix operator-(const VMatrix& v) const noexcept
	{
		VMatrix ret;

		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				ret.Base()[i] = Base()[i] - v.Base()[i];

		return ret;
	}
	Vector operator*(const Vector& v) const noexcept
	{
		Vector ret;

		ret.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
		ret.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
		ret.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];

		return ret;
	}
};

namespace VectorHelper
{
	inline void MatrixCopy(const VMatrix& src, VMatrix& dst)
	{
		if (&src != &dst)
			memcpy(&dst, &src, sizeof(VMatrix));
	}
}