#pragma once

#include "Vector3D.hpp"

struct Matrix3x4
{
	Matrix3x4() = default;
	Matrix3x4(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23) noexcept
	{
		MatVal[0][0] = m00;	MatVal[0][1] = m01; MatVal[0][2] = m02; MatVal[0][3] = m03;
		MatVal[1][0] = m10;	MatVal[1][1] = m11; MatVal[1][2] = m12; MatVal[1][3] = m13;
		MatVal[2][0] = m20;	MatVal[2][1] = m21; MatVal[2][2] = m22; MatVal[2][3] = m23;
	}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	void Init(const Vector& x, const Vector& y, const Vector& z, const Vector& origin) noexcept
	{
		MatVal[0][0] = x.x; MatVal[0][1] = y.x; MatVal[0][2] = z.x; MatVal[0][3] = origin.x;
		MatVal[1][0] = x.y; MatVal[1][1] = y.y; MatVal[1][2] = z.y; MatVal[1][3] = origin.y;
		MatVal[2][0] = x.z; MatVal[2][1] = y.z; MatVal[2][2] = z.z; MatVal[2][3] = origin.z;
	}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	Matrix3x4(const Vector& x, const Vector& y, const Vector& z, const Vector& origin) noexcept
	{
		Init(x, y, z, origin);
	}

	inline void Invalidate() noexcept
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 4; j++)
				MatVal[i][j] = NAN;
		}
	}

	float* operator[](int i) noexcept { return MatVal[i]; }
	const float* operator[](int i) const { return MatVal[i]; }
	float* Base() { reinterpret_cast<float*>(this); }
	const float* Base() const noexcept { reinterpret_cast<const float*>(this); }

	float MatVal[3][4]{ };
};

namespace VectorHelper
{
	void MatrixTranspose(const Matrix3x4& src, Matrix3x4& dst);

	inline void MatrixCopy(const Matrix3x4& src, Matrix3x4& dst)
	{
		if (&src != &dst)
			memcpy(&dst, &src, sizeof(Matrix3x4));
	}

	inline void MatrixSetColumn(const Vector& in, int column, Matrix3x4& out)
	{
		out[0][column] = in.x;
		out[1][column] = in.y;
		out[2][column] = in.z;
	}

	inline void MatrixGetColumn(const Matrix3x4& in, int column, Vector& out)
	{
		out.x = in[0][column];
		out.y = in[1][column];
		out.z = in[2][column];
	}

	inline void PositionMatrix(const Vector& position, Matrix3x4& mat)
	{
		MatrixSetColumn(position, 3, mat);
	}

	inline void MatrixPosition(const Matrix3x4& matrix, Vector& position)
	{
		MatrixGetColumn(matrix, 3, position);
	}

	inline void AngleMatrix(const Vector& angles, const Vector& position, Matrix3x4& matrix)
	{
		AngleMatrix(angles, matrix);
		MatrixSetColumn(position, 3, matrix);
	}

	void MatrixAngles(const Matrix3x4& matrix, float* angles);


	inline void MatrixAngles(const Matrix3x4& matrix, Vector& angles)
	{
		MatrixAngles(matrix, &angles.x);
	}

	inline void MatrixAngles(const Matrix3x4& matrix, QAngle& angles, Vector& position)
	{
		MatrixAngles(matrix, angles);
		MatrixPosition(matrix, position);
	}
}