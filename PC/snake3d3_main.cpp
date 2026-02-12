//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public
//
//============================================================================
// Name        : snake3d3_main.cpp
// Author      : Or Avnat
// Version     :
// Copyright   : BSD 2-Clause License
// Description : Snake3D version 3, PC edition main routine
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <math.h>

//#define NETWORK

#ifdef NETWORK
#include "StreamSocket.h"
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN // needed before <windows.h> if using <Winsock2.h>
#include <windows.h>
#include <mmsystem.h>
#else 
// windows constants:
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#endif

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#include <AudioToolbox/AudioToolbox.h>
#else
#include "GL/glew.h"
#include "GL/freeglut.h"
#endif
#include "examples/SnakeGame/SnakeGame.hpp"
//#include "Menu.h"

using namespace std;

int g_mainwin;
SnakeGame* g_snakeGame;
BaseGame* g_game;
BaseGame* g_pausedGame;
SnakeRenderer g_snakeRenderer;

#ifdef NETWORK
// client vars:
float m_lastServerPacketTime = 0.0f;
StreamSocket m_ssock;
bool m_serverConnected = false;
#endif
// external functions:
bool checkGlError(const char* funcName);
#ifdef NETWORK
int RunServer();
#endif

#define M_PI       3.14159265358979323846   // pi

double g_nWidth;
double g_nHeight;

const wchar_t* sounds[] = {
	L"sounds/welcometosnake3d.wav",
	L"sounds/take.wav",
	L"sounds/explosion_deposito.wav",
	L"sounds/shoot.wav",
	L"sounds/rock_crash.wav",
	L"sounds/car_shoot_hit.wav"
};

#ifdef WIN32
void InitSound()
{
}
void DeinitSound()
{
}
void PlaySoundByIndex(int index)
{
    PlaySound(sounds[index], NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}
#elif defined(__APPLE__)
static SystemSoundID soundIDs[6];
void InitSound()
{
    wstring str;
    for (int i = 0; i < 6; i++)
    {
        str = L"/Users/oravnat/Documents/GitHub/Snake3D/";
        str += sounds[i];
        CFStringRef soundPath = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)str.c_str(), str.length() * sizeof(wchar_t), kCFStringEncodingUTF32LE, false);
        CFURLRef soundURL = CFURLCreateWithFileSystemPath(NULL, soundPath, kCFURLPOSIXPathStyle, false);
        OSStatus status = AudioServicesCreateSystemSoundID(soundURL, &soundIDs[i]);
        
        if (status != kAudioServicesNoError)
            std::cerr << "Failed to load sound file." << std::endl;
        CFRelease(soundURL);
        CFRelease(soundPath);
    }
}
void DeinitSound()
{
    for (int i = 0; i < 6; i++)
        AudioServicesDisposeSystemSoundID(soundIDs[i]);
}
void PlaySoundByIndex(int index)
{
    AudioServicesPlaySystemSound(soundIDs[index]);
}
#else
void InitSound()
{
}
void DeinitSound()
{
}
void PlaySoundByIndex(int index)
{
}
#endif


void Client_ExecuteServerCommands(float nowSec)
{
#ifdef NETWORK
	if (m_serverConnected && g_game)
	{
		if (m_lastServerPacketTime == 0.0f || nowSec - m_lastServerPacketTime > 0.1f)
		{
			uint8_t buffer[SEND_BUFFER_SIZE + 50];
			//int bytes = m_sock.ReceivePacket(buffer, SEND_BUFFER_SIZE, 0);
			if (StreamSocket::Select(&m_ssock, 1, 0) == 0)
			{
				int bytes = 0;
				if (g_game->m_nReceiveBufferBytes > 0)
				{
					memcpy(buffer, g_game->m_receiveReaminsBuffer, g_game->m_nReceiveBufferBytes);
					bytes = m_ssock.Receive(buffer + g_game->m_nReceiveBufferBytes, SEND_BUFFER_SIZE);
					bytes += g_game->m_nReceiveBufferBytes;
				}
				else
					bytes = m_ssock.Receive(buffer, SEND_BUFFER_SIZE);
				if (bytes > 0)
					g_game->Execute(buffer, bytes);
				m_lastServerPacketTime = nowSec;
			}
		}
	}
#endif
}

