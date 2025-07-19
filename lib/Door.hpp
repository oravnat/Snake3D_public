//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once
#include "Entity.hpp"
#include <stdlib.h>

class Door :
	public Entity
{
public:
	Door(const Vector& pos, double radius, Door* pTwin = NULL);
	~Door(void);
public:
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
    virtual bool ReactToCar(SnakeGame& game, Car& car);
private:
	double m_radius;
	Door* m_pTwin;
};
