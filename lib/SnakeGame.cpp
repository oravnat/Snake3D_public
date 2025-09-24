//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "SnakeGame.hpp"
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include "defines.hpp"
#include "MeshReader.hpp"
#include "Model.hpp"
#include "TakableObject.hpp"
//#include "Hand.h"
#include "Ball.hpp"
#include "Prize.hpp"

#include <limits>
#include <algorithm>

#ifdef WIN32
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#define CAMERA_SPEED 50.0
#define NUM_OF_PRIZES 4

using namespace std;

int SnakeGame::m_nPlayers = 1;
SnakeGame* SnakeGame::m_pCurrentGame = NULL;
extern const char* g_path;

const int LEFT = 0;
const int RIGHT = 1;
const int LEFT_AIM = 2;
const int RIGHT_AIM = 3;

const float colorBarX1 = -0.9f;
const float colorBarX2 = -0.2f;
const float colorBarY1 = 0.9f;
const float colorBarY2 = 0.87f;
const float colorDiffY = -0.07f;

Model g_carModel;
Model g_carOldModel;
Model g_sphereModel;
Model g_bulletModel;
Model g_airplaneModel;
Model g_gunModel;
Model g_bombModel;
Model g_pistolModel;
Model g_wheelModel;
BaseGame* g_menuUI;

struct MenuData
{
    double carAcceleration;
};

MenuData g_menuData;

void InitMenu();
bool FontTextOut(SnakeRenderer& renderer, const char* string, float x, float y, float z, float size);
bool InitDefaultFont(const char* path);

/*
Model Coordinates ------------> World Coordinates ------------> Camera Coordinates ----------------> Clip Coordinates
                  model matrix                    view matrix                      projection matrix
model space = object space
camera space = eye space
*/

bool SetModel(MeshReader& reader, Model& model, bool bIgnoreTexture);
bool LoadModel(const string& filename, Model& model, const string& materialFilename = "", bool bIgnoreTexture = false);
bool LoadMaterials(Model& model, MeshReader objReader, istream& ifile);
int LoadTexture(const char* filename, t_float* pUsedWidth, t_float* pUsedHeight);


double Random()
{
	return (rand() % 1000) / 1000.0;
}


// The range of each coordinate is (-1, 1)
struct Button
{
	float x1, y1, x2, y2;
	bool IsInside(float x, float y) { return x >= x1 && x <= x2 && y >= y1 && y <= y2; }
};

Button pauseButton = { 0.85f , 0.85f , 0.95f , 0.95f };

SnakeGame::SnakeGame() : m_pPlane(NULL), m_cameraAngle(0.0),
               m_bShootButton(false), m_bShootButtonPrev(false), m_bBallButton(false), m_bBallButtonPrev(false),
	           m_viewMode(ViewMode_Follow), m_bSplitView(false), m_playerIndex(-1),
	           m_lastNewBulletTime(0), m_bMenuActive(false)
{
	for (int i = 0; i < MAX_PLAYERS; i++)
		m_players[i] = nullptr;
	m_handPoses[RIGHT] = Matrix::RotateX((t_float)(M_PI / 2.0));
	srand(0);
}

SnakeGame::~SnakeGame(void)
{
	//SendCommandToServer(Command_Disconnecting, m_playerIndex);
	//for (int i = 0; i < m_entities; i++)
		//delete m_entities[i];
}

bool SnakeGame::Init(const char* path)
{
	//cout << "Current path: " << current_path() << endl;
	cout << "Game::Init() started" << endl;
	g_path = path;
	bool result = true;
	if (result && path != nullptr)
	{
		result = LoadModels(path);
		if (!result)
			cout << "ERROR LoadModels" << endl;
	}
	CreateSphereModel();
	memset(m_keys, 0, sizeof(m_keys));
	//cone::LoadSplineTensorProduct(23, 13);

	//InitDefaultFont("");

    return result;
}

void SnakeGame::Resize(int width, int height)
{
	BaseGame::Resize(width, height);
	pauseButton.y1 = (float)(pauseButton.y2 - (pauseButton.x2 - pauseButton.x1) * m_AspectRatio);
}


void SnakeGame::NewGame()
{
	ClearEntities();

	m_level.m_type = LevelType_Simple;
	/*// VR shooter tests
	{
		m_level.m_type = LevelType_Shooter;
		//m_viewMode = ViewMode_Keys;
		m_viewMode = ViewMode_Up;
	}/* */
	m_level.NewGame(*this);

    m_network.m_nLevelEntities = m_lastAddedEntityIndex + 1;

	if (!m_bGameServer)
	{
		// this is only a client, so don't init entities, but create one player (us)
		Player* pPlayer = new Player(this);
		m_players[0] = pPlayer;
		pPlayer->m_bLocalCar = true;
	}
	if (m_bGameServer)
	{
		// m_nPlayers is 0 on start when this is a pure server:
		for (int i = 0; i < m_nPlayers; i++)
		{
			Player* pPlayer = new Player(this);
			Car* pCar = new Car(this, pPlayer);
			if (i == 0)
			{
				pCar->SetHullColor(BColor(0, 230, 0));
				// Green color for balls:
				pCar->m_color = BColor(0, 230, 0);
                pPlayer->m_input = PlayerInput_LEFT_HAND;
			}
            if (i == 1)
            {
				pCar->SetHullColor(BColor(230, 0, 0));
                // Red color for balls:
				pCar->m_color = BColor(230, 0, 0);
            }
			AddEntity(pCar);
			if (i == 0)
			{
				m_playerIndex = m_lastAddedEntityIndex;
				pPlayer->m_bLocalCar = true;
			}
			Tail* pTail = new Tail(pCar);
			AddEntity(pTail);
			m_players[i] = pPlayer;
			pPlayer->m_pCar = pCar;
			pPlayer->m_entityIndex = pCar->m_index;
			pPlayer->NewGame();
		}
		for (int i = 0; i < NUM_OF_PRIZES; i++)
		    CreatePrize();

		m_pPlane = new Plane();
        /*Hand* pLeftHand = new Hand(LEFT, &g_wheelModel);
        AddEntity(pLeftHand);
		Hand* pRightHand = new Hand(RIGHT, &g_pistolModel);
		AddEntity(pRightHand);*/
	}
	if (m_pPlane)
		AddEntity(m_pPlane);

	m_lastNewBulletTime = 0;

	m_cameraPos = Vector(0.0f, 20.0f, 0.0f);

	//CustomizeCycle();
}

