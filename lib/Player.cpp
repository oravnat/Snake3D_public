//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Player.hpp"
#include "Car.hpp"
#include <math.h>
#include "SnakeGame.hpp"
#include <vector>
#include "Model.hpp"
#include "Bullet.hpp"
#include "defines.hpp"

const int LEFT = 0;
const int RIGHT = 1;

const int machine_gun_rate_ms = 100;
const int pistol_rate_ms = 1000;

void Player::NextLife()
{
	Vector unit(0.0, 0.0, -1.0);
	double yrotation = 0.0;
	Vector m_pos;
	if (!m_pCar)
		return;
	if (pGame->m_bGameServer)
	{
		m_pos = pGame->RandomLocation();
		m_pos[1] = 30.0; // meters
	}
	switch (pGame->m_level.m_type)
	{
	case LevelType_Simple:
	case LevelType_Shooter:
	{
		Vector dir((t_float)(pGame->m_level.m_gridMaxX / 2.0), 0.0f, (t_float)(pGame->m_level.m_gridMaxZ / 2.0));
		dir -= m_pos;
		//dir[0] = Random() - 0.5;
		dir[1] = 0.0;
		//dir[2] = Random() - 0.5;
		dir.Normalize();
		if (pGame->m_bGameServer)
		{
			m_pCar->m_orientation = Matrix::Convert(-dir, Vector(0.0, 1.0, 0.0));
			Vector unit(0.0, 0.0, -1.0);
			m_pCar->m_velocity = (t_float)m_pCar->m_speed * (m_pCar->m_orientation * unit);
		}
		else
			m_pCar->m_bOriented = false;
		m_pCar->m_speed = DEFAULT_SPEED;
		yrotation = atan2(-dir[0], -dir[2]);
		if (yrotation < 0.0)
			yrotation += 2.0 * M_PI;
		break;
	}
	case LevelType_Customize:
		m_pos = Vector(0.0f, 0.0f, 0.0f);
		m_pCar->m_speed = 0.0;
		m_pCar->m_velocity = Vector(0.0f, 0.0f, 0.0f);
		m_pCar->m_orientation = Matrix::I();
		break;
	default:
		Vector dir;
		dir[0] = (t_float)(Random() - 0.5);
		dir[1] = (t_float)(Random() - 0.5);
		dir[2] = (t_float)(Random() - 0.5);
		m_pCar->m_orientation = Matrix::Convert(-dir, m_pos);
		m_pCar->m_speed = DEFAULT_SPEED;
		m_pCar->m_velocity = (t_float)m_pCar->m_speed * (m_pCar->m_orientation * unit);
	}
	m_cameraOrientation = m_pCar->m_orientation;
	m_pCar->m_acceleration = CAR_DEFAULT_ACCELERATION;
	m_points = 0;
	if (m_pCar->m_pTail != NULL)
	{
		m_pCar->m_pTail->m_nextTail = 0;
		m_pCar->m_pTail->m_tailSize = 0;
		m_pCar->m_pTail->m_maxTailSize = TAIL_START_SIZE;
	}
	m_pCar->m_lastFireTime = 0;
	m_pCar->m_lastBulletFireTime = 0;
	m_pCar->bFirst = true;
	m_pCar->m_nBalls = 1;
	m_pCar->m_deadUntil = 0;
	m_pCar->m_deadTime = 0;
    m_pCar->m_pos = m_pos;
	m_pCar->m_carBack = m_pos;
	m_pCar->m_prevBack = m_pos;
	m_pCar->m_prevFront = m_pos;
	m_pCar->m_carFront = m_pCar->m_carBack + CAR_LENGTH * (m_pCar->m_orientation * unit);
	m_pCar->m_shieldHp = 100;
	m_pCar->m_shieldVisibleUntil = 0;
	m_pCar->m_shieldAlpha = 0.0;
	m_pCar->m_gunUntil = 0;
	m_keyReverseUntil = 0;
	if (pGame->m_bGameServer && m_pCar->m_index != -1)
	{
        pGame->m_network.AddVectorPacket(Command_SetVectorProperty, m_pCar->m_index, Property_Position, m_pos);
        pGame->m_network.AddVectorPacket(Command_SetVectorProperty, m_pCar->m_index, Property_Velocity, m_pCar->m_velocity);
        pGame->m_network.AddScalarPacket(Command_SetIntProperty, m_pCar->m_index, CAR_INT_PROPERTY_Y_ROTATION, (uint16_t)(yrotation * 0xFFFF / (2.0 * M_PI)));
	}
	/*if (m_lives > 0)
		m_lives--;
	else
	{
		pGame->Pause();
	}*/
	if (m_pCar)
		m_pCar->m_pos = m_pos;
}

