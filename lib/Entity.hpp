//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Vector.hpp"
#include "SnakeRenderer.hpp"

class SnakeGame;
class Car;

enum EntityType
{
	EntityType_Unknown, EntityType_Car, EntityType_Plane, EntityType_Ball, EntityType_Prize, EntityType_Wall,
	EntityType_Tail, EntityType_Hand, EntityType_Bullet, EntityType_Count

};

enum Property
{
	Property_Position, Property_Velocity
};

class Entity
{
public:
	Entity();
	virtual ~Entity();
public:
	virtual void Advance(double timediff);
	virtual bool Think(SnakeGame& game) = 0;
	virtual void Render(SnakeRenderer& renderer) = 0;
	virtual void Draw2D(SnakeRenderer& renderer);
	virtual bool SetProperty(int index, int val) { return false; }
	// index 0 is position, index 1 is velocity:
	virtual bool SetProperty(int index, const Vector& val);
	// will be used as double dispath call:
	virtual bool CollideWith(Entity* pOther) { return false; }
    virtual bool ReactToCar(SnakeGame& game, Car& car) { return false; }
public:
	Vector m_pos;
	Vector m_velocity;
	EntityType m_type;
	int m_index; // entity index in game.m_entities[] array
};