void SnakeGame::ThinkFrame()
{
	double secPerFrame = m_timeDiff;
    SnakeGame& game = *this; // for the macro IS_KEY to work properly
	for (int i = 0; i < m_nEntities; i++)
	{
		if (m_entities[i])
			m_entities[i]->Advance(secPerFrame);
	}
	if (m_viewMode == ViewMode_Keys)
	{
		/*
		// Move camera:
		if (IS_KEY(VK_UP))
		{
			m_cameraPos[0] += secPerFrame * CAMERA_SPEED * sin(m_cameraAngle);
			m_cameraPos[2] -= secPerFrame * CAMERA_SPEED * cos(m_cameraAngle);
		}
		if (IS_KEY(VK_DOWN))
		{
			m_cameraPos[0] -= secPerFrame * CAMERA_SPEED * sin(m_cameraAngle);
			m_cameraPos[2] += secPerFrame * CAMERA_SPEED * cos(m_cameraAngle);
		}
		if (IS_KEY(VK_RIGHT))
			m_cameraAngle += secPerFrame * ROTATE_SPEED;
		if (IS_KEY(VK_LEFT))
			m_cameraAngle -= secPerFrame * ROTATE_SPEED;
		 */
	}

	for (int i = 0; i < m_nPlayers; i++)
	{
		m_players[i]->Think();
        Car* pCar = m_players[i]->m_pCar;
        if (pCar)
        {
            for (int iEnt = 0; iEnt < m_nEntities; iEnt++)
            {
                if (m_entities[iEnt])
                {
                    if (m_entities[iEnt]->ReactToCar(*this, *pCar))
                    {
                        // Delete entity only if we are the server:
                        if (m_bGameServer)
                        {
                            Execute(Command_DeleteEntity, iEnt, 0, 0);
                        }
                    }
                }
            }
        }
	}

	for (int iEnt = 0; iEnt < m_nEntities; iEnt++)
	{
		if (m_entities[iEnt] != NULL && m_entities[iEnt]->Think(*this))
		{
			// Delete entity only if we are the server:
			if (m_bGameServer)
			{
                Execute(Command_DeleteEntity, iEnt, 0, 0);
			}
		}
	}
	m_level.Think(*this);
}

bool SnakeGame::Think(double nowSeconds)
{
	const int msPerFrame = 1;
	const double secPerFrame = msPerFrame / 1000.0;
	bool bResult = false;

    if (buttons[Buttons_Menu] && !buttonsPrevious[Buttons_Menu])
    {
        //if (!g_menuUI)
            //InitMenu();
        m_bMenuActive = !m_bMenuActive;

        if (m_bMenuActive)
        {
            if (m_players[0] && m_players[0]->m_pCar)
                g_menuData.carAcceleration = m_players[0]->m_pCar->m_acceleration;
        }
        else
        {
            if (m_players[0] && m_players[0]->m_pCar)
                m_players[0]->m_pCar->m_acceleration = g_menuData.carAcceleration;
        }
    }

    if (m_bMenuActive && g_menuUI)
    {
        return g_menuUI->Think(nowSeconds);
    }
    if (m_bPause)
		return false;

	m_pCurrentGame = this;
	// Get keyboard:
    SnakeGame& game = *this; // for the macro IS_KEY to work properly
	if (m_players[0])
	{
		m_players[0]->m_bShootButton = m_bShootButton;
		//m_players[0]->m_bShootButton = true;
	}

    if (m_currentSecs == 0.0)
	{
		m_currentSecs = nowSeconds;
		m_lastUnpauseTime = nowSeconds;
	}
	double time = m_currentSecs;
	m_timeDiff = secPerFrame;

	//if (m_timeDiff > 1.0)
		//m_timeDiff = 1.0;
	m_renderSecs = nowSeconds;
	while (time < nowSeconds)
	{
		m_timestamp = (int)(time * 1000.0);
		ThinkFrame();
		time += secPerFrame;
	}
	m_currentSecs = time;

	if (m_level.m_type == LevelType_Simple)
	{
		if (IS_KEY('H'))
		{
			Vector pos = RandomLocation();

			if (Random() < 0.3)
			{
				TakableObject* pObj = new TakableObject(pos, m_timestamp + 10000, &g_gunModel);
				AddEntity(pObj);
			}
			else
			{
				Ball* pBall = new Ball(pos, Vector(0.0f, 0.0f, 0.0f), BColor(0, 255, 0), NULL);
				pBall->m_shrinkSpeed = 0.2;
				pBall->m_bPassive = true;
				if (m_pPlane != NULL)
					m_pPlane->Order(pos, pBall);
				else
					AddEntity(pBall);
			}
		}
		if (m_bGameServer)
		{
			if (m_lastNewBulletTime == 0 || m_timestamp - m_lastNewBulletTime > NEW_BULLET_RATE)
			{
				if (Random() < 0.4)
				{
					Vector pos = RandomLocation();

					/*if (Random() < 0.3)
					{
						TakableObject* pObj = new TakableObject(pos, m_timestamp + 10000, &g_gunModel);
						AddEntity(pObj);
					}
					else*/
					{
						// Add ball:
                        Execute(Command_NewEntity, FIND_ENTITY_INDEX, 0, EntityType_Ball);
                        Execute(Command_SetVectorProperty, LAST_ADDED_ENTITY, Property_Position, pos);
                        Execute(Command_SetIntProperty, LAST_ADDED_ENTITY, BallProperty_Color, GreenColor);
                        Execute(Command_SetIntProperty, LAST_ADDED_ENTITY, BallProperty_ShrinkSpeed, (int)(0.2f * 100.0f));
                        Execute(Command_SetIntProperty, LAST_ADDED_ENTITY, BallProperty_Passive, true);
					}
				}
				m_lastNewBulletTime = m_timestamp;
			}
		}
	}

	/*m_framesFromLastFps++;
	if (m_lastFpsEstimationTime == 0.0)
	{
		m_lastFpsEstimationTime = nowSeconds;
		m_framesFromLastFps = 0;
	}
	if (nowSeconds - m_lastFpsEstimationTime > 1.0)
	{
		double time = nowSeconds - m_lastFpsEstimationTime;
		m_fps = (int)(m_framesFromLastFps / time);
		m_lastFpsEstimationTime = nowSeconds;
		m_framesFromLastFps = 0;
	}*/

	if (m_level.m_bLevelCompleted) {
		m_level.NextLevel();
		NewGame();
	}

    if (m_bShootButton != m_bShootButtonPrev)
    {
        if (m_bShootButton)
        {
            m_shootPose = m_handPoses[RIGHT];
            // Remove translation part:
            for (int i = 0; i < 3; i++)
                m_shootPose[3][i] = 0.0;
        }
        else
        {
            Matrix m = m_handPoses[RIGHT] * m_shootPose.Transpose();
            for (int i = 0; i < 3; i++)
                m[3][i] = 0.0;
            m_shootPose = m;
        }
    }
    m_bShootButtonPrev = m_bShootButton;
    m_bBallButtonPrev = m_bBallButton;

    return bResult;
}

