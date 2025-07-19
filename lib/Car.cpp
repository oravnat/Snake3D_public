//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Car.hpp"
#include <math.h>
#include "SnakeGame.hpp"
#include <vector>
#include "Model.hpp"
#include "Bullet.hpp"
#include "defines.hpp"

#define GUN_RANGE 15.0
//#define AHEAD_LENGTH 30.0


using namespace std;

extern Model g_carModel;
extern Model g_sphereModel;
extern Model g_bulletModel;
extern Model g_airplaneModel;
extern Model g_pistolModel;


Car::Car(SnakeGame* pGame, Player* pPlayer) : m_pGame(pGame), m_pTail(NULL), m_rotation(0), m_pPlayer(pPlayer), bFirst(false),
                                         m_acceleration(0.0), m_gunUntil(0), m_lastBulletFireTime(0), m_lastFireTime(0), m_shieldAlpha(0.0),
	                                     m_pitch(0.0), m_yawAngle(0.0), m_shieldHp(0), m_shieldVisibleUntil(0), m_speed(0.0), m_tailSizeWhenDead(0)
{
	m_type = EntityType_Car;
	m_nBalls = 0;
	m_deadUntil = 0;
	m_deadTime = 0;
	m_undeadUntil = 0;
	m_airRejectionConstant = 5.0;
	m_groundFrictionConstant = 5.0;
	m_bLaser = false;
	m_bOriented = false;

	m_orientation = Matrix::I();
}

Car::~Car(void)
{
}

void Car::Render(SnakeRenderer& renderer)
{
    renderer.Color(m_hullColor.c[0], m_hullColor.c[1], m_hullColor.c[2], 255);

    renderer.PushMatrix();
	// Render the car itself:
	RenderCar(renderer);
	renderer.PopMatrix();

	if (m_deadUntil != 0)
		return;

	renderer.PushMatrix();
	RenderShield(renderer);
	renderer.PopMatrix();

	renderer.PushMatrix();
	//RenderGun();
	renderer.PopMatrix();
}

void Car::Draw2D(SnakeRenderer& renderer)
{
	if (m_deadUntil != 0)
		return;
	renderer.Color(0, 0, 0, 255);
	renderer.PointSize(3.0);
	Vector a[2];
	a[0] = m_carBack;
	a[1] = m_carFront;
	renderer.DrawVectors(a, 2, SNAKE_POINTS);
	renderer.DrawVectors(a, 2, SNAKE_LINES);
}

void Car::Advance(double timediff)
{
	m_prevBack = m_pos;
	m_prevFront	= m_carFront;
	Entity::Advance(timediff);
	Vector unit(0.0, 0.0, -1.0);
	m_carBack = m_pos;
    Matrix pitcMtr = Matrix::RotateX((t_float)m_pitch);

    m_carFront = m_carBack + CAR_LENGTH * (m_orientation * (pitcMtr * unit));
    m_frontWheel.m_pos = m_carFront;
}

