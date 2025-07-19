//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Prize.hpp"
#include "SnakeGame.hpp"
#include "SnakeRenderer.hpp"

Prize::Prize(bool bEarthLevel)
{
	m_type = EntityType_Prize;
	m_bEarthLevel = bEarthLevel;
}

Prize::~Prize(void)
{
}

bool Prize::Think(SnakeGame& game)
{
	return false;
}

bool Prize::ReactToCar(SnakeGame& game, Car& car)
{
    if (!game.m_bGameServer)
        return false;
    // Check if the car ate us:
    bool bAte = car.IsPointInside(m_pos);
    if (bAte)
    {
        game.CreatePrize();
        game.Execute(Command_SetIntProperty, car.m_index, CAR_INT_PROPERTY_PRIZE_TAKEN, 0);
        return true;
    }
    return false;
}


void Prize::Render(SnakeRenderer& renderer)
{
	const int N = 10;
	Vertex3f3f a[3 * N];
	if (!m_bEarthLevel)
	{
		Vector from = Vector(0.0, 1.0, 0.0);
		Vector to = m_pos;
		to.Normalize();
		Matrix mat = Matrix::RotateAndMirror(from, to);
        renderer.MultMatrix(mat);
	}
    renderer.Color(255, 255, 0, 255);
	t_float sin45 = (t_float)sin(M_PI / 4);
	t_float cos45 = (t_float)cos(M_PI / 4);
	for (int i = 0; i < N; i++)
	{
		t_float ang1 = (t_float)((i + 1) * M_PI / 5);
		t_float ang0 = (t_float)(i * M_PI / 5);
		t_float ang05 = (t_float)((i + 0.5) * M_PI / 5);
		a[3 * i].pos = Vector(0.5f * cosf(ang1), 0.0f, 0.5f * sinf(ang1));
		a[3 * i].normal = Vector(cos45*cosf(ang1), sin45, cos45 * sinf(ang1));
		a[3 * i + 1].pos = Vector(0.5f * cosf(ang0), 0.0f, 0.5f * sinf(ang0));
		a[3 * i + 1].normal = Vector(cos45 * cosf(ang0), sin45, cos45 * sinf(ang0));
		a[3 * i + 2].pos = Vector(0.0f, 0.5f, 0.0f);
		a[3 * i + 2].normal = Vector(cos45 * cosf(ang05), sin45, cos45 * sinf(ang05));
	}

    renderer.DrawVertex3f3f(a, 3*N);
}

void Prize::Draw2D(SnakeRenderer& renderer)
{
	// The prizes:
	renderer.PointSize(3.0);
	renderer.Color(255, 255, 0, 255);
	renderer.DrawVectors(&m_pos, 1, SNAKE_POINTS);
}