void SnakeGame::GetViewMatFollowMode(Car* pCar, Player* pPlayer, Matrix* viewMat, Matrix* inverseViewMat)
{
    Vector cameraPos;
    Matrix orient = pPlayer->m_cameraOrientation;
    if (pCar)
        cameraPos = pCar->m_carBack;
    // Go back and up 10 meters:
    cameraPos = cameraPos + (orient * Vector(0.0, 3.0, 2.0));
    Vector up(0.0, 1.0, 0.0);
    up = orient * up;
    //Vector center = cameraPos + (orient * Vector(0.0, -0.5, -1.0));
    //renderer.gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], center[0], center[1], center[2], up[0], up[1], up[2]);
    if (viewMat)
        *viewMat = Matrix::RotateX((t_float)(0.25 * 0.5 * M_PI)) *  orient.Transpose() * Matrix::Translate(-cameraPos[0], -cameraPos[1], -cameraPos[2]);
    if (inverseViewMat)
        *inverseViewMat = Matrix::Translate(cameraPos[0], cameraPos[1], cameraPos[2]) * orient * Matrix::RotateX((t_float)(- 0.25 * 0.5 * M_PI));
}


void SnakeGame::SetCamera(SnakeRenderer& renderer, int player)
{
	Vector cameraPos;
	//double cameraAngle;
	// Set camera location:
	if (m_level.m_type == LevelType_Shooter)
	{
		if (m_viewMode == ViewMode_Up)
		{
			Matrix viewMat = Matrix::RotateX((t_float)(0.5 * M_PI)) * Matrix::Translate(-50.0f, -110.0f, -50.0f);
			renderer.MultMatrix(viewMat, false);
		}
		else
		{
			Matrix viewMat = Matrix::RotateX((t_float)(0.2 * 0.5 * M_PI)) * Matrix::RotateY((t_float)m_cameraAngle) * Matrix::Translate(-m_cameraPos[0], -m_cameraPos[1], -m_cameraPos[2]);
			renderer.MultMatrix(viewMat, false);
		}
	}
	else if (m_level.m_type == LevelType_Customize || m_level.m_type == LevelType_ShowModel)
	{
		if (m_viewMode == ViewMode_Follow)
		{
			renderer.TranslateTo(-m_cameraPos);
		}
		if (m_viewMode == ViewMode_FollowPlane)
		{
			Matrix orient = m_pPlane->m_orientation;
			cameraPos = m_pPlane->m_pos;
			cameraPos[2] += 30.0;
			//cameraPos = cameraPos + (orient * Vector(0.0, 0.0, 80.0));
			//Vector up(0.0, 1.0, 0.0);
			//up = orient * up;
			//Vector center = cameraPos + (orient * Vector(0.0, 0.0, -1.0));
			//renderer.gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], center[0], center[1], center[2], up[0], up[1], up[2]);
			renderer.TranslateTo(-cameraPos);
		}
	}
	else
	{
		Matrix viewMat = Matrix::I();
		switch (m_viewMode)
		{
		case ViewMode_Keys:
			viewMat = Matrix::RotateX((t_float)(0.2 * 0.5 * M_PI)) * Matrix::RotateY((t_float)m_cameraAngle) * Matrix::Translate(-m_cameraPos[0], -m_cameraPos[1], -m_cameraPos[2]);
			break;
		case ViewMode_Follow:
            GetViewMatFollowMode(m_players[player]->m_pCar, m_players[player], &viewMat, nullptr);
		    break;
		case ViewMode_FollowPlane:
			if (m_pPlane)
			{
				Matrix orient = m_pPlane->m_orientation;
				cameraPos = m_pPlane->m_pos;
				cameraPos = cameraPos + (orient * Vector(0.0, 0.0, 80.0)); // go back 80 units
				viewMat = orient.Transpose() * Matrix::Translate(-cameraPos[0], -cameraPos[1], -cameraPos[2]);
			}
			break;
		case ViewMode_Up:
			viewMat = Matrix::RotateX((t_float)(0.5 * M_PI)) *  Matrix::Translate(-50.0, -100.0, -50.0);
			break;
		}
		//renderer.SetModelView(viewMat);
		m_viewMat = viewMat;
		renderer.MultMatrix(viewMat, false);
	}
}

