//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Level.hpp"
#include "SnakeGame.hpp"
#include "Sphere.hpp"
#include "Door.hpp"
#include "Wall.hpp"
//#include "Model.h"
#include <math.h>

const float RAMP_WIDTH = 10;

extern Model g_sphereModel;

struct Ramp
{
	Vector v1, v2;
};

Ramp ramps[1] = {Vector(100.0f, 0.0f, 100.0f), Vector(100.0f, 50.0f, 50.0f)};
//int nRamps = 1;
int nRamps = 0;

Level::Level(void) : m_iLevel(1), m_bLevelCompleted(false), m_pGame(NULL)
{
	m_sphereRadius = SPHERE_RADIUS;
	m_g = -10.0 * GRAV;
	m_gridMaxX = 100.0; // meters
	m_gridMaxZ = 100.0; // meters
	m_bSunMove = false;
	m_sunCirclesPerSecond = 60.0f;
	m_lampColor = BColor(230, 204, 102);
	m_type = LevelType_Simple;
	m_iCurrentBackgound = 0;
}

Level::~Level(void)
{
}

bool Level::Think(SnakeGame& game)
{
	m_bNight = false;
	switch (m_type)
	{
	case LevelType_Simple:
		if (m_bSunMove && m_pGame)
		{
			int time = m_pGame->m_timestamp;
			float angle = (float)(time / 1000.0 * M_PI * 2 / m_sunCirclesPerSecond);
			float height = 300.0f;
			float c = cosf(angle);
			if (c <= 0.0f)
				m_bNight = true;
			if (m_bNight)
				m_lightPos = Vector(m_gridMaxX / 2.0f, 20.0f, m_gridMaxZ / 2.0f);
			else
				m_lightPos = Vector(m_gridMaxX / 2.0f, height * cosf(angle), m_gridMaxZ / 2.0f + height * sinf(angle));
		}
		else
			m_lightPos = Vector(m_gridMaxX / 2.0f, 100.0f, 0.0f);
		break;
	case LevelType_Customize:
	case LevelType_ShowModel:
		m_lightPos = Vector(0.0f, 0.0f, 30.0f);
		break;
	case LevelType_Shooter:
		m_lightPos = Vector(m_gridMaxX / 2.0f, 200.0f, 30.0f);
		break;
	default: // Assume ball map:
		m_lightPos = Vector(0.0f, 2.0f * (t_float)m_sphereRadius, 0.0f);
	}

	for (int i = 0; i < m_shieldVisibleUntil.size(); i++)
	{
		if (m_shieldVisibleUntil[i] < game.m_timestamp)
			m_shieldVisibleUntil[i] = 0;
	}

	return false;
}

