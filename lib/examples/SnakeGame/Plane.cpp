//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Plane.hpp"
#include "Model.hpp"
#include "SnakeGame.hpp"
#include "defines.hpp"
#ifdef WIN32
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif
#include <Windows.h>
#endif

#define CHANGE_TARGET_RATE 1000

extern Model g_airplaneModel;


double Random();

enum Target
{
	Target_Forward, Target_Player1, Target_Player2, Target_Left, Target_Right
};

Plane::Plane(void)
{
	m_type = EntityType_Plane;
	m_orientation = Matrix::I();
	m_pos = Vector(0.0, 10.0, 0.0);
	m_speed = PLANE_SPEED;
	m_target = Target_Forward;
	m_lastTargetChange = 0;
	m_pOrderEntity = NULL;
}

Plane::~Plane(void)
{
}

bool Plane::Think(SnakeGame& game)
{
	if (!game.m_bGameServer)
		return false;
	bool bGotoDest = false;
	double time = game.m_timeDiff;
	//Vector direction = m_orientation * Vector(0.0, 0.0, -1.0);
	//Vector p1 = game.m_players[0]->m_pos - m_pos;
	Vector destPosition;
	if (game.m_players[0] && game.m_players[0]->m_pCar) {
		destPosition = game.m_players[0]->m_pCar->m_pos;
		bGotoDest = true;
	}

	/*if (m_pOrderEntity != NULL)
	{
		Vector destDiff = m_orderDest - m_pos;
		Vector destDir = destDiff;
		destDir.Normalize();
		destDir = m_orientation.Transpose() * destDir;
		Vector direction = Vector(0.0, 0.0, -1.0);
		Vector d = destDir - direction;
		if (d[0] > 0.1)
			m_orientation = m_orientation * Matrix::RotateY(-PLANE_ROTATE_SPEED * time);
		else if (d[0] < -0.1)
			m_orientation = m_orientation * Matrix::RotateY(PLANE_ROTATE_SPEED * time);
		if (d[1] > 0.1)
			m_orientation = m_orientation * Matrix::RotateX(-PLANE_ROTATE_SPEED * time);
		else if (d[1] < -0.1)
			m_orientation = m_orientation * Matrix::RotateX(PLANE_ROTATE_SPEED * time);
		if (destDiff.Length() < 2.0)
		{
			game.AddEntity(m_pOrderEntity);
			m_pOrderEntity = NULL;
		}
	}
	else*/ if (bGotoDest)
	{
		Vector destDir = destPosition - m_pos;
		destDir.Normalize();
		destDir[1] = 0.0; // don't move in the y coordinate
		Vector d = m_orientation.Transpose() * destDir;
		//Vector direction = Vector(0.0, 0.0, -1.0);
		//Vector forwardDirection = m_orientation * Vector(0.0, 0.0, -1.0);

		//Vector d = destDir - direction;
		if (d[0] > 0.1)
			m_orientation = m_orientation * Matrix::RotateY((t_float)(-PLANE_ROTATE_SPEED * time * M_PI / 180.0f));
		else if (d[0] < -0.1)
			m_orientation = m_orientation * Matrix::RotateY((t_float)(PLANE_ROTATE_SPEED * time * M_PI / 180.0f));
		/*if (d[1] > 0.1)
			m_orientation = m_orientation * Matrix::RotateX(-PLANE_ROTATE_SPEED * time);
		else if (d[1] < -0.1)
			m_orientation = m_orientation * Matrix::RotateX(PLANE_ROTATE_SPEED * time);*/

		/*bool bChangeTarget = false;
		m_target = Target_Player1;
		if (m_lastTargetChange == 0 || m_lastTargetChange - game.m_timestamp > CHANGE_TARGET_RATE)
		{
			if (Random() < 0.10)
			{
				bChangeTarget = true;
			}
		}
		switch (m_target)
		{
		case Target_Forward:
			if (bChangeTarget)
				m_target = rand() % 5;
			break;
		case Target_Left:
			m_angle += PLANE_ROTATE_SPEED * time;
			if (bChangeTarget)
				m_target = rand() % 5;
			break;
		case Target_Right:
			m_angle -= PLANE_ROTATE_SPEED * time;
			if (bChangeTarget)
				m_target = rand() % 5;
			break;
		case Target_Player1:
			if (p1.Length() < 3.0)
				m_target = rand() % 5;
			break;
		case Target_Player2:
			p1.Normalize();
			if (direction.DotProduct(p1) < 0)
				m_angle += PLANE_ROTATE_SPEED * time;
			else
				m_angle -= PLANE_ROTATE_SPEED * time;
			//m_angle = acos(p1.DotProduct(p1));
			break;
		}
		//m_angle += (2.0 * Random() - 1.0) * 20.0 * time;
		if (m_lastTargetChange == 0 || m_lastTargetChange - game.m_timestamp > CHANGE_TARGET_RATE)
		{
			if (Random() < 0.10)
			{
				m_target = rand() % 3;
			}
		}*/
	}
#ifdef WIN32
	if (IS_KEY('O'))
		m_orientation = m_orientation * Matrix::RotateY((t_float)(+ ROTATE_SPEED * time));
	else if (IS_KEY('P'))
		m_orientation = m_orientation * Matrix::RotateY((t_float)(-ROTATE_SPEED * time));
	if (IS_KEY(VK_LEFT))
		m_orientation = m_orientation * Matrix::RotateZ((t_float)(+ROTATE_SPEED * time));
	else if (IS_KEY(VK_RIGHT))
		m_orientation = m_orientation * Matrix::RotateZ((t_float)(-ROTATE_SPEED * time));
	if (IS_KEY(VK_DOWN))
		m_orientation = m_orientation * Matrix::RotateX((t_float)(+ROTATE_SPEED * time));
	else if (IS_KEY(VK_UP))
		m_orientation = m_orientation * Matrix::RotateX((t_float)(-ROTATE_SPEED * time));
	/*if (IS_KEY('9'))
		m_speed = PLANE_SPEED;
	if (IS_KEY('2'))
		m_speed = DEFAULT_SPEED;
	if (IS_KEY('1'))
		m_speed = 5;
	if (IS_KEY('0'))
		m_speed = 0;
	/*if (m_pos[0] > ARENAX * 1.1)
		m_pos[0] -= ARENAX * 1.1;
	else if (m_pos[0] < -ARENAX * 0.1)
		m_pos[0] += ARENAX * 1.1;
	if (m_pos[2] < ARENAZ * 1.1)
		m_pos[2] -= ARENAZ * 1.1;
	else if (m_pos[2] > -ARENAZ * 0.1)
		m_pos[2] += ARENAZ * 1.1;*/
#else
	// windows constants:
	const int VK_LEFT = 0x25;
	const int VK_UP = 0x26;
	const int VK_RIGHT = 0x27;
	const int VK_DOWN = 0x28;

	if (game.m_keys['O'])
		m_orientation = m_orientation * Matrix::RotateY(+ROTATE_SPEED * time);
	else if (game.m_keys['P'])
		m_orientation = m_orientation * Matrix::RotateY(-ROTATE_SPEED * time);
	if (game.m_keys[VK_LEFT])
		m_orientation = m_orientation * Matrix::RotateZ(+ROTATE_SPEED * time);
	else if (game.m_keys[VK_RIGHT])
		m_orientation = m_orientation * Matrix::RotateZ(-ROTATE_SPEED * time);
	if (game.m_keys[VK_DOWN])
		m_orientation = m_orientation * Matrix::RotateX(+ROTATE_SPEED * time);
	else if (game.m_keys[VK_UP])
		m_orientation = m_orientation * Matrix::RotateX(-ROTATE_SPEED * time);
#endif
	m_velocity = (t_float)m_speed * (m_orientation * Vector(0.0, 0.0, -1.0));
	//m_velocity = Vector(0.0, 0.0, 0.0);*/
	
	//game.AddEvent(Command_SetVectorProperty, m_index, Property_Position, m_pos, false);
	//game.AddEvent(Command_SetVectorProperty, m_index, Property_Velocity, m_velocity, false);

	return false;
}

void Plane::Render(SnakeRenderer& renderer)
{
	renderer.MultMatrix(m_orientation);
	renderer.RotateY(180);
	renderer.Scale(5.0, 5.0, 5.0);
	// fix f16 model center:
	renderer.Translate(1.5, -3.25, 0.0);
	renderer.Color(0, 255, 0, 255);
	g_airplaneModel.Render(renderer);
}

void Plane::Draw2D(SnakeRenderer& renderer)
{
	renderer.Color(255, 0, 0, 255);
	renderer.PointSize(4.0);
	renderer.DrawVectors(&m_pos, 1, SNAKE_POINTS);
}

void Plane::Order(const Vector& dest, Entity* pNewEntity)
{
	m_orderDest = dest;
	if (m_pOrderEntity != NULL)
		delete m_pOrderEntity;
	m_pOrderEntity = pNewEntity;
}

bool Plane::SetProperty(int index, const Vector& val)
{
	bool bResult = Entity::SetProperty(index, val);
	// for now assume only y rotation:
	if (index == Property_Velocity)
	{
		double y_angle = atan2(-val[0], -val[2]);
		m_orientation = Matrix::RotateY((t_float)y_angle);
	}
	return bResult;
}