void SnakeGame::Render(SnakeRenderer& renderer, int player, double timeDiffSec)
{
	SetCamera(renderer, player);

	// The SetLights() call should be made after setting the view matrix (e.g. gluLookAt)
	m_level.SetLights(renderer);

	m_level.RenderEarth(renderer);
	// Render shadows (only cars and only on ground for now):
	renderer.ShadowMode(true);
    renderer.PushMatrix();
	m_level.UseShadowMatrix(renderer);
	for (int i = 0; i < m_nPlayers; i++)
	{
		Car* pCar = m_players[i]->m_pCar;
		if (pCar)
		{
			renderer.PushMatrix();
			renderer.TranslateTo(pCar->m_pos);
			pCar->RenderShadow(renderer);
			renderer.PopMatrix();
		}
	}
	if (m_pPlane)
	{
		renderer.PushMatrix();
		renderer.TranslateTo(m_pPlane->m_pos);
		m_pPlane->Render(renderer);
		renderer.PopMatrix();
	}
	m_level.RenderShadow(renderer);
    renderer.PopMatrix();
	renderer.ShadowMode(false);

	m_level.Render(renderer);
    //graphicsEngine.SetShininess(10.0f);

    // Render the entities:
	for (int i = 0; i < m_nEntities; i++)
	{
		if (m_entities[i] != NULL)
		{
			//renderer.SetPosition(m_entities[i]->m_pos);
			renderer.PushMatrix();

			renderer.TranslateTo(m_entities[i]->m_pos + (t_float)timeDiffSec * m_entities[i]->m_velocity);
			m_entities[i]->Render(renderer);
			renderer.PopMatrix();
		}
	}
    //renderer.ResetModelTrasfrom();
    //FontTextOut(renderer, "123", 0.0f, 0.5f, -3.0f, 0.01f);
}

void SnakeGame::Render(SnakeRenderer& renderer)
{
    renderer.BindState();
	renderer.Start3DRendering();

	if (m_bSplitView && m_nPlayers == 2 && m_bGameServer)
	{
		renderer.PushMatrix();
		renderer.SetPerspective(60.0f, (float)(m_AspectRatio / 2.0), zNear, zFar);
		renderer.Viewport(0, 0, m_windowWidth / 2, m_windowHeight);
		renderer.LoadIdentity();
		Render(renderer, 0);
		renderer.PopMatrix();

		renderer.PushMatrix();
		renderer.SetPerspective(60.0f, (float)(m_AspectRatio / 2.0), zNear, zFar);
		renderer.Viewport(m_windowWidth / 2, 0, m_windowWidth / 2, m_windowHeight);
		renderer.LoadIdentity();
		Render(renderer, 1);
		renderer.PopMatrix();

		renderer.Viewport(0, 0, m_windowWidth, m_windowHeight);
	}
	else
	{
		renderer.SetPerspective(60.0f, (float)m_AspectRatio, zNear, zFar);
        //TestDraw(renderer);
		renderer.LoadIdentity();
		Render(renderer, 0, m_renderSecs - m_currentSecs);
	}

	//renderer.Start2DRendering();
	//renderer.RotateZ(m_screenRotation);

	//if (m_level.m_type == LevelType_Simple)
	//{
		// Draw the radar:
		//DrawRadar(renderer);
	//}

/*
	// Draw pause lines:
	if (!m_bPause)
	{
		Vector a[4];
		renderer.Color(0, 0, 0, 128);
		a[0] = Vector(pauseButton.x1, pauseButton.y2, 0.0f);
		a[1] = Vector(pauseButton.x1, pauseButton.y1, 0.0f);
		a[2] = Vector(pauseButton.x2, pauseButton.y2, 0.0f);
		a[3] = Vector(pauseButton.x2, pauseButton.y1, 0.0f);
		renderer.DrawVectors(a, 4, GL_TRIANGLE_STRIP);
		renderer.Color(255, 0, 0, 255);
		a[0] = Vector(pauseButton.x1 + 0.01f, pauseButton.y2, 0.0f);
		a[1] = Vector(pauseButton.x1 + 0.01f, pauseButton.y1, 0.0f);
		a[2] = Vector(pauseButton.x2 - 0.01f, pauseButton.y2, 0.0f);
		a[3] = Vector(pauseButton.x2 - 0.01f, pauseButton.y1, 0.0f);
#if !defined(__APPLE__)
		renderer.LineWidth(3.0f); // not supported on my macos
#endif
		renderer.DrawVectors(a, 4, GL_LINES);
		renderer.LineWidth(1.0f);
	}
	else
	{
		Vector a[4];
		renderer.Color(0, 0, 0, 128);
		a[0] = Vector(pauseButton.x1, pauseButton.y2, 0.0f);
		a[1] = Vector(pauseButton.x1, pauseButton.y1, 0.0f);
		a[2] = Vector(pauseButton.x2, pauseButton.y2, 0.0f);
		a[3] = Vector(pauseButton.x2, pauseButton.y1, 0.0f);
		renderer.DrawVectors(a, 4, GL_TRIANGLE_STRIP);
		renderer.Color(255, 0, 0, 255);
		a[0] = Vector(pauseButton.x1 + 0.01f, pauseButton.y1, 0.0f);
		a[1] = Vector(pauseButton.x2 - 0.01f, 0.5 * (pauseButton.y1 + pauseButton.y2), 0.0f);
		a[2] = Vector(pauseButton.x1 + 0.01f, pauseButton.y2, 0.0f);
		renderer.DrawVectors(a, 3, GL_TRIANGLES);
	}*/

	//if (m_level.m_type == LevelType_Customize)
		//DrawCustomize2D();


	if (m_level.m_type == LevelType_Simple)
	{
		// Draw border line between 2 player views:
		if (m_bSplitView && m_nPlayers == 2 && m_bGameServer)
		{
			Vector a[2];
			renderer.Color(0, 255, 0, 255);
			a[0] = Vector(0.0f, -1.0f, 0.0f);
			a[1] = Vector(0.0f, 1.0f, 0.0f);
			renderer.DrawVectors(a, 2, SNAKE_LINES);
		}
		/*
		// draw input line
		if (m_bHaveLine)
		{
			// ax + by + c = 0
			// y = -1 -> x = (b - c)/a
			graphicsEngine.Color4ub(255, 0, 0, 255);
			//a[0] = Vector((m_b - m_c) / m_a, -1.0f, 0.0f);
			//a[1] = Vector(-(m_b + m_c) / m_a, 1.0f, 0.0f);
			graphicsEngine.DrawVectors(m_lines[0], 2, GL_LINES);
			graphicsEngine.DrawVectors(m_lines[1], 2, GL_LINES);
		}*/
		/*if (m_nNumOfDrawPoints != 0)
		{
			renderer.Color(0, 0, 255, 255);
			renderer.DrawVectors(m_pts, m_nNumOfDrawPoints, GL_LINE_STRIP);
		}*/
	}

	/*
	if (m_bPause)
	{
		graphicsEngine.Color4ub(200, 50, 0, 255);
		if (m_gameStarted)
			Draw3DText(-0.4, 0.8 , "Unpause",0.06);
		Draw3DText(-0.4, 0.6 , "New Game",0.06);
		Draw3DText(-0.4, 0.4 , "Connect to server",0.06);
		Draw3DText(-0.4, 0.2 , "Host a game",0.06);
		Draw3DText(-0.4, 0 , "Hall of Fame",0.06);
		Draw3DText(-0.4, -0.2 , "Rotate", 0.06);
		Draw3DText(-0.4, -0.4 , "Keys", 0.06);
	}
	/* */
}

