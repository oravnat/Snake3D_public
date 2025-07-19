//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Entity.hpp"
#include "Matrix.hpp"

class Plane : public Entity
{
public:
	Plane(void);
	~Plane(void);
public:
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
	virtual void Draw2D(SnakeRenderer& renderer);
	virtual bool SetProperty(int index, const Vector& val);
public:
	void Order(const Vector& dest, Entity* pNewEntity);
public:
	// m_orientation == I means the plane is towrad the negative z
	Matrix m_orientation;
	double m_speed;
	int m_target;
	int m_lastTargetChange;
	Vector m_orderDest;
	Entity* m_pOrderEntity;
};
