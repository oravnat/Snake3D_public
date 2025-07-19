//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once
#include "Entity.hpp"
#include <vector>

class SnakeGame;
class Car;

enum LevelType
{
	LevelType_Simple, LevelType_Gravity, LevelType_Spider, LevelType_Customize, LevelType_ShowModel, LevelType_Sphere, LevelType_Shooter
};

class Level : public Entity
{
public:
	Level(void);
	~Level(void);
public:
	void LoadEarthLevel(SnakeGame& game);
	void NewGame(SnakeGame& game);
	Vector RandomLocation();
	void FixCarForces(Car& car);
	void FixGravityForces(Entity& ent, double time);
	void SetLights(SnakeRenderer& renderer);
	void UseShadowMatrix(SnakeRenderer& renderer);
	void RenderEarth(SnakeRenderer& renderer);
	void RenderShadow(SnakeRenderer& renderer);
	void NextLevel();
public:
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
public:
	// The gravity constant:
	double m_g;
	double m_sphereRadius;
	float m_gridMaxX;
	float m_gridMaxZ;
	std::vector<Vector> m_terrain;
	std::vector<Vector> m_shields;
	std::vector<int> m_shieldVisibleUntil;
    SnakeGame* m_pGame;
	bool m_bSunMove;
	float m_sunCirclesPerSecond;
	BColor m_lampColor;
	LevelType m_type;
	Vector m_lightPos;
	bool m_bNight;
	//uint8* m_pBackgroundIndex;
	uint8_t m_backgroundIndex[2][128][128];
	int m_backgroundWidth;
	int m_backgroundHeight;
	int m_iCurrentBackgound;
	int m_iLevel;
	int m_pointsFotNextLevel;
	Texture m_Texture;
	bool m_bLevelCompleted;
};