void Level::Render(SnakeRenderer& renderer)
{
	const int N = 100;
	Vertex3f3f a[N];
	if (m_type == LevelType_Simple)
	{
		renderer.Color(204, 128, 77, 255);
		for (int iRamp = 0; iRamp < nRamps; iRamp++)
		{
			Vector v1 = ramps[iRamp].v1;
			Vector v2 = ramps[iRamp].v2;
			Vector xdiff = (v2 - v1).CrossProduct(Vector(0.0f, 1.0f, 0.0f));
			xdiff.Normalize();
			// The up side:
			for (int i = 0; i < N; i += 2)
			{
				float b = (float)i / (N - 2);
				Vector v = (1.0f - b) * v1 + b * v2;
				a[i].pos = v - RAMP_WIDTH * xdiff;
				a[i].normal = Vector(0, cosf(45.0), sinf(45.0));
				a[i+1].pos = v + RAMP_WIDTH * xdiff;
				a[i+1].normal = Vector(0, cosf(45.0), sinf(45.0));
			}
            renderer.DrawVertex3f3f(a, N, SNAKE_TRIANGLE_STRIP);
			// The right side:
			for (int i = 0; i < N; i += 2)
			{
				float b = (float)i / (N - 2);
				Vector v = (1.0f - b) * v1 + b * v2 + RAMP_WIDTH * xdiff;
				a[i].pos = v;
				v[1] = 0;
				a[i].normal = Vector(1.0f, 0.0f, 0.0f);
				a[i+1].pos = v;
				a[i+1].normal = Vector(1.0f, 0.0f, 0.0f);
			}
            renderer.DrawVertex3f3f(a, N, SNAKE_TRIANGLE_STRIP);
			// The left side:
			for (int i = 0; i < N; i += 2)
			{
				float b = (float)i / (N - 2);
				Vector v = (1.0f - b) * v1 + b * v2 - RAMP_WIDTH * xdiff;
				a[i+1].pos = v;
				a[i+1].normal = Vector(1.0f, 0.0f, 0.0f);
				v[1] = 0;
				a[i].pos = v;
				a[i].normal = Vector(-1.0f, 0.0f, 0.0f);
			}
            renderer.DrawVertex3f3f(a, N, SNAKE_TRIANGLE_STRIP);
			// The far side:
			for (int i = 0; i < N; i += 2)
			{
				float b = (float)i / (N - 2);
				Vector v = (1.0f - b) * v1 + b * v2;
				v[2] = v2[2];
				a[i].pos = v + RAMP_WIDTH * xdiff;
				a[i].normal = Vector(0.0f, 1.0f, 0.0f);
				a[i+1].pos = v - RAMP_WIDTH * xdiff;
				a[i+1].normal = Vector(0.0f, 1.0f, 0.0f);
			}
            renderer.DrawVertex3f3f(a, N, SNAKE_TRIANGLE_STRIP);
		}
		/*
		// Render the lamp:
		if (pMaterial != NULL)
		{
			if (m_bNight)
				pMaterial->m_eColor = m_lampColor;
			else
				pMaterial->m_eColor = BColor(0, 0, 0);
		}
		Model::m_lamp.Render();*/
	}

	// Render shields:
	for (int i = 0; i < m_shieldVisibleUntil.size(); i++)
	{
		if (m_shieldVisibleUntil[i] != 0)
		{
			Vector pos = m_shields[i];
			renderer.PushMatrix();
			renderer.TranslateTo(pos);
			renderer.Scale(10, 10, 10);
			//if (!bShadow)
				//renderer.Color((::byte)(255.0 * (1.0 - m_shieldHp / 100.0)), (::byte)(255.0 * (m_shieldHp / 100.0)), 0, 255);
			renderer.Color(255, 0, 0, 255);
			g_sphereModel.Render(renderer);
			renderer.PopMatrix();
		}
	}
}

void Level::RenderEarth(SnakeRenderer& renderer)
{
	if (m_type == LevelType_Simple || m_type == LevelType_ShowModel || m_type == LevelType_Shooter)
	{
		const int N = 50;

		if (!m_Texture.IsValid())
		{
			Vertex3f3f a[2 * N + 2];
			renderer.Color(255, 255, 255, 255);
			for (int i = 0; i < N; i++)
			{
				for (int j = 0; j <= N; j++)
				{
					a[2 * j + 0].pos = Vector((t_float)((i + 1) * m_gridMaxX / N), 0.0f, (t_float)(j * m_gridMaxZ / N));
					a[2 * j + 1].pos = Vector((t_float)(i * m_gridMaxX / N), 0.0f, (t_float)(j * m_gridMaxZ / N));
					a[2 * j + 0].normal = Vector(0.0f, 1.0f, 0.0f);
					a[2 * j + 1].normal = Vector(0.0f, 1.0f, 0.0f);
				}
				renderer.DrawVertex3f3f(a, 2 * N + 2, SNAKE_TRIANGLE_STRIP);
			}
		}

		if (m_Texture.IsValid())
		{
			Vertex3f3f2f a[6];
			Vertex3f3f2f v1, v2, v3, v4;
			v1.pos = Vector(0.0, 0.0, 0.0);
			v1.u = 0.0;
			v1.v = 0.0;
			v2.pos = Vector((t_float)m_gridMaxX, 0.0f, 0.0f);
			v2.u = 1.0;
			v2.v = 0.0;
			v3.pos = Vector(0.0f, 0.0f, (t_float)m_gridMaxZ);
			v3.u = 0.0;
			v3.v = 1.0;
			v4.pos = Vector((t_float)m_gridMaxX, 0.0f, (t_float)m_gridMaxZ);
			v4.u = 1.0;
			v4.v = 1.0;
			a[0] = v1;
			a[1] = v2;
			a[2] = v3;
			a[3] = v2;
			a[4] = v3;
			a[5] = v4;
			renderer.UseTexture(m_Texture.m_textHandle);
			renderer.DrawVertex3f3f2f(a, 6);
			renderer.Use3DProgram();
		}

		/*if (m_pGame->m_groundTex2 != 0)
		{

			glDepthFunc(GL_EQUAL);

			glBindTexture(GL_TEXTURE_2D, m_pGame->m_groundTex2);
			int size = m_backgroundWidth * m_backgroundHeight;
			uint8* pixs = new uint8[4 * size];
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixs);
			int current = m_iCurrentBackgound;
			int nextBack = m_iCurrentBackgound + 1;
			if (nextBack > 1)
				nextBack = 0;
			for (int i = 1; i < 127; i++)
			{
				for (int j = 1; j < 127; j++)
				{
					bool b = false;
					for (int ii = -1; ii <= 1 && !b; ii++)
					{
						for (int jj = -1; jj <= 1; jj++)
						{
							if (m_backgroundIndex[current][i + ii][j + jj] == 1)
							{
								b = true;
								break;
							}
						}
					}
					if (b)
						m_backgroundIndex[nextBack][i][j] = 1;
					else
						m_backgroundIndex[nextBack][i][j] = 0;
				}
			}
			for (int i = 0; i < size; i++)
			{
				int x = i % m_backgroundWidth;
				int y = i / m_backgroundWidth;
				if (m_backgroundIndex[nextBack][x * 128 / m_backgroundWidth][y * 128 / m_backgroundHeight] == 1)
					pixs[4 * i + 3] = 255;
				else
					pixs[4 * i + 3] = 0;
			}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_backgroundWidth, m_backgroundHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixs);
			delete[] pixs;
			glEnable(GL_BLEND);

			mw = m_pGame->m_maxGroundTexWidth2;
			mh = m_pGame->m_maxGroundTexHeight2;
			for (int i = 0; i < N; i++)
			{
				for (int j = 0; j <= N; j++)
				{
					a[2*j] = Vector((i + 1) * m_gridMaxX / N, 0.0, j * m_gridMaxZ / N);
					a[2*j+1] = Vector(i * m_gridMaxX / N, 0.0, j * m_gridMaxZ / N);
					t[2*j].u = mw * (i + 1) / N;
					t[2*j].v = mh * j / N;
					t[2*j+1].u = mw * i / N;
					t[2*j+1].v = mh * j / N;
				}
				m_pGame->DrawVectors(a, 2 * N + 2, SNAKE_TRIANGLE_STRIP);
			}
			m_iCurrentBackgound = nextBack;
		}*/
	}
}

