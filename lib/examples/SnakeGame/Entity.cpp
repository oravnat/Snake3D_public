//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Entity.hpp"

using namespace std;

const char* strTypes[] = {"EntityType_Unknown", "EntityType_Car", "EntityType_Plane", "EntityType_Ball", "EntityType_Prize", "EntityType_Wall"};

Entity::Entity() : m_type(EntityType_Unknown)
{
	m_index = -1;
}

Entity::~Entity()
{
}

void Entity::Advance(double timediff)
{
	m_pos += (t_float)timediff * m_velocity;
}

void Entity::Draw2D(SnakeRenderer& renderer)
{

}

bool Entity::SetProperty(int index, const Vector& val)
{
	switch (index) 
	{
		case Property_Position:
			m_pos = val;
			//cout << "Entity #" << m_index << " (" << strTypes[m_type] << ") has been moved to (" << val[0] << ", " << val[1] << ", " << val[2] << ")" << endl;
			break;
		case Property_Velocity:
			m_velocity = val;
			break;
		default:
			return false;
	}
	return true;
}
