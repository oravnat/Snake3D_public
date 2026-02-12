//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Wall.hpp"
#include "SnakeGame.hpp"
#include "SnakeRenderer.hpp"

Wall::Wall(void) : m_pCircleNormals(NULL)
{
	m_type = EntityType_Wall;
	m_wallType = WallType_Circle;
	m_radius = 13;
	m_innerRadius = m_radius - 1.0;
	m_height = 3.3;
	m_bPassThrough = true;
	m_bShrinking = true;

	m_nLineVertices = 0;
	m_pLineVertices[0] = NULL;
	m_pLineVertices[1] = NULL;
	m_pLineVertices[2] = NULL;
	m_nCircleVertices = 20;

	m_pCircleVertices[0] = NULL;
	m_pCircleVertices[1] = NULL;
	m_pCircleVertices[2] = NULL;
}

Wall::Wall(Vector p1, Vector p2) : m_pCircleNormals(NULL)
{
	m_type = EntityType_Wall;
	m_p1 = p1;
	m_p2 = p2;
	m_pos = 0.5 * (m_p1 + m_p2);
	m_wallType = WallType_Line;
	m_height = 3.3;
	m_bPassThrough = false;
	m_nLineVertices = 20;
	m_pLineVertices[0] = NULL;
	m_pLineVertices[1] = NULL;
	m_pLineVertices[2] = NULL;
	m_nCircleVertices = 0;
	m_width = 1.0;
	m_bShrinking = false;
	m_pCircleVertices[0] = NULL;
	m_pCircleVertices[1] = NULL;
	m_pCircleVertices[2] = NULL;
	CreateLine();
}

Wall::~Wall(void)
{
	delete m_pLineVertices[0];
	delete m_pLineVertices[1];
	delete m_pLineVertices[2];
}

bool Wall::Think(SnakeGame& game)
{
	double time = game.m_timeDiff;
	if (m_bShrinking)
		m_height -= time;
	if (m_height < 0.0)
	{
		//m_height = 10.0;
		return true;
	}

	return false;
}

void Wall::Render(SnakeRenderer& renderer)
{
	renderer.Color(255, 255, 0, 255);
	
	switch (m_wallType)
	{
	case WallType_Circle:
		RenderCircle(renderer);
		break;
	case WallType_Line:
		RenderLine(renderer);
		break;
	}
}

void Wall::Draw2D(SnakeRenderer& renderer)
{
	const int MAX_VECTORS = 50;
	Vector a[MAX_VECTORS];
	renderer.Color(255, 255, 0, 255);
	switch (m_wallType)
	{
	case WallType_Circle:
		for (int i = 0; i < MAX_VECTORS; i++)
		{
			float angle = (float)(i * 2 * M_PI / MAX_VECTORS);
			a[i][0] = (t_float)(m_pos[0] + m_radius * cos(angle));
			a[i][1] = 0.0f;
			a[i][2] = (t_float)(m_pos[2] + m_radius * sin(angle));
		}
		renderer.DrawVectors(a, MAX_VECTORS, SNAKE_LINE_LOOP);
		break;
	case WallType_Line:
		a[0] = m_p1;
		a[1] = m_p2;
			renderer.DrawVectors(a, 2, SNAKE_LINES);
		break;
	}
}

void Wall::RenderLine(SnakeRenderer& renderer)
{
	Vector diff = m_p2 - m_p1;
	Vector up(0.0, 1.0, 0.0);
	Matrix rot = Matrix::LookAt(diff, up);
	renderer.MultMatrix(rot);
	renderer.Scale((t_float)m_width, (t_float)m_width, diff.Length() / 2.0f);

	// Render the right side:
	renderer.DrawVertex3f3f(m_pLineVertices[0], m_nLineVertices * 2, SNAKE_TRIANGLE_STRIP);

	// Render the left side:
	renderer.DrawVertex3f3f(m_pLineVertices[1], m_nLineVertices * 2, SNAKE_TRIANGLE_STRIP);

	// Render the up side:
	renderer.DrawVertex3f3f(m_pLineVertices[2], m_nLineVertices * 2, SNAKE_TRIANGLE_STRIP);
}

