//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Entity.hpp"
#include "Matrix.hpp"
#include "Tail.hpp"

#define CAR_INT_PROPERTY_PLAYER 0
#define CAR_INT_PROPERTY_PRIZE_TAKEN 1
#define CAR_INT_PROPERTY_COMMAND 2
#define CAR_INT_PROPERTY_Y_ROTATION 3

enum CarCommand
{
	CarCommand_Killed
};

class SnakeGame;
class Player;

class Wheel : public Entity
{
	void Render(SnakeRenderer& renderer) {}
	bool Think(SnakeGame& game) {return false;}
};

class Car : public Entity
{
public:
	Car(SnakeGame* pGame, Player* pPlayer);
	~Car(void);
public:
	virtual void Advance(double timediff);
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
	virtual void Draw2D(SnakeRenderer& renderer);
	virtual bool SetProperty(int index, int val);
	virtual bool SetProperty(int index, const Vector& val);
    virtual bool ReactToCar(SnakeGame& game, Car& car);
public:
	void Kill();
	void RenderShadow(SnakeRenderer& renderer);
	void SetHullColor(BColor color);
    bool IsPointInside(const Vector& p);
private:
	void RenderCar(SnakeRenderer& renderer);
	void RenderShield(SnakeRenderer& renderer, bool bShadow = false);
	/*void RenderGun();*/
public:
    SnakeGame* m_pGame;
	// Angles 
	double m_yawAngle; // yaw in radians (angle around the local y axis)
	double m_pitch; // in radians (angle around the local x axis)
	//double m_roll;
	Tail* m_pTail;
	int m_tailSizeWhenDead;
	Vector m_carBack, m_carFront;
	Vector m_prevBack, m_prevFront;
	double m_speed;
	double m_acceleration;
	int m_lastFireTime;
	int m_deadUntil;
	int m_deadTime;
	// Ball color:
	BColor m_color;
	// Car hull color:
	BColor m_hullColor;
	// m_orientation == I means the plane is toward the negative z axis
	Matrix m_orientation;
	bool bFirst;
	int m_nBalls;
	int m_gunUntil;
	int m_lastBulletFireTime;
	int m_shieldHp;
	float m_shieldAlpha;
	int m_shieldVisibleUntil;
	int m_undeadUntil;
	Vector m_drawVector;
	float m_airRejectionConstant;
	float m_groundFrictionConstant;
	bool m_bLaser;
	Vector m_laserTarget;
	bool m_bOriented;
	float m_rotation; // [-1, 1], positive is left, negtive is right
	Wheel m_frontWheel;
	//int m_iPlayer; // -1 for no player
	Player* m_pPlayer; // may be null
};