void SnakeGame::RenderView(SnakeRenderer& renderer)
{
    if (m_bMenuActive && g_menuUI)
    {
        g_menuUI->RenderView(renderer);
        return;
    }

    renderer.BindState();
	Render(renderer, 0, m_renderSecs - m_currentSecs);
	//TestDraw(m_snakeRenderer);
}


void SnakeGame::DrawRadar(SnakeRenderer& renderer, int player, int xpos)
{
	const double RadarDistance = 1.0;
	const int N = 50;
	float rx = RADAR_X; // x coordinate of radar center
	float ry = (float)(1.0f - RADAR_TOP_DISTANCE * m_AspectRatio); // y coordinate of radar center


	VertexColor2f interleavedArray[3 * N];
	Vector ballPoints[MAX_BALLS];
	Vector background[3 * N];

	renderer.PushMatrix();
	renderer.Translate((float)rx, (float)ry, 0.0f);
	renderer.Scale(1.0f, (float)m_AspectRatio, 1.0f); // fix aspect ratio
	renderer.Color(0, 230, 0, 255);
	renderer.Scale(RADAR_SIZE / 2.0f, RADAR_SIZE / 2.0f, 1.0f);

    renderer.PrepareStencil();

	// Draw the radar background (also set stencil region):
	for (int i = 0; i < N; i++)
	{
		float angle = (float)(2.0 * M_PI * i / N);
		background[3 * i + 0] = Vector(cosf(angle), sinf(angle), 0.0f);
		angle = (float)(2.0 * M_PI * (i + 1) / N);
		background[3 * i + 1] = Vector(cosf(angle), sinf(angle), 0.0f);
		background[3 * i + 2] = Vector(0.0f, 0.0f, 0.0f);
	}
    renderer.DrawVectors(background, 3*N, SNAKE_TRIANGLES);

	renderer.PopMatrix();

	renderer.UseStencil();
	renderer.PushMatrix();
	renderer.Translate(rx, ry, 0.0f);
	renderer.Scale(1.0f, (float)m_AspectRatio, 1.0f); // fix aspect ratio

	Car* pCar = m_players[player]->m_pCar;
	if (!pCar)
		return;

	Vector radarCenter = pCar->m_pos;
	renderer.Scale(RADAR_SIZE / 2.0f, RADAR_SIZE / 2.0f, 1.0f);
	renderer.RotateX(90.0f);
	// m_orientation is a unitary matrix, so its transpose is its inverse:
	Matrix revOrient = pCar->m_orientation.Transpose();
	renderer.MultMatrix(revOrient);
	renderer.Scale(2.0f / 200.0f, 2.0f / 200.0f, 2.0f / 200.0f);
	// -(x, y, z) coordinates which will be shown in the center of the radar
	renderer.Translate(-radarCenter[0], -radarCenter[1], -radarCenter[2]);

	for (int i = 0; i < m_nEntities; i++)
	{
		if (m_entities[i] != NULL)
			m_entities[i]->Draw2D(renderer);
	}
	renderer.PopMatrix();

	renderer.PushMatrix();
	renderer.Translate(rx, ry, 0.0f);
	renderer.Scale(1.0f, (float)m_AspectRatio, 1.0f); // fix aspect ratio
	renderer.Scale(RADAR_SIZE / 2.0f, RADAR_SIZE / 2.0f, 1.0f);
	// Draw the second layer of the radar background:
	for (int i = 0; i < N; i++)
	{
		unsigned char alpha = 255 * i / N;
		int index = ((i - (m_timestamp / 50) % N) + N) % N;
		float angle = (float)(2.0 * M_PI * index / N);
		interleavedArray[3 * i].x = cosf(angle);
		interleavedArray[3 * i].y = sinf(angle);
		angle = (float)(2.0 * M_PI * (index + 1) / N);
		interleavedArray[3 * i + 1].x = cosf(angle);
		interleavedArray[3 * i + 1].y = sinf(angle);
		interleavedArray[3 * i + 2].x = 0.0f;
		interleavedArray[3 * i + 2].y = 0.0f;
		for (int j = 0; j < 3; j++)
		{
			interleavedArray[3 * i + j].r = 0;
			interleavedArray[3 * i + j].g = 204;
			interleavedArray[3 * i + j].b = 0;
			interleavedArray[3 * i + j].alpha = alpha;
		}
	}
	renderer.SetBlendData(&interleavedArray[0], 3 * N);
	renderer.DrawArrays(SNAKE_TRIANGLES, 0, 3 * N);
	renderer.PopMatrix();

	renderer.DisableStencil();

	// draw a point for each ball the user has
	renderer.PointSize(3.0);
	renderer.Color(0, 255, 0, 255);
	if (pCar)
	{
		int nBalls = min(pCar->m_nBalls, MAX_BALLS);
		for (int j = 0; j < nBalls; j++)
			ballPoints[j] = Vector(-0.9f + j * 0.04f + player * 1.0f, -0.8f, 0.0f);
		renderer.DrawVectors(ballPoints, nBalls, SNAKE_POINTS);
	}
}