void Wall::CreateLine()
{
	float diff = -2.0f / (m_nLineVertices - 1);
	int index;
	float z;
	// Create right side:
	z = 1.0;
	index = 0;
	delete m_pLineVertices[0];
	m_pLineVertices[0] = new Vertex3f3f[m_nLineVertices * 2];
	for (int i = 0; i < m_nLineVertices; i++)
	{
		m_pLineVertices[0][index].pos = Vector(1.0f, 1.0f, z);
		m_pLineVertices[0][index].normal = Vector(1.0f, 0.0f, 0.0f);
		index++;
		m_pLineVertices[0][index].pos = Vector(1.0f, -1.0f, z);
		m_pLineVertices[0][index].normal = Vector(1.0f, 0.0f, 0.0f);
		index++;
		z += diff;
	}
	// Create left side:
	z = 1.0;
	index = 0;
	delete m_pLineVertices[1];
	m_pLineVertices[1] = new Vertex3f3f[m_nLineVertices * 2];
	for (int i = 0; i < m_nLineVertices; i++)
	{
		m_pLineVertices[1][index].pos = Vector(-1.0f, -1.0f, z);
		m_pLineVertices[1][index].normal = Vector(-1.0f, 0.0f, 0.0f);
		index++;
		m_pLineVertices[1][index].pos = Vector(-1.0f, 1.0f, z);
		m_pLineVertices[1][index].normal = Vector(-1.0f, 0.0f, 0.0f);
		index++;
		z += diff;
	}
	// Create up side:
	z = 1.0;
	index = 0;
	delete m_pLineVertices[2];
	m_pLineVertices[2] = new Vertex3f3f[m_nLineVertices * 2];
	for (int i = 0; i < m_nLineVertices; i++)
	{
		m_pLineVertices[2][index].pos = Vector(-1.0f, 1.0f, z);
		m_pLineVertices[2][index].normal = Vector(0.0f, 1.0f, 0.0f);
		index++;
		m_pLineVertices[2][index].pos = Vector(1.0f, 1.0f, z);
		m_pLineVertices[2][index].normal = Vector(0.0f, 1.0f, 0.0f);
		index++;
		z += diff;
	}
}

void Wall::RenderCircle(SnakeRenderer& renderer)
{
}

void Wall::CreateCircle()
{
		/*
		// The outer side:
		glBegin(GL_TRIANGLE_STRIP);
		for (double angle = 0; angle < 2 * M_PI; angle += 0.1)
		{
			glNormal3d(cos(angle), 0.0, sin(angle));
			glVertex3d(m_pos[0] + m_radius * cos(angle), 0.0, m_pos[2] + m_radius * sin(angle));
			glVertex3d(m_pos[0] + m_radius * cos(angle), m_height, m_pos[2] + m_radius * sin(angle));
		}
		glEnd();

		// The inner side:
		glBegin(GL_TRIANGLE_STRIP);
		for (double angle = 2 * M_PI; angle >= 0.0 ; angle -= 0.1)
		{
			glNormal3d(-cos(angle), 0.0, -sin(angle));
			glVertex3d(m_pos[0] + m_innerRadius * cos(angle), 0.0, m_pos[2] + m_innerRadius * sin(angle));
			glVertex3d(m_pos[0] + m_innerRadius * cos(angle), m_height, m_pos[2] + m_innerRadius * sin(angle));
		}
		glEnd();

		// The roof:
		glBegin(GL_TRIANGLE_STRIP);
		for (double angle = 0; angle < 2 * M_PI; angle += 0.1)
		{
			glNormal3d(0.0, 1.0, 0.0);
			glVertex3d(m_pos[0] + m_radius * cos(angle), m_height, m_pos[2] + m_radius * sin(angle));
			glVertex3d(m_pos[0] + m_innerRadius * cos(angle), m_height, m_pos[2] + m_innerRadius * sin(angle));
		}
		glEnd();*/
}
