//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Entity.hpp"
#include "Matrix.hpp"
#include "Car.hpp"
#include "Tail.hpp"

enum PlayerInputMethod
{
	PlayerInput_NONE, PlayerInput_WASD, PlayerInput_DIR_KEYS, PlayerInput_RIGHT_HAND, PlayerInput_LEFT_HAND
};

class Player
{
public:
	// pGame must not be null!
    Player(SnakeGame* pGame) : pGame(pGame), m_entityIndex(-1), m_pCar(nullptr), m_bLocalCar(false), m_input(PlayerInput_NONE),
                          m_bShootButton(false), m_bPrevShootButton(false), m_lives(0), m_keyReverseUntil(0)
	{
		m_money = 0;
		m_points = 0;
		//m_player = -1;
	}
public:
	void NextLife();
	void NewGame();
	void Think();

public:
	int m_entityIndex;
	Car* m_pCar; // may be null
public:
    SnakeGame* pGame;
	//double m_cameraAngle;
	int m_lives;
	//int m_player;
	int m_keyReverseUntil;
	int m_points;
	int m_money;
	bool m_bLocalCar; // in a server multiplayer game: true if this is the car of the local client
	Matrix m_cameraOrientation;
	bool m_bShootButton;
    bool m_bPrevShootButton;
    PlayerInputMethod m_input;
};