void Level::RenderShadow(SnakeRenderer& renderer)
{
}


void Level::LoadEarthLevel(SnakeGame& game)
{
	m_type = LevelType_Simple;
	NewGame(game);
}

void Level::NewGame(SnakeGame& game)
{
    const float w = 1.0;
	switch (m_type)
	{
	case LevelType_Simple:
	case LevelType_ShowModel:
	case LevelType_Shooter:
	{
		if (m_iLevel == 1 || m_iLevel == 3)
		{
			m_gridMaxX = 100.0;
			m_gridMaxZ = 100.0;
			Vector zero(-w, w, -w);
			Vector x(m_gridMaxX + w, w, -w);
			Vector xz(m_gridMaxX + w, w, m_gridMaxZ + w);
			Vector z(-w, w, m_gridMaxZ + w);
			Vector dx(w, 0.0f, 0.0f);
			Vector dz(0.0f, 0.0f, w);
			Wall* pWall = new Wall(zero - dx, x + dx);
			game.AddEntity(pWall);
			pWall = new Wall(zero - dz, z + dz);
			game.AddEntity(pWall);
			pWall = new Wall(x - dz, xz + dz);
			game.AddEntity(pWall);
			pWall = new Wall(z - dx, xz + dx);
			game.AddEntity(pWall);

			if (m_iLevel == 3)
			{
				Door* pDoor1 = new Door(RandomLocation(), 5.0);
				Door* pDoor2 = new Door(RandomLocation(), 5.0, pDoor1);
				game.AddEntity(pDoor1);
				game.AddEntity(pDoor2);
			}

			m_pointsFotNextLevel = 100;
		}
		else
		{
			m_gridMaxZ = 200.0;
			m_gridMaxX = 30.0;
			// x and z are going from -w to max+w, because the line have a width of 5, and this
			// way the interior of the lines is exactly the rectangle from 0 to max
			Vector zero(-w, w, -w);
			Vector x(m_gridMaxX + w, w, -w);
			Vector xz(m_gridMaxX + w, w, m_gridMaxZ + w);
			Vector z(-w, w, m_gridMaxZ + w);
			Vector dx(w, 0.0f, 0.0f);
			Vector dz(0.0f, 0.0f, w);
			Wall* pWall = new Wall(zero - dx, x + dx);
			game.AddEntity(pWall);
			pWall = new Wall(zero - dz, z + dz);
			game.AddEntity(pWall);
			pWall = new Wall(x - dz, xz + dz);
			game.AddEntity(pWall);
			pWall = new Wall(z - dx, xz + dx);
			game.AddEntity(pWall);

			Vector half_z(0.0f, w, m_gridMaxZ / 2.0f);
			Vector half_zx(m_gridMaxX, w, m_gridMaxZ / 2.0f);
			pWall = new Wall(half_z, half_z+Vector(2*w, 0.0f, 0.0f));
			game.AddEntity(pWall);
			pWall = new Wall(half_zx-Vector(2*w, 0.0f, 0.0f), half_zx);
			game.AddEntity(pWall);
			m_pointsFotNextLevel = 100;
		}
		break;
	}
	case LevelType_Customize:
		break;
	default: // default for ball level for now
		// The earth:
		Sphere* pSphere = new Sphere();
		game.AddEntity(pSphere);
		// The sun:
		pSphere = new Sphere(5.0);
		pSphere->m_pos[1] = (t_float)(2.0 * m_sphereRadius);
		pSphere->SetColor(BColor(255, 255, 0));
		game.AddEntity(pSphere);
		// Create a door pair:
		Door* pDoor1 = new Door(RandomLocation(), 5.0);
		Door* pDoor2 = new Door(RandomLocation(), 5.0, pDoor1);
		game.AddEntity(pDoor1);
		game.AddEntity(pDoor2);
	}
	m_pGame = &game;

	if (m_type == LevelType_Shooter)
	{
		m_shields.clear();
		m_shieldVisibleUntil.clear();
		for (double x = 10.0; x < m_gridMaxX - 10.0; x += 20)
		{
			for (double z = 10.0; z < m_gridMaxZ - 10.0; z += 20)
			{
				m_shields.push_back(Vector((t_float)x, 0, (t_float)z));
				m_shieldVisibleUntil.push_back(0);
			}

		}
	}

	/*glBindTexture(GL_TEXTURE_2D, m_pGame->m_groundTex2);
	memset(m_backgroundIndex, 0, 2 * 128 * 128);
	glBindTexture(GL_TEXTURE_2D, 0);*/
}