bool Car::Think(SnakeGame& game)
{
	bool bResult = false;
	bool bAngleChanged = false;
	double time = game.m_timeDiff;

	if (game.m_level.m_type == LevelType_Customize)
	{
		m_orientation = Matrix::RotateY((t_float)(game.m_timestamp * 2 * M_PI / 10000));
		return false;
	}

	// If the player is dead:
	if (m_deadUntil != 0)
	{
		if (game.m_timestamp >= m_deadUntil)
		{
			if (m_pPlayer)
				m_pPlayer->NextLife();
		}
		else
		{
			// Shrink his tail:
			if (m_pTail != NULL)
			{
				m_pTail->m_tailSize = m_tailSizeWhenDead - (game.m_timestamp - m_deadTime) / 10;
				if (m_pTail->m_tailSize < 0)
					m_pTail->m_tailSize = 0;
			}
			return false;
		}
	}

	if (bFirst)
		m_undeadUntil = m_pGame->m_timestamp + UNDEAD_TIME;

	// Add tail:
    if (!bFirst && m_undeadUntil == 0 && m_pTail != NULL)
    {
		// on a multiplayer game, only the server makes the tails grow:
		if (game.m_bGameServer)
		{
			Vector up = m_orientation * Vector(0.0, 1.0, 0.0);
			Vector right = m_orientation * Vector(1.0, 0.0, 0.0);
			bool bAdd = m_pTail->Add(m_prevBack, right, up, game.m_timestamp);
			if (bAdd)
                game.m_network.AddVectorPacket(Command_SetVectorProperty, m_pTail->m_index, TAIL_VECTOR_PROPERTY_ADD, m_prevBack);
		}
    }
	float angle = (float)(m_rotation * ROTATE_SPEED * time);
	// Turn car according to m_rotation
	if (angle != 0.0f)
	{
		m_orientation = m_orientation * Matrix::RotateY(angle);
		bAngleChanged = true;
	}


    if (game.m_level.m_type == LevelType_Simple)
	{
		// Check if fall from the arena:
		// TODO: move to m_level
		if (m_pos[0] < 0.0 || m_pos[0] > game.m_level.m_gridMaxX || m_pos[2] < 0.0 || m_pos[2] > game.m_level.m_gridMaxZ)
		{
			//m_msg << empty << L"Player " << m_id << L" has been fall from the arena.\n";
			bResult = true;
		}
	}


	Vector unit(0.0, 0.0, -1.0);
    Vector dir = m_orientation * unit;
    Vector engineForce = (t_float)m_acceleration * dir;
    Vector brakes = m_velocity;
    //Vector groundBrakes = time * m_groundFrictionConstant * dir;
    //groundBrakes[1] = 0.0;
    //if (time * (m_airRejectionConstant + m_groundFrictionConstant / brakes.Length()) < 1.0)
    //brakes = time * m_airRejectionConstant * brakes + groundBrakes;
    if (time * m_airRejectionConstant < 1.0)
        brakes = (t_float)time * m_airRejectionConstant * brakes;

    m_velocity += (t_float)time * engineForce - brakes;

    float height = m_pos[1];
	game.m_level.FixGravityForces(*this, time);
	game.m_level.FixGravityForces(m_frontWheel, time);
    //if (m_frontWheel.m_pos[1] != m_carFront[1])
    {
        double dy = m_frontWheel.m_pos[1]-m_pos[1];
        m_pitch = atan2(dy, CAR_LENGTH);
    }

	if (m_pos[1] - height > 5)
	{
		bResult = true;
		m_pos[1] = height;
	}

	if (m_shieldVisibleUntil < game.m_timestamp)
		m_shieldVisibleUntil = 0;

	if (m_gunUntil < game.m_timestamp)
		m_gunUntil = 0;

	if (m_undeadUntil < game.m_timestamp)
		m_undeadUntil = 0;

	bFirst = false;

	if (bResult)
	{
		// Only the server decide to kill the client (is it good?):
		if (game.m_bGameServer)
        {
            Kill();
            game.m_network.AddScalarPacket(Command_SetIntProperty, m_index, CAR_INT_PROPERTY_COMMAND, CarCommand_Killed);
        }
		bResult = false;
	}

	if (m_pTail != NULL)
	{
		Vector up = m_orientation * Vector(0.0, 1.0, 0.0);
		Vector right = m_orientation * Vector(1.0, 0.0, 0.0);
		m_pTail->m_carPos = m_prevBack;
		m_pTail->m_right = right;
		m_pTail->m_up = up;
	}


	// for tests only:
	#define MAX_RUN_TIME 30000
	//m_gunUntil = game.m_timestamp + MAX_RUN_TIME;

	// Gun fire:
	if (m_gunUntil > 0 && game.m_nPlayers == 2)
	{
		//int otherIndex = 2 - m_id;
		int otherIndex = 1;
		Car* pOther = game.m_players[otherIndex]->m_pCar;
		// gun origin is -70 from car back (scaled by 0.1):
		Vector gPos = m_pos + 7.0 * dir;
		Vector otherDir = pOther->m_orientation * unit;
		Vector otherGPos = pOther->m_pos + 7.0 * otherDir;
		Vector diff = otherGPos - gPos;
		if (diff.Length() <= GUN_RANGE/* && diff.DotProduct(m_velocity) >= 0.0*/)
		{
			if (m_lastBulletFireTime == 0 || (game.m_timestamp - m_lastBulletFireTime > 100))
			{
				//Vector up = m_pos;
				//up.Normalize();
				Vector up(0.0, 1.0, 0.0);
				Vector velocity = diff;
				velocity.Normalize();
				// gun distance from origin to end is 55 (scaled by 0.1):
				Vector pos = gPos + 5.0 * up + 5.5 * velocity;
				velocity = BALL_SPEED * velocity;
				Bullet* pBullet = new Bullet(pos, velocity, game.m_timestamp + 2000, this);
				game.PlayResSound(3);
				if (game.AddEntity(pBullet))
				{
					m_lastBulletFireTime = game.m_timestamp;
				}
                if (game.m_network.AddScalarPacket(Command_BulletFire, 0, 0, 0))
				{
					//m_lastBulletFireTime = game.m_timestamp;
				}
			}
		}
	}
	
	m_yawAngle = atan2(-m_velocity[0], -m_velocity[2]);
	if (m_yawAngle < 0.0)
		m_yawAngle += 2.0 * M_PI;
	if (game.m_bGameServer)
	{
		// for now, the player decide its direction/velocity, the server decides the position:
        game.m_network.AddVectorPacket(Command_SetVectorProperty, m_index, Property_Position, m_pos);
		game.m_network.AddVectorPacket(Command_SetVectorProperty, m_index, Property_Velocity, m_velocity);
		game.m_network.AddScalarPacket(Command_SetIntProperty, m_index, CAR_INT_PROPERTY_Y_ROTATION, (uint16_t)(m_yawAngle * 0xFFFF / (2.0 * M_PI)));
	}
	else //if (bAngleChanged)
	{
		/*
		TODO: fix
		if (m_bLocalCar)
		{
			// for now, the player decide its direction/velocity, the server decides the position:
			game.AddEvent(Command_SetVectorProperty, m_index, Property_Velocity, m_velocity, false);
			game.AddEvent(Command_SetIntProperty, m_index, CAR_INT_PROPERTY_Y_ROTATION, (uint16_t)(m_yawAngle * 0xFFFF / (2.0 * M_PI)), false);
		}
		*/
	}

	return false;
}

