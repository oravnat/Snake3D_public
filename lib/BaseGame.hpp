//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Entity.hpp"
#include <vector>

#define MAX_ENTITIES 100

const int MAX_DRAW_POINTS = 100;
const int SEND_BUFFER_SIZE = 4*1024;

#define LAST_ADDED_ENTITY 30000
#define FIND_ENTITY_INDEX 30001

class Model;

enum ViewMode
{
	ViewMode_Keys, ViewMode_Follow, ViewMode_1stPerson, ViewMode_FollowPlane, ViewMode_Up
};

enum Command
{
	Command_NewPlayer, Command_NewEntity, Command_YourIndex, Command_Fire, Command_Dead,
	Command_Disconnecting, Command_SetIntProperty, Command_SetVectorProperty,
	Command_BulletFire, Command_DeleteEntity, Command_Pause, Command_NumOfCommands
};

// Command_SetFloatProperty should not be used, instead Command_SetIntProperty with manually casting should be used.

enum GameMode
{
	GameMode_Simple, GameMode_TimeGame, GameMode_Levels
};

enum Buttons { Buttons_LeftGrab, Buttons_RightGrab, Buttons_LeftSelect, Buttons_RightSelect, Buttons_LeftAdditional, Buttons_RightAdditional, Buttons_Menu, Buttons_NumOfButtons };

//8 bytes
struct ScalarPacket
{
	uint16_t cmd;
	uint16_t index;
	uint16_t prop;
	uint16_t val;
};

// 20 bytes
struct VectorPacket
{
	uint16_t cmd;
	uint16_t index;
	uint16_t prop;
    uint16_t add;
	float vals[3];
};

class BaseGame
{
public:
	BaseGame();
	virtual ~BaseGame(void) {};
public:
	virtual bool Init(const char* path) = 0;
	virtual void NewGame() = 0;
	virtual bool Think(double now = 0.0) = 0;
	virtual void Resize(int width, int height);
	virtual void RenderView(SnakeRenderer& renderer) = 0;
	virtual void Render(SnakeRenderer& renderer) = 0;
	bool PlayResSound(int iSoundIndex);
	bool IsSoundPlayed(int iSoundIndex);
	Entity* GetEntity(uint32_t index);
	bool AddEntity(Entity* pEntity);
	bool AddEntity(int index, Entity* pEntity);
	void Pause();
	virtual void Unpause() { }
	virtual void ToggleView() { }
	virtual void ClearEntities();
	void FixOrientation(double& x, double& y);
	virtual void StartClick(double x, double y) {} // x and y ranges are [-1, 1]
	virtual void ClickMove(double x, double y) {} // x and y ranges are [-1, 1]
	virtual void EndClick(double x, double y) {} // x and y ranges are [-1, 1]
	virtual void EndClick(double x, double y, int button) {} // x and y ranges are [-1, 1]
	virtual void SetObjectPose(int objIndex, const float poseMatrix[16])
    {
        if (objIndex >= 0 && objIndex < 4)
        {
            m_handPoses[objIndex] = poseMatrix;
            bHandValid[objIndex] = true;
        }
    }

    virtual void SnakeSetButton(int buttonIndex, bool bPushed)
	{ 
		if (buttonIndex >= 0 && buttonIndex < Buttons_NumOfButtons)
			buttons[buttonIndex] = bPushed;
	}
    virtual bool Execute(Command cmd, int index, int prop, int val) { return false; }
    virtual bool Execute(Command cmd, int index, int prop, const Vector& val);
    virtual int OnAddNewPlayer() { return -1; }
    void AfterThink()
    {
        for (int i = 0; i < Buttons_NumOfButtons; i++)
            buttonsPrevious[i] = buttons[i];
        for (bool & bValid : bHandValid)
            bValid = false;
    }
protected:
	void Unpause(double now);
public:
	int m_nEntities;
	Entity* m_entities[MAX_ENTITIES];
	static const int MAX_SOUND_FILES = 10;
	bool aPlays[MAX_SOUND_FILES];
	int m_width, m_height;
	double m_AspectRatio;
	float m_screenRotation;
	int m_windowWidth, m_windowHeight;
	int m_timestamp; // milliseconds, for think methods
	int m_lastDeletedEntityIndex;
	int m_lastAddedEntityIndex;
	//double m_timestampd; // seconds
	double m_timeDiff; // seconds
	double m_lastUnpauseTime;
	double m_currentSecs; // seconds
	double m_renderSecs; // seconds

	// user inputs (xr):
	bool buttons[Buttons_NumOfButtons];
	bool buttonsPrevious[Buttons_NumOfButtons];
    // m_handPoses[0] is left, m_handPoses[1] is right, [2] is left aim, [3] is right aim
    Matrix m_handPoses[4];
    bool bHandValid[4];
public:
    float zNear, zFar;

	uint8_t m_keys[256]; // keys from outside
	// true if this is the game server, false if this is only a client
	// in case of one player game, the player is also the server:
	bool m_bGameServer;
	bool m_bSavePackets;
	bool m_bPause;
	bool m_gameStarted;
};


class NetworkBuffer
{
public:
    NetworkBuffer();
	~NetworkBuffer(void) {};

// Client-Server functions:
public:
    bool Execute(BaseGame* pGame, const uint8_t* pBuffer, int size);
	bool NewClientBuffer(BaseGame* pGame, uint8_t* pBuffer, int& bytes);
	bool AddPacketRaw(void* pData, size_t size);
	bool AddScalarPacket(Command cmd, uint16_t index, uint16_t prop, uint16_t val);
	bool AddVectorPacket(Command cmd, uint16_t index, uint16_t prop, const Vector& val);
	void ClearSendBuffer();
public:
	uint8_t m_sendBuffer[SEND_BUFFER_SIZE];
	int m_nSendBufferBytes;
	uint8_t m_receiveReaminsBuffer[50]; // used for incomplete data packet (size must be only as the biggest packet size)
	int m_nReceiveBufferBytes;
    bool m_bEnabled;

public:
	int m_nLevelEntities;
};