Vector Level::RandomLocation()
{
	Vector pos;
	if (m_type == LevelType_Simple || m_type == LevelType_Shooter)
	{
		pos[0] = (t_float)(Random() * (m_gridMaxX - 20.0) + 10);
		pos[1] = 0;
		pos[2] = (t_float)(Random() * (m_gridMaxZ - 20.0) + 10);
	}
	else
	{
		pos[0] = (t_float)((Random() - 0.5) * m_sphereRadius);
		pos[1] = (t_float)((Random() - 0.5) * m_sphereRadius);
		pos[2] = (t_float)((Random() - 0.5) * m_sphereRadius);
		pos = ((t_float)m_sphereRadius / pos.Length()) * pos;
	}
	return pos;
}

void Level::FixCarForces(Car& car)
{
	if (m_type == LevelType_Simple || m_type == LevelType_Shooter)
	{
		//car.m_orientation = Matrix::LookAt(-car.m_velocity, Vector(0.0, 1.0, 0.0));
		//Vector up = m_terrain[0];
		//car.m_orientation = Matrix::Convert(-car.m_velocity, up);

		// Currently the m_pos var is the car back pos and m_carFront is the car front pos.
		// We want to change m_orientation to be in the direction between them.
	}
	else
	{
		// Stick the car to the sphere ground and fix orientation:
		car.m_pos = ((t_float)m_sphereRadius / car.m_pos.Length()) * car.m_pos;
		car.m_orientation = Matrix::Convert(-car.m_velocity, car.m_pos);
	}
}