void Car::SetHullColor(BColor color)
{
    m_hullColor = color;
    /*Material* pHull = m_model.GetMaterial("Hull");
    if (pHull != NULL)
    {
        // Blue diffuse:
        pHull->m_dColor = color;
        // Dark yellow specular:
        pHull->m_sColor = BColor(128, 128, 0);
    }*/
}


void Car::RenderCar(SnakeRenderer& renderer)
{
	// Blinking effect: draw only half of the time:
	if (m_undeadUntil != 0 && ((m_undeadUntil - m_pGame->m_timestamp) % 250) < 125)
		return;

	// Render the car:
	renderer.MultMatrix(m_orientation);
    renderer.RotateX((float)(m_pitch * 180.0 / M_PI));

    renderer.Scale(0.2f, 0.2f, 0.2f); // change from 10 meter length to 2 meter length
	renderer.Translate(0.0, 2.0, -4.0);
	renderer.RotateY(180);
	renderer.RotateX(-90);

	if (m_deadTime == 0)
	{
		if (g_carModel.m_materials.size() > 0)
			g_carModel.m_materials[0].m_dColor = m_hullColor;
		g_carModel.Render(renderer);
	}
	else
	{
	    Model m_tempModel;
		float r = (m_pGame->m_timestamp - m_deadTime) / 100.0f;
        g_carModel.Boom(m_tempModel, r);
		m_tempModel.Render(renderer);
	}
}

void Car::RenderShield(SnakeRenderer& renderer, bool bShadow)
{
	// Render shield:
	if (m_shieldVisibleUntil != 0)
	{
		Vector pos = 0.5 * (m_carFront - m_pos);
        renderer.TranslateTo(pos);
        renderer.Scale(SHIELD_SIZE, SHIELD_SIZE, SHIELD_SIZE);
		if (!bShadow)
            renderer.Color((uint8_t)(255.0 * (1.0 - m_shieldHp / 100.0)), (uint8_t)(255.0 * (m_shieldHp / 100.0)), 0, 255);
        g_sphereModel.Render(renderer);
	}
}