void SampleDisplay(void)
{
	int win = glutGetWindow();

	if (g_game) {
		const GLfloat time = glutGet(GLUT_ELAPSED_TIME) / 1000.f;

		Client_ExecuteServerCommands(time);
#ifdef WIN32
		static bool bKeyRotation = false;
		SnakeGame* p_game = dynamic_cast<SnakeGame*>(g_game);
		if (p_game)
		{
			SnakeGame& game = *p_game;
			GetKeyboardState(g_game->m_keys);
			if (game.m_players[0])
			{
				if (game.m_bGameServer && game.m_bSavePackets)
				{
					if (game.m_players[0]->m_pCar)
						game.m_players[0]->m_pCar->m_rotation = 0.9f;
				}
				else
				{
					Car* pCar = game.m_players[0]->m_pCar;
					if (pCar)
					{
						if (IS_KEY('A'))
						{
							pCar->m_rotation = 1.0;
							bKeyRotation = true;
						}
						else if (IS_KEY('D'))
						{
							pCar->m_rotation = -1.0;
							bKeyRotation = true;
						}
						else if (bKeyRotation)
						{
							pCar->m_rotation = 0.0;
							bKeyRotation = false;
						}
					}
				}
			}
			if (game.m_players[1])
			{
				Car* pCar = game.m_players[1]->m_pCar;
				/*if (IS_KEY('J'))
					m_players[1]->m_rotation = 1.0;
				else if (IS_KEY('L'))
					m_players[1]->m_rotation = -1.0;
				else
					m_players[1]->m_rotation = 0.0;*/
				if (pCar)
				{
					if (!(game.m_bGameServer && game.m_bSavePackets))
						pCar->m_rotation = 0.9f;
				}
			}
		}
#else
        SnakeGame* p_game = dynamic_cast<SnakeGame*>(g_game);
        if (p_game)
        {
            if (p_game->m_players[0])
            {
                Car* pCar = p_game->m_players[0]->m_pCar;
                if (pCar)
                {
                    if (p_game->m_bGameServer && p_game->m_bSavePackets)
                    {
                        pCar->m_rotation = 0.9;
                    }
                }
            }
            if (p_game->m_players[1] && p_game->m_players[1]->m_pCar)
                p_game->m_players[1]->m_pCar->m_rotation = 0.9;
        }
#endif
		g_game->Think(time);
		g_game->Render(g_snakeRenderer);
		checkGlError("Game::Render");
#ifdef NETWORK
		if (m_serverConnected)
		{
			m_ssock.Send(g_game->m_sendBuffer, g_game->m_nSendBufferBytes);
		}
        g_game->ClearSendBuffer();
#endif

		for (int i = 0; i < 6; i++)
		{
			if (g_game->IsSoundPlayed(i))
                PlaySoundByIndex(i);
		}
	}

	glutSwapBuffers();
	glutPostWindowRedisplay(win);
}

void SampleIdle(void)
{
}

void SampleEntry(int state)
{
	int window = glutGetWindow();
	printf("Window %d Entry Callback: %d\n", window, state);
}

void SampleReshape(int nWidth, int nHeight)
{
	g_nWidth = nWidth;
	g_nHeight = nHeight;

	glViewport(0, 0, nWidth, nHeight);

	if (g_game)
		g_game->Resize(nWidth, nHeight);

}

void SampleKeyboard(unsigned char cChar, int nMouseX, int nMouseY)
{
	if (g_game)
	{
		if (cChar == 'p' || cChar == 'P')
		{
			if (g_game->m_bPause)
				g_game->Unpause();
			else
				g_game->Pause();
		}
        /*if (cChar == 'm')
        {
            if (g_pausedGame)
            {
                delete g_game;
                g_game = g_pausedGame;
                g_pausedGame = nullptr;
            }
            else
            {
                MenuUI* p_menuUI = new MenuUI;
                p_menuUI->Init(nullptr);
                p_menuUI->NewGame();
                p_menuUI->Resize(g_game->m_windowWidth, g_game->m_windowHeight);

                g_pausedGame = g_game;
                g_game = p_menuUI;
            }
        }*/

#if !defined(WIN32)
        if (g_snakeGame && g_snakeGame->m_players[0] && g_snakeGame->m_players[0]->m_pCar)
        {
            Car* pCar = g_snakeGame->m_players[0]->m_pCar;
            if (cChar == 'a')
                pCar->m_rotation = 1.0;
            if (cChar == 'd')
                pCar->m_rotation = -1.0;
        }
#endif

	}
}

void KeyUp( unsigned char key, int x, int y )
{
#if !defined(WIN32)
    if (g_snakeGame && g_snakeGame->m_players[0] && g_snakeGame->m_players[0]->m_pCar)
	{
        Car* pCar = g_snakeGame->m_players[0]->m_pCar;
		if (key == 'a')
            pCar->m_rotation = 0.0;
		if (key == 'd')
            pCar->m_rotation = 0.0;
	}
#endif
}

void SampleSpecial(int nSpecial, int nMouseX, int nMouseY)
{
	if (g_game)
	{
		switch (nSpecial)
		{
		case GLUT_KEY_LEFT:
			g_game->m_keys[VK_LEFT] = 1;
			break;
		case GLUT_KEY_UP:
			g_game->m_keys[VK_UP] = 1;
			break;
		case GLUT_KEY_RIGHT:
			g_game->m_keys[VK_RIGHT] = 1;
			break;
		case GLUT_KEY_DOWN:
			g_game->m_keys[VK_DOWN] = 1;
			break;
		case GLUT_KEY_F1:
			g_game->ToggleView();
			break;
		}
	}
}

