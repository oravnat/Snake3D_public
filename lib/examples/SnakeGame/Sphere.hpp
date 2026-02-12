//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Entity.hpp"
#include "Model.hpp"
#include "defines.hpp"

// A sphere surface the players can walk on inside.
class Sphere :
	public Entity
{
public:
	Sphere(double radius = SPHERE_RADIUS);
	~Sphere(void);
public:
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
public:
	void SetColor(const BColor& color);
private:
	double m_radius;
	Model* m_pModels;
	BColor m_color;
	int m_nModels;
};