void SnakeGame::DrawRadar(SnakeRenderer& renderer)
{
	/*if (m_bGameServer)
	{
		for (int player = 0; player < m_nPlayers; player++)
			DrawRadar(renderer, player, m_nPlayers - 1 - player);
	}
	else*/
		DrawRadar(renderer, 0, 0);
}

void SnakeGame::TestDraw(SnakeRenderer& renderer)
{
    // Test drawing:
    Vertex3f3f a[3];
    a[0].pos = Vector(-0.5, -0.5, -0.5);
    a[1].pos = Vector(0.5, -0.5, -0.5);
    a[2].pos = Vector(0.5, 0.5, -0.5);
	renderer.Color(255, 0, 0, 255);
    renderer.DrawVertex3f3f(a, 3);
}

void SnakeGame::Unpause()
{
	m_bPause = false;
	m_currentSecs = 0.0;
	m_timestamp = (int)(m_currentSecs * 1000);
	m_gameStarted = true;
	for (int i = 0; i < m_nPlayers; i++)
	{
		if (m_players[i]->m_lives)
			m_players[i]->NewGame();
	}
}

void SnakeGame::ToggleView()
{
	switch (m_viewMode)
	{
	case ViewMode_Keys:
		m_viewMode = ViewMode_Follow;
		break;
	case ViewMode_Follow:
		m_viewMode = ViewMode_FollowPlane;
		break;
	//case ViewMode_1stPerson:
		//break;
	case ViewMode_FollowPlane:
		m_viewMode = ViewMode_Up;
		break;
	case ViewMode_Up:
		m_viewMode = ViewMode_Keys;
		break;
	default:
		m_viewMode = ViewMode_Follow;
		break;
	}

}



Vector SnakeGame::RandomLocation()
{
    return m_level.RandomLocation();
}

bool SnakeGame::CreatePrize()
{
	Vector pos = RandomLocation();

    Execute(Command_NewEntity, FIND_ENTITY_INDEX, 0, EntityType_Prize);
    Execute(Command_SetVectorProperty, LAST_ADDED_ENTITY, Property_Position, pos);

	return true;
}


void SnakeGame::ClearEntities()
{
	BaseGame::ClearEntities();
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		delete m_players[i];
		m_players[i] = NULL;
	}
	m_pPlane = NULL;
}

void SnakeGame::DeletePlayer(int iPlayer)
{
	// TODO: need to change Car.m_player and Player.m_player here
    delete m_players[iPlayer];
	for (int i = iPlayer; i < m_nPlayers - 1; i++)
	{
		m_players[i] = m_players[i + 1];
	}
	m_nPlayers--;
}


void SnakeGame::StartClick(double x, double y)
{
	FixOrientation(x, y);
	if (pauseButton.IsInside((float)x, (float)y))
	{
		if (m_bPause)
			Unpause();
		else
			Pause();
	}
	if (y > 0)
	{
		if (x >= 0.41 && y >= 0.4)
		{
		    /*if (m_bPause)
                Unpause();
		    else
                Pause();*/
        }
		else
			m_bBallButton = true;
	}
	else
	{
		/*if (x < 0)
			m_rotateLeft = 1.0f;
		else
			m_rotateRight = 1.0f;*/
	}
}

void SnakeGame::EndClick(double x, double y)
{
	FixOrientation(x, y);
	//m_rotateRight = 0.0f;
	//m_rotateLeft = 0.0f;
    m_bBallButton = false;
}

void SnakeGame::SetObjectPose(int objIndex, const float poseMatrix[16])
{
    BaseGame::SetObjectPose(objIndex, poseMatrix);
    if (g_menuUI)
        g_menuUI->SetObjectPose(objIndex, poseMatrix);
}

void SnakeGame::SnakeSetButton(int buttonIndex, bool bPushed)
{
	BaseGame::SnakeSetButton(buttonIndex, bPushed);
	if (buttonIndex == Buttons_RightSelect)
		m_bShootButton = bPushed;
    if (buttonIndex == Buttons_RightGrab)
        m_bBallButton = bPushed;
    if (g_menuUI)
        g_menuUI->SnakeSetButton(buttonIndex, bPushed);
}



// Client-Server functions:

const char* commands[] =
{
	"Command_NewPlayer", "Command_NewEntity", "Command_YourIndex", "Command_Fire", "Command_Dead",
	"Command_Disconnecting", "Command_SetIntProperty", "Command_SetVectorProperty",
	"Command_BulletFire", "Command_DeleteEntity", "Command_Pause"
};