void Level::FixGravityForces(Entity& ent, double time)
{
	if (m_type == LevelType_Simple || m_type == LevelType_Shooter)
	{
		float groundHeight = 0.0;

		/*
		Vector v = m_terrain[0];
		Vector g = v;
		g[1] = m_pos[1];
		double distance = (g - ent.m_pos).Length();
		if (distance <= 60.0)
			groundHeight = 20.0 * (1.0 - distance / 60.0);
		 */

		//Vector v = m_terrain[0];
		for (int iRamp = 0; iRamp < nRamps; iRamp++)
		{
			Vector v1 = ramps[iRamp].v1;
			Vector v2 = ramps[iRamp].v2;
			Vector xdiff = (v2 - v1).CrossProduct(Vector(0.0f, 1.0f, 0.0f));
			if (ent.m_pos[0] >= v1[0] - RAMP_WIDTH && ent.m_pos[0] <= v1[0] + RAMP_WIDTH && ent.m_pos[2] >= v1[2] - 50 && ent.m_pos[2] <= v1[2])
				groundHeight = v1[2] - ent.m_pos[2];
		}
		if (ent.m_pos[1] <= groundHeight)
		{
			ent.m_pos[1] = groundHeight;
			if (ent.m_velocity[1] < 0.0)
			{
				// 90% opposite force:
				ent.m_velocity[1] =  -0.9f * ent.m_velocity[1];
				//ent.m_velocity[1] =  -1.0 * ent.m_velocity[1];
			}
			if (ent.m_velocity[1] < 7.0)
				ent.m_velocity[1] = 0.0;
		}
		else
		{
			// gravity:
			ent.m_velocity[1] += (t_float)(m_g * time);
		}
	}
	else if (m_type != LevelType_Customize)
	{
		if (ent.m_pos.Length() < m_sphereRadius)
		{
			// The normal:
			Vector n = ent.m_pos;
			n.Normalize();
			double len = n.DotProduct(ent.m_velocity);
			// -1.0 to eliminte current velocity, -0.9 for 90% opposite force:
			ent.m_velocity = ent.m_velocity - (t_float)(1.9 * len) * n;
		}
		else
		{
			// gravity:
			float g = (float)m_g;
			Vector gravity = ent.m_pos;
			gravity.Normalize();
			gravity = g * gravity;
			ent.m_velocity = ent.m_velocity + (t_float)time * gravity;
		}
	}
}

// The following is not always true, depending on the light implementation in SnakeRenderer:
// The SetLights() call must be made after setting the view matrix (e.g. gluLookAt)
// when calling SetLights() we should have: viewing transformation and only the viewing transformation is applied to the MODELVIEW matrix
void Level::SetLights(SnakeRenderer& renderer)
{
	Color white(1.0, 1.0, 1.0);
	renderer.SetLight(0, m_lightPos, white);

	if (m_bNight)
	{
		// Darker, yellowish color from the lamp at night:
		Color color;
		for (int i = 0; i < 4; i++) 
			color.c[i] = m_lampColor.c[i] / 255.0f;
		renderer.SetLight(0, m_lightPos, color);
	}
}

void Level::UseShadowMatrix(SnakeRenderer& renderer)
{
	//float height = 300.0f;
	Matrix mat;
	// The earth noraml.  Use -1, so the resulting vectors (x, y, z, w) will have positive w, else
	// this is not works usually.
	Vector n(0.0f, -1.0f, 0.0f);
	// The light position:
	Vector l = m_lightPos;

	float d = l.DotProduct(n);
	float c = -d; // This is only in our special case where the plane is passing the origin
	// c is the distance between the light source to the plane (in our case, d is negative)
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
			mat[i][j] = l[j] * n[i];
		mat[i][3] = n[i]; // the w coordinate of result vector will be (n*v-d), v is the original vector
	}
	for (int i = 0; i < 3; i++)
		mat[i][i] += c;
	for (int j = 0; j < 3; j++)
		mat[3][j] = 0.0f; // This is only in our special case where the plane is passing the origin (no translation)
	mat[3][3] = -d;
	renderer.MultMatrix(mat);
	
	// v_new = (l*((n^T)*v) + c*v)/(n*v-d) = ((n*v)l - c*v) / (n*v-d)
	// -n*v id the distance between v to the plane, let -n*v=g (d=-c)
	// => v_new = (-g*l-c*v) / (-g-c) = (g*l+c*v) / (g+c)
	
	// from "casting shadows on flat surfaces" by Thant Tessman, which explain casting on the y=0 plane:
	// (lx, ly) - light source position
	// by similar triangles: (lx-Xnew) / ly = (lx-Xold) / (ly-Yold)
	// => Xnew = (ly*Xold - lx*Yold) / (ly -Yold)
	// => M = | ly 0  0  0 |
	//        |-lx 0 -lz -1|
	//        | 0  0  ly 0 |
	//        | 0  0  0  ly|
	// Xnew = Xold*M
	
	// see also http://www.faqs.org/faqs/graphics/opengl-faq/part3/
}

void Level::NextLevel()
{
	m_iLevel++;
	m_terrain.clear();
	m_bLevelCompleted = false;
	//if (m_iLevel == 2)
		//m_type = LevelType_Sphere;
	if (m_iLevel == 3) {
		m_terrain.push_back(Vector(100.0f, 20.0f, 100.0f));
		nRamps = 1;
	}
	// Level::NewGame() will be called soon with other initializations
}
