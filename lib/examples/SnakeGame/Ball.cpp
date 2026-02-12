//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Ball.hpp"
#include "SnakeGame.hpp"
#include <math.h>
#include "defines.hpp"
#include "Model.hpp"

#define REVERSE_KEYS_TIME 10000

extern Model g_carModel;
extern Model g_sphereModel;

const double defaultRadius = 0.5;

Ball::Ball()
{
	Vector pos, velocity;
	BColor color(0, 255, 0);
	m_radius = 2.0;
	New(pos, velocity, color, defaultRadius);
	m_bPassive = false;
	m_type = EntityType_Ball;
	m_shooterIndex = -1;
	m_shrinkSpeed = 0.4;
	m_pOwner = NULL;
}

Ball::Ball(const Vector& pos, const Vector& velocity, const BColor& color, Car* pOwner, double radius)
{
	New(pos, velocity, color, radius);
	m_bPassive = false;
	m_type = EntityType_Ball;
	m_shooterIndex = -1;
	m_shrinkSpeed = 0.4;
	m_pOwner = pOwner;
}

Ball::~Ball(void)
{
}

bool Ball::Think(SnakeGame& game)
{
    double time = game.m_timeDiff;
    
    if (game.m_bGameServer && !m_bPassive)
        game.m_level.FixGravityForces(*this, time);

	m_radius -= m_shrinkSpeed * time;

	m_prevPos = m_pos;
	return (m_radius <= 0);
}

bool Ball::ReactToCar(SnakeGame& game, Car& car)
{
    double time = game.m_timeDiff;
    const int PIECES_TO_MOVE = 7;

    if (!game.m_bGameServer)
        return false;
    if (m_bPassive)
    {
        // Passive ball: check if the car ate us:
        bool bAte = car.IsPointInside(m_pos);
        if (bAte)
        {
            car.m_nBalls++;
            car.m_gunUntil = 0;
            game.PlayResSound(1);
            return true;
        }
        return false;
    }
    // Acrtive ball: check if the ball hit a car:
    bool bAte = car.IsPointInside(m_pos);
    if (bAte)
    {
        // The ball hit an opposite car:
        if (m_shooterIndex != car.m_index)
        {
            //game.CreateCircleWall(m_pos);
            //play Car hit by a ball sound
            game.PlayResSound(5);

            if (car.m_pPlayer)
            {
                car.m_pPlayer->m_keyReverseUntil = game.m_timestamp + REVERSE_KEYS_TIME;
            }
            if (m_pOwner->m_pPlayer)
            {
                m_pOwner->m_pPlayer->m_points += BALL_HIT_PLAYER_POINTS;
            }
            game.m_bUpdateStatus = true;
            return true;
        }
        // Else, this is our ball, if it is small enough, take it back:
        else if (m_radius <= 0.9 * defaultRadius)
        {
            car.m_nBalls++;
            car.m_gunUntil = 0;
            game.PlayResSound(1);
            return true;
        }
    }
    Tail* pTail = car.m_pTail;
    if (!pTail)
        return false;
    // Check if the ball hit the tail:
    bool bDone = false;
    for (int j = 0; j < 4 && !bDone; j += 2)
    {
        int index = pTail->CrossIndex(m_prevPos, m_pos, j);
        if (index != -1)
        {
            Vector n = pTail->m_tail[j][index].normal;
                    
            // Move the tail:
            for (int k = 0; k < PIECES_TO_MOVE; k++)
            {
                t_float moveAmount = (t_float)(8.0 * m_radius * (1.0 - (double)k / PIECES_TO_MOVE));
                for (int side = 0; side < 4; side++)
                {
                    Vector pos = pTail->m_tail[side][(index + k) % MAX_TAIL].pos;
                    pos.Normalize();
                    Vector movDir = pos;
                    if (game.m_level.m_type == LevelType_Simple)
                        movDir = Vector(0.0, 1.0, 0.0);
                    pTail->m_tail[side][(index + k) % MAX_TAIL].pos += moveAmount * movDir;
                }
                if (k != 0)
                {
                    for (int side = 0; side < 4; side++)
                    {
                        Vector pos = pTail->m_tail[side][(index - k + MAX_TAIL) % MAX_TAIL].pos;
                        pos.Normalize();
                        Vector movDir = pos;
                        if (game.m_level.m_type == LevelType_Simple)
                            movDir = Vector(0.0, 1.0, 0.0);
                        pTail->m_tail[side][(index - k + MAX_TAIL) % MAX_TAIL].pos += moveAmount * movDir;
                    }
                }
            }
            m_velocity = m_velocity - 2.0f * n.DotProduct(m_velocity) * n;
            m_pos = m_prevPos + (t_float)time * m_velocity;

            //play sound
            game.PlayResSound(4);

            bDone = true;
            break;
        }
    }

    return false;
}


void Ball::Render(SnakeRenderer& renderer)
{
	if (m_radius <= 0)
		return;
	renderer.Color(m_color.c[0], m_color.c[1], m_color.c[2], m_color.c[3]);
    renderer.Scale((t_float)m_radius, (t_float)m_radius, (t_float)m_radius);
    g_sphereModel.Render(renderer);
}

void Ball::New(const Vector& pos, const Vector& velocity, const BColor& color, double radius)
{
	m_pos = pos;
	m_prevPos = pos;
	m_velocity = velocity;
	m_radius = radius;
	m_color = color;
    m_pos[1] += (t_float)m_radius;
}

bool Ball::SetProperty(int index, int val)
{
	switch (index) {
		case BallProperty_Color:
			if (val == GreenColor)
				m_color = BColor(0, 255, 0);
			break;
		case BallProperty_Passive:
			m_bPassive = val;
			break;
		case BallProperty_ShrinkSpeed:
			m_shrinkSpeed = val / 100.0;
			break;
		default:
			return Entity::SetProperty(index, val);
	}
	return true;
}
