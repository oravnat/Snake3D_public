//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Sphere.hpp"

extern Model g_sphereModel;

Sphere::Sphere(double radius)
{
	m_radius = radius;
	m_color = BColor(255, 128, 128);
}

Sphere::~Sphere(void)
{
}

bool Sphere::Think(SnakeGame& game)
{
	return false;
}

void Sphere::Render(SnakeRenderer& renderer)
{
	if (m_radius <= 0)
		return;
	renderer.Color(m_color.c[0], m_color.c[1], m_color.c[2], m_color.c[3]);
	renderer.Scale((t_float)m_radius, (t_float)m_radius, (t_float)m_radius);
	g_sphereModel.Render(renderer);
}

void Sphere::SetColor(const BColor& color)
{
	m_color = color;
}
