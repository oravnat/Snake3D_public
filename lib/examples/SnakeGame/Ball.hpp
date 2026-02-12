//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Entity.hpp"

class SnakeGame;
class Car;

enum BallProperty
{
	BallProperty_Color = 10, BallProperty_ShrinkSpeed, BallProperty_Passive
};

#define GreenColor 1


class Ball : public Entity
{
public:
	Ball();
	Ball(const Vector& pos, const Vector& velocity, const BColor& color, Car* pOwner, double radius = 0.5);
	~Ball(void);
public:
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
	void New(const Vector& pos, const Vector& velocity, const BColor& color, double radius = 2.0);
	virtual bool SetProperty(int index, int val);
    virtual bool ReactToCar(SnakeGame& game, Car& car);
public:
	Vector m_prevPos;
	double m_radius;
	BColor m_color;
	int m_shooterIndex;
	double m_shrinkSpeed;
	bool m_bPassive;
	Car* m_pOwner;
};