bool SnakeGame::Execute(Command cmd, int index, int prop, int val)
{
	if (cmd >= Command_NumOfCommands)
	{
		cerr << "ERROR: cmd >= Command_NumOfCommands!" << endl;
	}
    assert(cmd < Command_NumOfCommands);
    Entity* pEntity = NULL;
    EntityType type;
    Car* pCar;
    Vector velocity;
    Ball* pBall;
    switch (cmd)
    {
        case Command_NewEntity:
            type = (EntityType)val;
            switch (type)
            {
                case EntityType_Prize:
                    pEntity = new Prize(m_level.m_type == LevelType_Simple);
                    break;
                case EntityType_Car:
					// if this is the car that belongs to this client, it will be
					// locally deleted later with Command_YourIndex
                    pCar = new Car(this, nullptr);
					pCar->SetHullColor(BColor(230, 0, 0)); // red color for enemies
					pEntity = pCar;
                    break;
                case EntityType_Ball:
                    pEntity = new Ball;
                    break;
                case EntityType_Tail:
					pCar = (Car*)m_entities[prop];
                    if (pCar == NULL)
                        return false;
                    pEntity = new Tail(pCar);
                    break;
                case EntityType_Plane:
                    m_pPlane = new Plane;
                    pEntity = m_pPlane;
                    break;
                default:
					cerr << "ERROR: unknown type for Command_NewEntity command" << endl;
                    return false;
            }
            if (index == FIND_ENTITY_INDEX)
                AddEntity(pEntity);
            else
                AddEntity(index, pEntity);
            return true;
        case Command_DeleteEntity:
            for (int iPlayer = 0; iPlayer < m_nPlayers; iPlayer++)
            {
                if (m_players[iPlayer]->m_entityIndex == index)
                {
                    DeletePlayer(iPlayer);
                    break;
                }
            }
            delete m_entities[index];
            m_entities[index] = NULL;
            m_lastDeletedEntityIndex = index;
            return true;
        case Command_Fire:
            assert(val < MAX_ENTITIES);
            if (val >= MAX_ENTITIES)
                return false;
            //pPlayer = dynamic_cast<Car*>(m_entities[val]);
            pCar = (Car*)(m_entities[val]);
            assert(pCar != NULL);
            if (pCar == NULL)
                return false;
            velocity = BALL_SPEED * (pCar->m_orientation * Vector(0.0, 0.0, -1.0));
            pBall = new Ball(pCar->m_carFront, velocity, pCar->m_color, pCar);
            pBall->m_shooterIndex = val;
            AddEntity(pBall);
			PlayResSound(3);
            return true;
        case Command_Pause:
            if (val == 1)
                m_bPause = true;
            else
            {
				m_currentSecs = 0;
                Unpause();
            }
            return true;
    }

    if (index == LAST_ADDED_ENTITY)
        index = m_lastAddedEntityIndex;
    if (index >= MAX_ENTITIES)
        return false;

	// TODO: index may be 0 if not used. this shouldn't be a problem because cmd would be something other
	// than Command_SetXXXProperty, but we should verify this again.
    pEntity = m_entities[index];

    switch (cmd)
    {
        case Command_YourIndex:
            m_playerIndex = index;
			pCar = dynamic_cast<Car*>(m_entities[m_playerIndex]);
			if (pCar && m_players[0])
			{
				pCar->m_pPlayer = m_players[0];
				m_players[0]->m_bLocalCar = true;
				m_players[0]->m_pCar = pCar;
			}
            return true;
        case Command_Disconnecting:
            for (int iPlayer = 0; iPlayer < m_nPlayers; iPlayer++)
            {
                if (m_players[iPlayer]->m_entityIndex == index)
                {
                    DeletePlayer(iPlayer);
                    break;
                }
            }
            cout << "client with entity index#" << index << " has disconnected" << endl;
            delete m_entities[index];
            m_entities[index] = NULL;
            return true;
    }
    if (pEntity != NULL)
    {
        switch (cmd)
        {
            case Command_SetIntProperty:
                pEntity->SetProperty(prop, val);
                return true;
        }
    }
    return false;
}

bool SnakeGame::Execute(Command cmd, int index, int prop, const Vector& val)
{
    return BaseGame::Execute(cmd, index, prop, val);
}

// Called on server side when a new client is connected:
int SnakeGame::AddNewPlayer()
{
	int index = -1;
	Player* pPlayer = new Player(this);
	Car* pCar = new Car(this, pPlayer);

	m_players[m_nPlayers] = pPlayer;
	//pCar->SetKeys(m_rightKey2, m_LeftKey2, m_fireKey2);
	//pCar->SetHullColor(m_player2Color);
	//pCar->SetAILevel(m_player2AILevel);
	// Blue color for balls:
	pCar->m_color = BColor(0, 0, 255);
	m_nPlayers++;
	if (AddEntity(pCar))
	{
		index = m_lastAddedEntityIndex;
		pPlayer->NewGame();
        m_network.AddScalarPacket(Command_NewEntity, index, 0, EntityType_Car);
        m_network.AddVectorPacket(Command_SetVectorProperty, index, Property_Position, pCar->m_pos);
        m_network.AddVectorPacket(Command_SetVectorProperty, index, Property_Velocity, pCar->m_velocity);
		pPlayer->m_pCar = pCar;
		pPlayer->m_entityIndex = pCar->m_index;
	}
	Tail* pTail = new Tail(pCar);
	if (AddEntity(pTail))
	{
		int tIndex = m_lastAddedEntityIndex;
        m_network.AddScalarPacket(Command_NewEntity, tIndex, index, EntityType_Tail);
	}
	return index;
}

