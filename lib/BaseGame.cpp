//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "BaseGame.hpp"
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <cassert>
#include <iostream>
#include <limits>

using namespace std;

const int LEFT = 0;
const int RIGHT = 1;
const int LEFT_AIM = 2;
const int RIGHT_AIM = 3;

BaseGame::BaseGame() : zNear(0.1f), zFar(200.0f), m_screenRotation(0.0), buttons(), buttonsPrevious(), bHandValid()
{
	m_bPause = false;
	m_lastDeletedEntityIndex = 0;
	m_nEntities = 0;
	for (int i = 0; i < MAX_ENTITIES; i++)
		m_entities[i] = NULL;
	m_bGameServer = true;
	m_bSavePackets = false;
	m_gameStarted = false;
	m_currentSecs = 0.0;
	m_lastUnpauseTime = 0.0;
	memset(m_keys, 0, sizeof(m_keys));
	for (int i = 0; i < MAX_SOUND_FILES; i++)
		aPlays[i] = false;
    m_handPoses[LEFT] = Matrix::I(); // left hand
    m_handPoses[RIGHT] = Matrix::I(); // right hand
    m_handPoses[LEFT_AIM] = Matrix::I(); // left hand aim
    m_handPoses[RIGHT_AIM] = Matrix::I(); // right hand aim
    srand(0);
}

void BaseGame::Resize(int width, int height)
{
    double aspectRatio = (double)width / (double)height;
    m_windowWidth = width;
    m_windowHeight = height;
    m_AspectRatio = aspectRatio;
}


void BaseGame::Pause()
{
	m_bPause = true;
	//if (!m_bGameServer)
		//AddEvent(Command_Pause, 0, 0, 1);
}

void BaseGame::Unpause(double now)
{
	m_currentSecs = now;
	m_lastUnpauseTime = now;
	Unpause();
	//if (!m_bGameServer)
		//AddEvent(Command_Pause, 0, 0, 0);
}


bool BaseGame::AddEntity(int index, Entity* pEntity)
{
	if (m_entities[index] == NULL)
	{
		m_entities[index] = pEntity;
		pEntity->m_index = index;
		if (m_nEntities <= index)
			m_nEntities = index + 1;
		m_lastAddedEntityIndex = index;
		return true;
	}
	m_lastAddedEntityIndex = index;
	return false;
}

bool BaseGame::AddEntity(Entity* pEntity)
{
	if (m_entities[m_lastDeletedEntityIndex] == NULL)
		return AddEntity(m_lastDeletedEntityIndex, pEntity);
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		if (m_entities[i] == NULL)
			return AddEntity(i, pEntity);
	}
	return false;
}

Entity* BaseGame::GetEntity(uint32_t index)
{
	if (index >= MAX_ENTITIES)
		return NULL;

	return m_entities[index];
}

void BaseGame::ClearEntities()
{
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		delete m_entities[i];
		m_entities[i] = NULL;
	}
	m_nEntities = 0;
	m_lastDeletedEntityIndex = 0;
}


void BaseGame::FixOrientation(double& x, double& y)
{
	y = -y;
	if (m_screenRotation == 90.0)
	{
		double t = x;
		x = y;
		y = -t;
	}
	else if (m_screenRotation == 180.0)
	{
		x = -x;
		y = -y;
	}
	else if (m_screenRotation == 270.0)
	{
		double t = x;
		x = -y;
		y = t;
	}
}

bool BaseGame::PlayResSound(int iSoundIndex)
{
	if (iSoundIndex >= 0 && iSoundIndex < MAX_SOUND_FILES)
	{
		aPlays[iSoundIndex] = true;
		return true;
	}
	return false;
}

bool BaseGame::IsSoundPlayed(int iSoundIndex)
{
	bool result = false;
	if (iSoundIndex >= 0 && iSoundIndex < MAX_SOUND_FILES)
	{
		result = aPlays[iSoundIndex];
		aPlays[iSoundIndex] = false;
	}
	return result;
}

bool BaseGame::Execute(Command cmd, int index, int prop, const Vector& val)
{
    assert(cmd < Command_NumOfCommands);
    Entity* pEntity = NULL;
    if (index == LAST_ADDED_ENTITY)
        index = m_lastAddedEntityIndex;
    if (index >= MAX_ENTITIES)
        return false;

    pEntity = m_entities[index];

    if (pEntity != NULL)
    {
        switch (cmd)
        {
        case Command_SetVectorProperty:
            pEntity->SetProperty(prop, val);
            return true;
        default:
            return false;
        }
    }
    return false;
}



NetworkBuffer::NetworkBuffer() : m_bEnabled(false)
{
	m_nSendBufferBytes = 0;
	m_nReceiveBufferBytes = 0;
}

