//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

//typedef double t_float;
//#define SN_FLOAT GL_DOUBLE
typedef float t_float;
//typedef GLfloat t_float;
#define SN_FLOAT GL_FLOAT

struct Color
{
	Color() { c[0] = c[1] = c[2] = 0.0f; c[3] = 1.0f; }
	Color(t_float _r, t_float _g, t_float _b) { c[0] = _r; c[1] = _g; c[2] = _b; c[3] = 1.0f; }
	void ToFloat(float* p) const {p[0] = (float)c[0]; p[1] = (float)c[1]; p[2] = (float)c[2];}
	t_float c[4];
};

struct BColor
{
	unsigned char c[4];
	BColor()
	{
		c[0] = 0;
		c[1] = 0;
		c[2] = 0;
		c[3] = 255;
	}
	BColor(unsigned char r, unsigned char g, unsigned char b)
	{
		c[0] = r;
		c[1] = g;
		c[2] = b;
		c[3] = 255;
	}
	operator Color()
	{
		Color result;
		result.c[0] = c[0] / 255.0f;
		result.c[1] = c[1] / 255.0f;
		result.c[2] = c[2] / 255.0f;
		result.c[3] = c[3] / 255.0f;

		return result;
	}
};


struct Vector
{
	Vector();
	Vector(t_float x, t_float y, t_float z);
	Vector(double x, double y, double z);
	Vector(const double v[3]);
	Vector(const float v[3]);

	t_float v[3];
	// Perform element wise addition:
	Vector& operator+=(const Vector& r);
	Vector operator+(const Vector& r) const;
	// Perform element wise subtraction:
	Vector& operator-=(const Vector& r);
	Vector operator-(const Vector& r) const;
	// unary minus:
	Vector operator-() const;
	// Perform Cross Product:
	Vector CrossProduct(const Vector& r) const;
	// Perform Dot Product:
	t_float DotProduct(const Vector& r) const;
	t_float Length2() const;
	t_float Length() const;
	void Normalize();
	t_float& operator[](int i);
	t_float operator[](int i) const;
	Vector& operator*=(t_float r);
};

Vector operator*(t_float l, const Vector& r);
Vector operator*(double l, const Vector& r);

struct Vec3d
{
	Vec3d();
	Vec3d(double x, double y, double z);
	Vec3d(const double v[3]);
	Vec3d(const float v[3]);
	double v[3];
	// Perform element wise addition:
	Vec3d& operator+=(const Vec3d& r);
	Vec3d operator+(const Vec3d& r) const;
	// Perform element wise subtraction:
	Vec3d& operator-=(const Vec3d& r);
	Vec3d operator-(const Vec3d& r) const;
	// unary minus:
	Vec3d operator-() const;
	// Perform Cross Product:
	Vec3d CrossProduct(const Vec3d& r) const;
	// Perform Dot Product:
	double DotProduct(const Vec3d& r) const;
	double Length2() const;
	double Length() const;
	void Normalize();
	double& operator[](int i);
	double operator[](int i) const;
	Vec3d& operator*=(double r);
};

Vec3d operator*(double l, const Vec3d& r);

struct Vertex3f3f
{
	Vector pos;
	Vector normal;
};

struct Vertex3f3f2f
{
	Vector pos;
	Vector normal;
	t_float u, v;
};

struct Vertex3f2f
{
	Vector pos;
	t_float u, v;
};

struct VertexColor2f
{
	unsigned char r, g, b, alpha;
	float x, y;
};