void SnakeGame::CustomizeCycle()
{
	ClearEntities();
	Player* pPlayer = new Player(this);
	Car* pCar = new Car(this, pPlayer);

	m_level.m_type = LevelType_Customize;
	m_level.NewGame(*this);
	m_players[0] = pPlayer;
	//m_players[0]->SetKeys(m_rightKey1, m_LeftKey1, m_fireKey1);
	//m_players[0]->SetHullColor(m_player1Color);
	pCar->SetHullColor(BColor(0, 230, 0));
	AddEntity(pCar);
	m_cameraPos = Vector(0.0f, 0.0f, 100.0f);
	m_cameraAngle = 0.0f;
	m_nPlayers = 1;
	pPlayer->m_pCar = pCar;
	pPlayer->m_entityIndex = pCar->m_index;
	pPlayer->NewGame();
}

void SnakeGame::ShowModel()
{
	ClearEntities();
	m_level.m_type = LevelType_ShowModel;
	m_level.NewGame(*this);
	m_cameraPos = Vector(0.0f, 0.0f, 100.0f);
	m_cameraAngle = 0.0f;
	m_nPlayers = 0;
	m_pPlane = new Plane();
	if (m_pPlane)
		AddEntity(m_pPlane);
}

void SnakeGame::CreateSphereModel()
{
	const int N_POINTS = 20;
	const int N_CIRCLES = 20;
	int nVertices = 2 * N_POINTS * (N_CIRCLES - 1);
	std::vector<Vertex3f3f> vertices(nVertices);

	int idx = 0;

	Vector x(1.0f, 0.0f, 0.0f);
	Vector y(0.0f, 1.0f, 0.0f);
	Vector z(0.0f, 0.0f, 1.0f);

	for (int j = 0; j < N_CIRCLES - 1; j++)
	{
		float r = (float)sin(M_PI * j / (N_CIRCLES - 1));
		float r2 = (float)sin(M_PI * (j + 1) / (N_CIRCLES - 1));
		Vector v = (float)cos(M_PI * j / (N_CIRCLES - 1)) * x;
		Vector v2 = (float)cos(M_PI * (j + 1) / (N_CIRCLES - 1)) * x;
		for (int i = 0; i < N_POINTS; i++, idx += 2)
		{
			float ang = (float)((2.0 * M_PI) * i / (N_POINTS - 1));
			float c1 = r * cosf(ang);
			float c2 = r * sinf(ang);
			Vector p = v + c1 * y + c2 * z;
			vertices[idx].pos = p;
			vertices[idx].normal = p;
			c1 = r2 * cosf(ang);
			c2 = r2 * sinf(ang);
			p = v2 + c1 * y + c2 * z;
			vertices[idx + 1].pos = p;
			vertices[idx + 1].normal = p;
		}
	}
	g_sphereModel.m_vertex3f3f.swap(vertices);
	g_sphereModel.mode = SNAKE_TRIANGLE_STRIP;
}

bool SnakeGame::LoadModel(int index, const char* objContent, int bytes, const char* matContent, int matBytes)
{
    /*if (index >= 0 && index <= 4)
	{
    	MeshReader reader;
    	string str(objContent, bytes);
		istringstream stream(str);
    	reader.loadObj(stream);

        if (index == 0) {
            InitModelMaterials();
            SetModel(reader, g_carModel, false);
        }
		else if (index == 1) {
            SetModel(reader, g_bulletModel, false);
        }
        else if (index == 2) {
            //SetModel(reader, g_carModel);
            SetModel(reader, g_carOldModel, false);
        }
		else if (index == 3) {
			SetModel(reader, g_airplaneModel, false);
			if (matContent) {
                string matstr(matContent, matBytes);
                istringstream matstream(matstr);
                return LoadMaterials(g_airplaneModel, reader, matstream);
            }
		}
        else if (index == 4) {
            SetModel(reader, g_pistolModel, true);
        }

    }*/
    // Also create the sphere model when index == 0
	if (index == 0)
	{
		// TODO: this cause a crash!
		CreateSphereModel();
	}
    return true;
}

/*
#if defined(ANDROID_NDK) || defined(__ANDROID_API__)
#include <asset_manager.h>
#include <asset_manager_jni.h>
*/

bool SnakeGame::LoadModels(const char* path)
{
	/*InitModelMaterials();

	if (!::LoadModel("car_model.obj", g_carModel))
	//if (!::LoadModel(sPath + "f16.obj", g_carModel, sPath + "f16.mtl"))
	//if (!::LoadModel(sPath + "plane.obj", g_carModel))
		return false;
	if (!::LoadModel("bullet.obj", g_bulletModel))
		return false;
	if (!::LoadModel("f16_model.obj", g_airplaneModel, "f16_materials.mtl"))
		return false;
	::LoadModel("pistol.obj", g_pistolModel, "", true);
    ::LoadModel("wheel.obj", g_wheelModel);
	//g_pistolModel.m_texture = LoadTexture("Pistol_obj", NULL, NULL);
	//m_level.m_Texture.m_textHandle = g_pistolModel.m_texture.m_textHandle;

	//if (!::LoadModel(sPath + "bomb.txt", g_airplaneModel))
		//return false;
	//g_bombModel
	/*
	Vector mins, maxs;
	g_airplaneModel.BoundBoxFromArray(mins, maxs);
	mins = { -5.83629990, 0.000000000, -5.98573017 }	float[3]
	maxs = 0x0077f258 {2.08682990, 6.51969004, 5.77042007}	float[3]
	*/

	return true;
}


void SnakeGame::InitModelMaterials()
{
	// car materials:
	int n = 3;
	g_carModel.m_materials.resize(n);
	// Hull material:
	//pMaterials[0].m_dColor = BColor(200, 200, 200);
	g_carModel.m_materials[0].m_dColor = BColor(0, 255, 0);
	// Chassis material:
	g_carModel.m_materials[1].m_dColor = BColor(128, 128, 128); // default Chassis color
	//pMaterials[1].m_dColor = BColor(255, 0, 0);
	// Window material:
	g_carModel.m_materials[2].m_dColor = BColor(0, 0, 0); // default window color
	//pMaterials[2].m_dColor = BColor(0, 0, 255);
}
