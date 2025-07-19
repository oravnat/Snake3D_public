//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Entity.hpp"
#include "BaseGame.hpp"
#include "Plane.hpp"
#include "Level.hpp"
#include "Player.hpp"

double Random();

class SnakeGame : public BaseGame
{
public:
    SnakeGame();
	virtual ~SnakeGame(void);
public:
	virtual bool Init(const char* path);
	virtual void NewGame();
	virtual bool Think(double now = 0.0);
	void ThinkFrame();
	virtual void Resize(int width, int height);
	void Render(SnakeRenderer& renderer, int player, double timeDiffSec = 0.0);
    virtual void Render(SnakeRenderer& renderer);
	virtual void RenderView(SnakeRenderer& renderer);
	bool CreatePrize();
	virtual void StartClick(double x, double y); // x and y ranges are [-1, 1]
	//virtual void ClickMove(double x, double y); // x and y ranges are [-1, 1]
	virtual void EndClick(double x, double y); // x and y ranges are [-1, 1]
	virtual void SetObjectPose(int objIndex, const float poseMatrix[16]);
	virtual void SnakeSetButton(int buttonIndex, bool bPushed);

	Vector RandomLocation();
	bool LoadModel(int index, const char* objContent, int bytes, const char* matContent, int matBytes);
	void Unpause();
	void ToggleView();
    void GetViewMatFollowMode(Car* pCar, Player* pPlayer, Matrix* viewMat, Matrix* inverseViewMat);
private:
    void ClearEntities();
	void DrawRadar(SnakeRenderer& renderer);
	void DrawRadar(SnakeRenderer& renderer, int player, int xpos);
	void TestDraw(SnakeRenderer& renderer);
	bool LoadModels(const char* path);
	void CreateSphereModel();
	void InitModelMaterials();
	void SetCamera(SnakeRenderer& renderer, int player);

    // Client-Server functions:
public:
	virtual bool Execute(Command cmd, int index, int prop, int val);
    virtual bool Execute(Command cmd, int index, int prop, const Vector& val);
	virtual int AddNewPlayer();

	void CustomizeCycle();
	void ShowModel();
	//void SetOSInterface(OSInterface* pOSInterface);
	void DeletePlayer(int iPlayer);
public:
	Vector m_cameraPos;
	double m_cameraAngle;
	bool m_bSplitView;
	bool m_bUpdateStatus;
	int m_lastNewBulletTime;
	int m_playerIndex;
	Plane* m_pPlane;
	Level m_level;
	/*
	BColor m_player1Color, m_player2Color;
	double m_lastFpsEstimationTime;
	int m_framesFromLastFps;
	double m_a, m_b, m_c;
	Vector m_lines[2][2];
	bool m_bHaveLine;
	*/
public:
    static int m_nPlayers;
    static SnakeGame* m_pCurrentGame;
    NetworkBuffer m_network;
	Player* m_players[MAX_PLAYERS];
	ViewMode m_viewMode;
	Matrix m_viewMat;
    bool m_bMenuActive;
	// user inputs:
    bool m_bShootButton;
	bool m_bShootButtonPrev;
    bool m_bBallButton;
    bool m_bBallButtonPrev;
    Matrix m_shootPose;
};