bool NetworkBuffer::Execute(BaseGame* pGame, const uint8_t* pBuffer, int size)
{
	const bool bVerbose = false;
	const ScalarPacket* sp;
	const VectorPacket* vp;
	while (size >= sizeof(*sp))
	{
		sp = (const ScalarPacket*)pBuffer;
		if (sp->cmd == Command_SetVectorProperty)
		{
			if (size >= sizeof(*vp))
			{
				vp = (const VectorPacket*)pBuffer;
                pGame->Execute((Command)vp->cmd, vp->index, vp->prop, Vector(vp->vals[0], vp->vals[1], vp->vals[2]));
				pBuffer += sizeof(*vp);
				size -= sizeof(*vp);
			}
			else
				break;
		}
		else
		{
			if (sp->cmd == Command_SetIntProperty)
                pGame->Execute((Command)sp->cmd, sp->index, sp->prop, sp->val);
			else // any other command also directed to the int version of execute() method:
                pGame->Execute((Command)sp->cmd, sp->index, sp->prop, sp->val);
			pBuffer += sizeof(*sp);
			size -= sizeof(*sp);
		}
		if (bVerbose)
			cout << (Command)sp->cmd << endl;
	}
	// incomplete data pakcet?
	if (size > 0)
	{
		//cout << "WARNING: size > 0 when exiting Game::Execute()!" << endl;
		memcpy(m_receiveReaminsBuffer, pBuffer, size);
		m_nReceiveBufferBytes = size;
	}
	else
		m_nReceiveBufferBytes = 0;
	return true;
}


// There are 3 cases:
// 1) this is a client. the buffer is used to send requests to the server
// 2) this is the server with some clients connected. the buffer is used to send commands to the clients
// 3) this is a 1 player mode server and client without connection to other computer. the buffer is not used in this case,
//    but we return true as success indication. m_bEnabled is false in this case.
bool NetworkBuffer::AddPacketRaw(void* pData, size_t size)
{
	if (!m_bEnabled)
		return true;
	assert(m_nSendBufferBytes + size < SEND_BUFFER_SIZE);
	if (m_nSendBufferBytes + size < SEND_BUFFER_SIZE)
	{
		memcpy(m_sendBuffer + m_nSendBufferBytes, pData, size);
		m_nSendBufferBytes += (int)size;
		return true;
	}
	return false;
}

bool NetworkBuffer::AddScalarPacket(Command cmd, uint16_t index, uint16_t prop, uint16_t val)
{
	ScalarPacket packet;
	packet.cmd = cmd;
	packet.index = index;
	packet.prop = prop;
	packet.val = val;
	return AddPacketRaw(&packet, sizeof(packet));
}

bool NetworkBuffer::AddVectorPacket(Command cmd, uint16_t index, uint16_t prop, const Vector& val)
{
	VectorPacket packet;
	packet.cmd = cmd;
	packet.index = index;
	packet.prop = prop;
    packet.add = 0;
    packet.vals[0] = (float)val[0];
    packet.vals[1] = (float)val[1];
	packet.vals[2] = (float)val[2];
	return AddPacketRaw(&packet, sizeof(packet));
}

void NetworkBuffer::ClearSendBuffer()
{
	m_nSendBufferBytes = 0;
}

bool NetworkBuffer::NewClientBuffer(BaseGame* pGame, uint8_t* pBuffer, int& bytes)
{
	ScalarPacket sPacket;
	VectorPacket vPacket;
	bytes = 0;
	for (int i = 0; i < pGame->m_nEntities; i++)
	{
		Entity* pEnt = pGame->m_entities[i];
		if (pEnt != NULL)
		{
			// level entities are already exist in the client, so no Command_NewEntity for them:
			// position and velocity may be needed for them in the future, but currently are not
			// needed and not sent.
			if (i >= m_nLevelEntities)
			{
				if (bytes + sizeof(sPacket) < SEND_BUFFER_SIZE)
				{
					sPacket.cmd = Command_NewEntity;
					sPacket.index = i;
					sPacket.prop = 0;
					sPacket.val = pEnt->m_type;
					memcpy(pBuffer + bytes, &sPacket, sizeof(sPacket));
					bytes += sizeof(sPacket);
				}
				else
					return false;
				if (bytes + sizeof(vPacket) < SEND_BUFFER_SIZE)
				{
					vPacket.cmd = Command_SetVectorProperty;
					vPacket.index = i;
					vPacket.prop = Property_Position;
                    vPacket.vals[0] = pEnt->m_pos[0];
                    vPacket.vals[1] = pEnt->m_pos[1];
					vPacket.vals[2] = pEnt->m_pos[2];
					memcpy(pBuffer + bytes, &vPacket, sizeof(vPacket));
					bytes += sizeof(vPacket);
				}
				else
					return false;
				if (bytes + sizeof(vPacket) < SEND_BUFFER_SIZE)
				{
					vPacket.cmd = Command_SetVectorProperty;
					vPacket.index = i;
					vPacket.prop = Property_Velocity;
                    vPacket.vals[0] = pEnt->m_velocity[0];
                    vPacket.vals[1] = pEnt->m_velocity[1];
                    vPacket.vals[2] = pEnt->m_velocity[2];
					memcpy(pBuffer + bytes, &vPacket, sizeof(vPacket));
					bytes += sizeof(vPacket);
				}
				else
					return false;
			}
		}
	}
	/*for (int i = 0; i < m_nPlayers; i++)
	{
		if (bytes + sizeof(sPacket) < SEND_BUFFER_SIZE)
		{
			sPacket.cmd = Command_SetIntProperty;
			sPacket.index = m_players[i]->m_index;
			sPacket.prop = CAR_INT_PROPERTY_PLAYER;
			sPacket.val = 0;
			memcpy(pBuffer + bytes, &sPacket, sizeof(sPacket));
			bytes += sizeof(sPacket);
		}
		else
			return false;
	}*/
	return true;
}
