//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once
#include "Entity.hpp"
#include <vector>

class Model;

class TakableObject :
	public Entity
{
public:
	TakableObject(const Vector& pos, int liveUntil, Model* pModel);
	~TakableObject(void);
public:
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
    virtual bool ReactToCar(SnakeGame& game, Car& car);
private:
	int m_liveUntil;
	Model* m_pModel;
};
