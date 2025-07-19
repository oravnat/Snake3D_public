//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Vector.hpp"

class Matrix
{
	// v[col][row]:
	t_float v[4][4];
public:
	Matrix();
	Matrix(const float m[16])
	{
		for (int i = 0, k = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				v[i][j] = m[k++];
			}

		}
	}
	Matrix(const double m[16])
	{
		for (int i = 0, k = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				v[i][j] = (t_float)m[k++];
			}

		}
	}
	~Matrix();
public:
	operator const t_float*() const;
	t_float* operator[](int index);
	const t_float* operator[](int index) const;
	Matrix Transpose() const;
public:
	static Matrix RotateX(t_float angle);
	static Matrix RotateY(t_float angle);
	static Matrix RotateZ(t_float angle);
	// Create a unitary rotation matrix that rotate the unit vector from to the unit vector to:
	static Matrix RotateAndMirror(const Vector& from, const Vector& to);
	static Matrix RotateFromTo(const Vector& from, const Vector& to);
		// Create a unitary rotation matrix that rotate around v:
	static Matrix Rotate(Vector v, t_float rad);
	// newZ and newUp must be unit vectors:
	static Matrix Convert(const Vector& newZ, const Vector& newUp);
	static Matrix LookAt(const Vector& newZ, const Vector& newUp);
	static Matrix Translate(t_float x, t_float y, t_float z);
	static Matrix Scale(t_float x, t_float y, t_float z);
	// Returns a matrix with rotate the negative z to point to the specific vector.
	// The up remains the same.
	//static Matrix RotateZToVector(const Vector& to);
	static Matrix I();
};

Matrix operator*(const Matrix& l, const Matrix& r);
Vector operator*(const Matrix& l, const Vector& r);
Matrix operator*(t_float l, const Matrix& r);
Matrix operator+(const Matrix& l, const Matrix& r);

inline Vector TakeTranslation(const Matrix& m)
{
	return Vector(m[3][0], m[3][1], m[3][2]);
}
inline Matrix ZeroTranslation(const Matrix& m)
{
	Matrix result = m;
	for (int i = 0; i < 3; i++)
	{
		result[3][i] = 0.0;
	}
	return result;
}
// invert matrix that have only rotation and translation parts
// M = T * R
// => M^(-1) = R^(-1) * T^(-1) = R^T * T^(-1)
inline Matrix InvertPoseMatrix(const Matrix& m)
{
	Vector t = TakeTranslation(m);
	Matrix invT = Matrix::Translate(-t[0], -t[1], -t[2]);
	Matrix R = ZeroTranslation(m);
	return R.Transpose() * invT;

}
