//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Matrix.hpp"
#include <math.h>

Matrix::Matrix()
{
}

Matrix::~Matrix()
{
}

// angle is in radians
Matrix Matrix::RotateX(t_float angle)
{
	Matrix result = I();
	t_float c = (t_float)cos(angle);
	t_float s = (t_float)sin(angle);
	result.v[1][1] = c;
	result.v[2][1] = -s;
	result.v[1][2] = s;
	result.v[2][2] = c;
	return result;
}

// angle is in radians
// Matrix for rotation of angle radians around the y axis, in direction from the x axis to z axis?:
Matrix Matrix::RotateY(t_float angle)
{
	Matrix result = I();
	t_float c = (t_float)cos(angle);
	t_float s = (t_float)sin(angle);
	result.v[0][0] = c;
	result.v[2][0] = s;
	result.v[0][2] = -s;
	result.v[2][2] = c;
	return result;
}

// angle is in radians
Matrix Matrix::RotateZ(t_float angle)
{
	Matrix result = I();
	t_float c = (t_float)cos(angle);
	t_float s = (t_float)sin(angle);
	result.v[0][0] = c;
	result.v[1][0] = -s;
	result.v[0][1] = s;
	result.v[1][1] = c;
	return result;
}

Matrix Matrix::Translate(t_float x, t_float y, t_float z)
{
	Matrix result = I();
	result.v[3][0] = x;
	result.v[3][1] = y;
	result.v[3][2] = z;
	return result;
}

Matrix Matrix::Scale(t_float x, t_float y, t_float z)
{
	Matrix result = I();
	result.v[0][0] = x;
	result.v[1][1] = y;
	result.v[2][2] = z;
	return result;
}

// Create a matrix that transform the "from" direction to "to" direction.  The transform also do a mirroring.
Matrix Matrix::RotateAndMirror(const Vector& from, const Vector& to)
{
	Vector u = from - to;
	u.Normalize();
	Matrix result = I();
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 3; i++)
			result[j][i] -= 2 * u[i] * u[j];
	}
	return result;
}

// from and to should be normalized vectors
Matrix Matrix::RotateFromTo(const Vector& from, const Vector& to)
{
	Vector v = from.CrossProduct(to);
	t_float rad = acosf(from.DotProduct(to));
	return Rotate(v, rad);
}

// Create a unitary rotation matrix that rotate around v:
Matrix Matrix::Rotate(Vector v, t_float rad)
{
	v.Normalize();
	t_float c = (t_float)cos(rad);
	t_float s = (t_float)sin(rad);
	Matrix result = I();
	result[0][0] = v[0] * v[0] * (1.0f - c) + c;
	result[1][0] = v[0] * v[1] * (1.0f - c) - v[2] * s;
	result[2][0] = v[0] * v[2] * (1.0f - c) + v[1] * s;
	result[0][1] = v[1] * v[0] * (1.0f - c) + v[2] * s;
	result[1][1] = v[1] * v[1] * (1.0f - c) + c;
	result[2][1] = v[1] * v[2] * (1.0f - c) - v[0] * s;
	result[0][2] = v[2] * v[0] * (1.0f - c) - v[1] * s;
	result[1][2] = v[2] * v[1] * (1.0f - c) + v[0] * s;
	result[2][2] = v[2] * v[2] * (1.0f - c) + c;
	return result;
}


// The newUp is only normalized.  The newZ is converted to be orthogonal to newUp.
Matrix Matrix::Convert(const Vector& newZ, const Vector& newUp)
{
	Matrix result = I();
	Vector newX = newUp.CrossProduct(newZ);
	//Vector newY = newZ.CrossProduct(newX);
	Vector newY = newUp;
	Vector Z = newX.CrossProduct(newY);
	newX.Normalize();
	newY.Normalize();
	Z.Normalize();
	for (int i = 0; i < 3; i++)
		result[0][i] = newX[i];
	for (int i = 0; i < 3; i++)
		result[1][i] = newY[i];
	for (int i = 0; i < 3; i++)
		result[2][i] = Z[i];

	return result;
}

// The newUp is only normalized.  The newZ is converted to be orthogonal to newUp.
Matrix Matrix::LookAt(const Vector& newZ, const Vector& newUp)
{
	Matrix result = I();
	Vector newX = newUp.CrossProduct(newZ);
	Vector newY = newZ.CrossProduct(newX);
	Vector Z = newZ;
	newX.Normalize();
	newY.Normalize();
	Z.Normalize();
	for (int i = 0; i < 3; i++)
		result[0][i] = newX[i];
	for (int i = 0; i < 3; i++)
		result[1][i] = newY[i];
	for (int i = 0; i < 3; i++)
		result[2][i] = Z[i];

	return result;
}


/*Matrix Matrix::RotateZToVector(const Vector& to)
{
	Matrix result = I();
	result.v[2][0] = -to[0];
	result.v[2][1] = -to[1];
	result.v[2][2] = -to[2];
	return result;
}*/

Matrix Matrix::I()
{
	Matrix result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			result.v[j][i] = 0.0f;
	}
	for (int i = 0; i < 4; i++)
		result.v[i][i] = 1.0f;
	return result;
}

Matrix::operator const t_float*() const
{
	return &v[0][0];
}

t_float* Matrix::operator[](int index)
{
	return v[index];
}

const t_float* Matrix::operator[](int index) const
{
	return v[index];
}

Matrix Matrix::Transpose() const 
{
	Matrix result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			result[j][i] = v[i][j];
	}
	return result;
}

Vector Matrix::MultVec(const Vector& r) const
{
	Vector result;
	for (int i = 0; i < 3; i++)
	{
		t_float val = 0.0;
		for (int k = 0; k < 3; k++)
			val += v[k][i] * r[k];
		result[i] = val;
	}
	return result;
}


Matrix operator*(const Matrix& l, const Matrix& r)
{
	Matrix result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			t_float val = 0.0;
			for (int k = 0; k < 4; k++)
				val += l[k][i] * r[j][k];
			result[j][i] = val;
		}
	}
	return result;
}

Vector operator*(const Matrix& l, const Vector& r)
{
	Vector result;
	for (int i = 0; i < 3; i++)
	{
		t_float val = 0.0;
		for (int k = 0; k < 3; k++)
			val += l[k][i] * r[k];
		val += l[3][i];
		result[i] = val;
	}
	return result;
}

Matrix operator*(t_float l, const Matrix& r)
{
	Matrix result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			result[i][j] = l * r[i][j];
	}
	return result;
}

Matrix operator+(const Matrix& l, const Matrix& r)
{
	Matrix result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			result[i][j] = l[i][j] + r[i][j];
	}
	return result;
}
