//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once
#include "Entity.hpp"


class Prize : public Entity
{
public:
	Prize(bool bEarthLevel);
	~Prize(void);
public:
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
	virtual void Draw2D(SnakeRenderer& renderer);
    virtual bool ReactToCar(SnakeGame& game, Car& car);
private:
	bool m_bEarthLevel;
};