void Car::RenderShadow(SnakeRenderer& renderer)
{
    if (m_deadUntil != 0)
    {
        renderer.PushMatrix();
        // Render the car itself:
        RenderCar(renderer);
        renderer.PopMatrix();
        return;
    }

    renderer.PushMatrix();
    // Render the car itself:
    RenderCar(renderer);
    renderer.PopMatrix();

    if (m_pTail != NULL)
    {
        renderer.PushMatrix();
        renderer.TranslateTo(-m_pos);
        // Render the tail:
        m_pTail->RenderShadow(renderer);
        renderer.PopMatrix();
    }

    renderer.PushMatrix();
    RenderShield(renderer, true);
    renderer.PopMatrix();
}

void Car::Kill()
{
    if (m_deadUntil == 0)
    {
        if (m_pTail != NULL)
        {
            m_tailSizeWhenDead = m_pTail->m_tailSize;
            m_pTail->m_maxTailSize = 0;
        }
        m_pGame->PlayResSound(2);
        m_deadUntil = m_pGame->m_timestamp + DEAD_TIME;
        m_deadTime = m_pGame->m_timestamp;
        m_nBalls = 0;
        //m_points += DEATH_POINTS;
        m_pGame->m_bUpdateStatus = true;
    }
}

bool Car::SetProperty(int index, int val)
{
	/*
	// TODO: fix
	if (index == CAR_INT_PROPERTY_PLAYER)
	{
	    m_player = val;
		m_pGame->m_players[m_pGame->m_nPlayers++] = this;
		SetHullColor(BColor(230, 0, 0));
		// Blue color for balls:
		m_color = BColor(0, 0, 255);
	}*/ 
	if (index == CAR_INT_PROPERTY_PRIZE_TAKEN)
	{
		if (m_pPlayer)
		{
			m_pPlayer->m_points += PRIZE_POINTS;
			m_pPlayer->m_money += PRIZE_DOLLARS;
		}
		m_gunUntil = 0;
		if (m_pTail != NULL)
			m_pTail->Enlarge();
		m_pGame->m_bUpdateStatus = true;
		m_pGame->PlayResSound(1);
        if (SnakeGame::m_nPlayers == 1 && m_pPlayer && m_pPlayer->m_points >= m_pGame->m_level.m_pointsFotNextLevel) {
			m_pPlayer->m_points = 0;
			m_pGame->m_level.m_bLevelCompleted = true;
		}
		return true;
	}
	if (index == CAR_INT_PROPERTY_COMMAND)
	{
		switch(val)
		{
		case CarCommand_Killed:
			Kill();  // server tells us to kill ourself
			return true;
		}
	}

	/*
	// TODO: fix
	if (index == CAR_INT_PROPERTY_Y_ROTATION)
	{
		m_yawAngle = val * 2.0 * M_PI / 0xFFFF;
		if (!m_bLocalCar)
			m_orientation = Matrix::RotateY(m_yawAngle);
	}*/
	return false;
}

bool Car::SetProperty(int index, const Vector& val)
{
	bool bResult = false;
	if (!m_pPlayer || !m_pPlayer-> m_bLocalCar || index != Property_Velocity)
		bResult = Entity::SetProperty(index, val);
	if (index == Property_Position)
	{
		//assert(m_pos[0] > 0.0);
		if (!m_pGame->m_bGameServer)
		{
			m_carBack = m_pos;
			m_prevBack = m_pos;
			m_prevFront = m_pos;
		}
	}
	if (index == Property_Velocity)
	{
	}
	return bResult;
}

bool Car::IsPointInside(const Vector& p)
{
    Vector diff = p - m_carFront;
    bool result = diff.Length2() < HIT_DISTANCE2;
    if (!result)
    {
        Vector diff = p - m_carBack;
        result = diff.Length2() < HIT_DISTANCE2;
    }
    return result;
}

bool Car::ReactToCar(SnakeGame& game, Car& car)
{
    return false;
}