void SpecialUp( int key, int x, int y )
{
	if (g_game)
	{
		switch (key)
		{
		case GLUT_KEY_LEFT:
			g_game->m_keys[VK_LEFT] = 0;
			break;
		case GLUT_KEY_UP:
			g_game->m_keys[VK_UP] = 0;
			break;
		case GLUT_KEY_RIGHT:
			g_game->m_keys[VK_RIGHT] = 0;
			break;
		case GLUT_KEY_DOWN:
			g_game->m_keys[VK_DOWN] = 0;
			break;
		}
	}
}

void MouseCallback(int button, int state, int sx, int sy)
{
	if (g_game)
	{
		// convert (x, y) to the range [-1, 1]:
		double x = sx, y = sy;
		x = x / g_game->m_windowWidth;
		y = y / g_game->m_windowHeight;
		x = (x - 0.5) * 2.0;
		y = (y - 0.5) * 2.0;
		if (button == 0)
		{
			if (state == GLUT_DOWN)
				g_game->StartClick(x, y);
			else if (state == GLUT_UP)
				g_game->EndClick(x, y);
		}
		if (state == GLUT_UP)
			g_game->EndClick(x, y, button);
	}
}

void MotionCallback(int sx, int sy)
{
	if (g_game)
	{
		// convert (x, y) to the range [-1, 1]:
		double x = sx, y = sy;
		x = x / g_game->m_windowWidth;
		y = y / g_game->m_windowHeight;
		x = (x - 0.5) * 2.0;
		y = (y - 0.5) * 2.0;
		g_game->ClickMove(x, y);
	}
}

bool appInit()
{
	if (g_game)
	{
		delete g_game;
		g_game = NULL;
		g_snakeGame = NULL;
	}
	if (!g_snakeRenderer.Init())
		return false;
    SnakeGame* pGame = new SnakeGame;
	g_snakeGame = pGame;
    pGame->zNear = 0.1f;
    pGame->zFar = 200.0f;
	g_game = pGame;
	g_game->m_screenRotation = 0;
    //char path[512] = "./";
	//char path[512] = "./models/";
    char path[512] = "/Users/oravnat/Documents/GitHub/Snake3D/models/";
	//char path[512] = "/home/or/GitHub/Snake3D/models/";
    
	if (!g_game->Init(path)) {
		delete g_game;
		g_game = NULL;
		return false;
	}

    PlaySoundByIndex(0);

	if (g_game)
		g_game->NewGame();
	return true;
}

void appDeinit()
{
	delete g_game;
}

bool ConnectToServerGame(const char* server)
{
#ifdef NETWORK

	if (!m_ssock.Connect(server, GAME_PORT))
	{
		cout << "Error connecting to " << server << endl;
		return false;
	}
	cout << "OK connected " << server << endl;
	m_ssock.UseExceptions(true);
	ScalarPacket packet;
	packet.cmd = Command_NewPlayer;
	packet.index = 0;
	packet.prop = 0;
	packet.val = 0;
	m_ssock.Send(&packet, sizeof(packet));
	m_serverConnected = true;
	Game::m_nPlayers = 1;
	g_game->m_bGameServer = false;
	g_game->m_bSavePackets = true;
	g_game->NewGame();
	return true;
#else
	return false;
#endif
}

int main(int argc, char** argv)
{
#ifdef NETWORK
    WSADATA wsdata;
    WSAStartup(MAKEWORD(2, 0), &wsdata);

	if (argc > 1 && strcmp(argv[1], "server") == 0)
	{
		return RunServer();
	}
#endif
    
    InitSound();

    glutInit(&argc, argv);

#if defined(__APPLE__)
    // for MacOS to support opengl3:
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_ALPHA);
#else
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
    glutIdleFunc(SampleIdle);

	glutInitWindowPosition(50, 20);
	glutInitWindowSize(1000, 600);
	g_mainwin = glutCreateWindow("Snake3D3");
	glutDisplayFunc(SampleDisplay);
	glutReshapeFunc(SampleReshape);
	glutKeyboardFunc(SampleKeyboard);
	glutKeyboardUpFunc(KeyUp);
	glutSpecialFunc(SampleSpecial);
	glutSpecialUpFunc(SpecialUp);
	glutEntryFunc(SampleEntry);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);

#if !defined(__APPLE__)
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}
	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
#endif
    cout << "glGetString(GL_VERSION): " << glGetString(GL_VERSION) << endl;
    cout << "glGetString(GL_SHADING_LANGUAGE_VERSION): " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    cout << "glGetString(GL_RENDERER): " << glGetString(GL_RENDERER) << endl;

	if (appInit())
		cout << "OK appInit" << endl;
	else
	{
		cout << "ERROR appInit" << endl;
		return -1;
	}
	ConnectToServerGame("127.0.0.1");

	/*
	* Enter the main FreeGLUT processing loop
	*/
	glutMainLoop();
    
    DeinitSound();

	return EXIT_SUCCESS;
}
