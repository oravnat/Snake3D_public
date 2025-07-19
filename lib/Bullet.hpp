//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once
#include "Entity.hpp"

class Car;
class Player;

class Bullet :
	public Entity
{
public:
	Bullet(const Vector& pos, const Vector& velocity, int liveUntil, Car* pOwner);
	~Bullet(void);
public:
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
    virtual bool ReactToCar(SnakeGame& game, Car& car);
private:
	int m_liveUntil;
	Car* m_pOwner;
};