void Player::NewGame()
{
	m_lives = 3;
	m_points = 0;
	m_money = 0;
	NextLife();
}

void Player::Think()
{
	switch (m_input)
	{
        case PlayerInput_NONE: // may be an external input (m_pCar->m_rotation already set)
		break;
        case PlayerInput_LEFT_HAND: 
		if (pGame->bHandValid[LEFT])
		{
			Vector z(0.0, 0.0, -1.0);
			Vector dir = pGame->m_handPoses[LEFT] * z;
			if (m_pCar)
				m_pCar->m_rotation = -dir[0];
		}
		break;
        case PlayerInput_RIGHT_HAND: 
		if (pGame->bHandValid[RIGHT])
		{
			Vector z(0.0, 0.0, -1.0);
			Vector dir = pGame->m_handPoses[RIGHT] * z;
			if (m_pCar)
				m_pCar->m_rotation = -dir[0]; 
		}
		break;
	}

	// For the reverse key to work, Player::Think() must be called before Car::Think()
	if (m_keyReverseUntil != 0)
	{
		if (m_pCar)
			m_pCar->m_rotation = -m_pCar->m_rotation;
	}

	// Fire a ball:
	if (pGame->m_bBallButton && m_bLocalCar && m_pCar)
	{
		if ((m_pCar->m_lastFireTime == 0 || (pGame->m_timestamp - m_pCar->m_lastFireTime > SHOOT_RATE)) && m_pCar->m_nBalls > 0)
		{
            if (pGame->Execute(Command_Fire, 0, 0, m_pCar->m_index))
			{
                pGame->m_network.AddScalarPacket(Command_Fire, 0, 0, m_pCar->m_index);
				m_pCar->m_lastFireTime = pGame->m_timestamp;
				//m_nBalls--;
			}
		}
	}

	if (m_pCar && pGame->m_viewMode == ViewMode_Follow)
	{
		float a = 0.2f;
		//m_cameraOrientation = a * m_pCar->m_orientation + (1.0f - a) * m_cameraOrientation;
        m_cameraOrientation = m_pCar->m_orientation;
	}

	if (m_keyReverseUntil < pGame->m_timestamp)
		m_keyReverseUntil = 0;

	if (m_bShootButton && !m_bPrevShootButton && m_pCar)
	{
		if (m_pCar->m_lastBulletFireTime == 0 || (pGame->m_timestamp - m_pCar->m_lastBulletFireTime > pistol_rate_ms))
		{
            Matrix inverseViewMat;
            pGame->GetViewMatFollowMode(m_pCar, this, nullptr, &inverseViewMat);

			Matrix& rightHand = pGame->m_handPoses[1]; // right hand matrix
			Matrix mat =  inverseViewMat * rightHand;
			Vector pos = mat * Vector(0.0, 0.0, 0.0); // take the translation part
			Vector dir = mat * Vector(0.0, -1.0, 0.0);
			Vector velocity = (dir - pos);
			velocity.Normalize(); // needed only in case the matrices has components other than translation and rotation
			velocity = BALL_SPEED * velocity;
			Bullet* pBullet = new Bullet(pos, velocity, pGame->m_timestamp + 20000, this->m_pCar);
			pGame->PlayResSound(3);
			if (pGame->AddEntity(pBullet))
			{
				m_pCar->m_lastBulletFireTime = pGame->m_timestamp;
			}
			if (pGame->m_network.AddScalarPacket(Command_BulletFire, 0, 0, 0))
			{
				//m_lastBulletFireTime = game.m_timestamp;
			}
		}
	}
    m_bPrevShootButton = m_bShootButton;
}
