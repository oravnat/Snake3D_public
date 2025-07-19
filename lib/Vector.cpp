//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Vector.hpp"
#include <math.h>
#include <assert.h>

Vector::Vector()
{
	for (int i = 0; i < 3; i++)
		v[i] = 0.0f;
}

Vector::Vector(t_float x, t_float y, t_float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

Vector::Vector(double x, double y, double z)
{
    v[0] = (t_float)x;
    v[1] = (t_float)y;
    v[2] = (t_float)z;
}

Vector::Vector(const double vv[3])
{
	v[0] = (t_float)vv[0];
	v[1] = (t_float)vv[1];
	v[2] = (t_float)vv[2];
}

Vector::Vector(const float vv[3])
{
	v[0] = vv[0];
	v[1] = vv[1];
	v[2] = vv[2];
}

Vector& Vector::operator+=(const Vector& r)
{
	for (int i = 0; i < 3; i++)
		v[i] += r[i];
	return *this;
}


Vector Vector::operator+(const Vector& r) const
{
	Vector result;
	for (int i = 0; i < 3; i++)
		result[i] = v[i] + r[i];

	return result;
}

Vector& Vector::operator-=(const Vector& r)
{
	for (int i = 0; i < 3; i++)
		v[i] -= r[i];
	return *this;
}

Vector Vector::operator-(const Vector& r) const
{
	Vector result;
	for (int i = 0; i < 3; i++)
		result[i] = v[i] - r[i];

	return result;
}

Vector Vector::operator-() const
{
	Vector result;
	for (int i = 0; i < 3; i++)
		result[i] = -v[i];

	return result;
}


Vector Vector::CrossProduct(const Vector& r) const
{
	Vector result;
	result[0] = v[1] * r [2] - v[2] * r[1];
	result[1] = v[2] * r [0] - v[0] * r[2];
	result[2] = v[0] * r [1] - v[1] * r[0];
	return result;
}

t_float Vector::DotProduct(const Vector& r) const
{
	t_float result = 0.0;
	for (int i = 0; i < 3; i++)
		result += v[i] * r[i];
	return result;
}


t_float& Vector::operator[](int i)
{
	return v[i];
}

t_float Vector::operator[](int i) const
{
	return v[i];
}

t_float Vector::Length2() const
{
	t_float result = 0.0;
	for (int i = 0; i < 3; i++)
		result += v[i] * v[i];
	return result;
}

t_float Vector::Length() const
{
	t_float result = 0.0;
	for (int i = 0; i < 3; i++)
		result += v[i] * v[i];
	return (t_float)sqrt(result);
}

void Vector::Normalize()
{
	t_float len = Length();
	if (len != 0.0f)
	{
		for (int i = 0; i < 3; i++)
			v[i] /= len;
	}
}

Vector& Vector::operator*=(t_float r)
{
	for (int i = 0; i < 3; i++)
		v[i] *= r;
	return *this;
}

Vector operator*(t_float l, const Vector& r)
{
	Vector result;
	for (int i = 0; i < 3; i++)
		result[i] = l * r[i];

	return result;
}

Vector operator*(double l, const Vector& r)
{
	Vector result;
	for (int i = 0; i < 3; i++)
		result[i] = (t_float)(l * r[i]);

	return result;
}



Vec3d::Vec3d()
{
	for (int i = 0; i < 3; i++)
		v[i] = 0.0;
}

Vec3d::Vec3d(double x, double y, double z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

Vec3d::Vec3d(const double vv[3])
{
	v[0] = vv[0];
	v[1] = vv[1];
	v[2] = vv[2];
}

Vec3d::Vec3d(const float vv[3])
{
	v[0] = vv[0];
	v[1] = vv[1];
	v[2] = vv[2];
}

Vec3d& Vec3d::operator+=(const Vec3d& r)
{
	for (int i = 0; i < 3; i++)
		v[i] += r[i];
	return *this;
}


Vec3d Vec3d::operator+(const Vec3d& r) const
{
	Vec3d result;
	for (int i = 0; i < 3; i++)
		result[i] = v[i] + r[i];

	return result;
}

Vec3d& Vec3d::operator-=(const Vec3d& r)
{
	for (int i = 0; i < 3; i++)
		v[i] -= r[i];
	return *this;
}

Vec3d Vec3d::operator-(const Vec3d& r) const
{
	Vec3d result;
	for (int i = 0; i < 3; i++)
		result[i] = v[i] - r[i];

	return result;
}

Vec3d Vec3d::operator-() const
{
	Vec3d result;
	for (int i = 0; i < 3; i++)
		result[i] = -v[i];

	return result;
}


Vec3d Vec3d::CrossProduct(const Vec3d& r) const
{
	Vec3d result;
	result[0] = v[1] * r [2] - v[2] * r[1];
	result[1] = v[2] * r [0] - v[0] * r[2];
	result[2] = v[0] * r [1] - v[1] * r[0];
	return result;
}

double Vec3d::DotProduct(const Vec3d& r) const
{
	double result = 0.0;
	for (int i = 0; i < 3; i++)
		result += v[i] * r[i];
	return result;
}


double& Vec3d::operator[](int i)
{
	return v[i];
}

double Vec3d::operator[](int i) const
{
	return v[i];
}

double Vec3d::Length2() const
{
	double result = 0.0;
	for (int i = 0; i < 3; i++)
		result += v[i] * v[i];
	return result;
}

double Vec3d::Length() const
{
	double result = 0.0;
	for (int i = 0; i < 3; i++)
		result += v[i] * v[i];
	return sqrt(result);
}

void Vec3d::Normalize()
{
	double len = Length();
	if (len != 0.0f)
	{
		for (int i = 0; i < 3; i++)
			v[i] /= len;
	}
}

Vec3d& Vec3d::operator*=(double r)
{
	for (int i = 0; i < 3; i++)
		v[i] *= r;
	return *this;
}


Vec3d operator*(double l, const Vec3d& r)
{
	Vec3d result;
	for (int i = 0; i < 3; i++)
		result[i] = l * r[i];

	return result;
}
