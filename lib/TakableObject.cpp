//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "TakableObject.hpp"
#include "SnakeGame.hpp"
#include "Model.hpp"

#define MAX_RUN_TIME 30000

TakableObject::TakableObject(const Vector& pos, int liveUntil, Model* pModel)
{
	m_pos = pos;
	m_velocity = Vector(0.0, 0.0, 0.0);
	m_pModel = pModel;
	m_liveUntil = liveUntil;
}

TakableObject::~TakableObject(void)
{
}

void TakableObject::Render(SnakeRenderer& renderer)
{
	renderer.Scale(0.1f, 0.1f, 0.1f);
	if (SnakeGame::m_pCurrentGame->m_level.m_type != LevelType_Simple)
	{
		Vector from = Vector(0.0, 1.0, 0.0);
		Vector to = m_pos;
		to.Normalize();
		Matrix mat = Matrix::RotateAndMirror(from, to);
		renderer.MultMatrix(mat);
	}
	m_pModel->Render(renderer);
}

bool TakableObject::Think(SnakeGame& game)
{
	return (m_liveUntil < game.m_timestamp);
}

bool TakableObject::ReactToCar(SnakeGame& game, Car& car)
{
    bool bAte = car.IsPointInside(m_pos);
    if (bAte)
    {
        car.m_gunUntil = game.m_timestamp + MAX_RUN_TIME;
        game.PlayResSound(1);
        return true;
    }
    return true;
}
