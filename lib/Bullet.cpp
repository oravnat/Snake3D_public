//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Bullet.hpp"
#include "SnakeGame.hpp"
#include "Model.hpp"
#include "Player.hpp"

extern Model g_bulletModel;

Bullet::Bullet(const Vector& pos, const Vector& velocity, int liveUntil, Car* pOwner)
{
	m_pos = pos;
	m_velocity = velocity;
	m_liveUntil = liveUntil;
	m_pOwner = pOwner;
    m_type = EntityType_Bullet;
}

Bullet::~Bullet(void)
{
}

bool Bullet::Think(SnakeGame& game)
{
	for (int i = 0; i < game.m_level.m_shields.size(); i++)
	{
		Vector pos = game.m_level.m_shields[i];
		Vector diff = pos - m_pos;
		if (diff.Length() <= 10 && m_velocity.DotProduct(diff) >= 0.0)
		{
			Vector n = -diff;
			n.Normalize();
			m_velocity -= 2.0f * m_velocity.DotProduct(n) * n;
			game.m_level.m_shieldVisibleUntil[i] = game.m_timestamp + 50;
		}
	}

	return m_liveUntil < game.m_timestamp;
}

bool Bullet::ReactToCar(SnakeGame& game, Car& car)
{
    Vector pos = 0.5f * (car.m_pos + car.m_carFront);
    Vector diff = pos - m_pos;
    if (diff.Length() <= SHIELD_SIZE && m_velocity.DotProduct(diff) >= 0.0)
    {
        Vector n = -diff;
        n.Normalize();
        m_velocity -= 2.0f * m_velocity.DotProduct(n) * n;
        car.m_shieldHp -= 2;
        if (car.m_shieldHp <= 0)
            car.Kill();
        if (&car != m_pOwner)
        {
            if (m_pOwner && m_pOwner->m_pPlayer)
                m_pOwner->m_pPlayer->m_points += SHOT_KILL_POINTS;
            game.m_bUpdateStatus = true;
        }
        car.m_shieldVisibleUntil = game.m_timestamp + 50;
    }
    return false;
}

void Bullet::Render(SnakeRenderer& renderer)
{
	renderer.Scale(0.001f, 0.001f, 0.001f);
	Vector to = m_velocity;
	to.Normalize();
	//Matrix mat = Matrix::RotateAndMirror(Vector(0.0, 1.0, 0.0), to);
    Matrix mat = Matrix::LookAt(to, Vector(0.0, 1.0, 0.0));
	renderer.MultMatrix(mat);
    renderer.RotateX(90);
	renderer.Color(204, 204, 204, 255);
	g_bulletModel.Render(renderer);
}
