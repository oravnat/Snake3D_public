//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Door.hpp"
#include "Model.hpp"
#include "SnakeGame.hpp"

extern Model g_sphereModel;

Door::Door(const Vector& pos, double radius, Door* pTwin)
{
	m_radius = radius;
	m_pos = pos;
	m_pTwin = pTwin;
	if (pTwin != NULL)
		pTwin->m_pTwin = this;
}

Door::~Door(void)
{
}

bool Door::Think(SnakeGame& game)
{
	return false;
}

bool Door::ReactToCar(SnakeGame& game, Car& car)
{
    bool bAte = car.IsPointInside(m_pos);
    if (bAte)
    {
        Vector diff = m_pos - car.m_pos;
        Vector velocity = car.m_velocity;
        if (velocity.DotProduct(diff) >= 0)
        {
            Vector dir = -velocity;
            //dir.Normalize();
            car.m_pos = m_pTwin->m_pos;
            car.m_pTail->m_tailSize = 0;
            car.m_acceleration = 500.0;
            //dir = -dir;
            /*if (Sphere level(TODO))
            {
                Vector up = game.m_players[i]->m_pos;
                //up.Normalize();
                game.m_players[i]->m_orientation = Matrix::Convert(dir, up);
            }*/
        }
    }
    return false;
}

void Door::Render(SnakeRenderer& renderer)
{
	if (m_radius <= 0)
		return;
	renderer.Color(0, 0, 0, 255);
	renderer.Scale((t_float)m_radius, (t_float)m_radius, (t_float)m_radius);
	g_sphereModel.Render(renderer);
}
