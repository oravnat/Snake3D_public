//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Vector.hpp"
#include "defines.hpp"
#include "Entity.hpp"

#define TAIL_VECTOR_PROPERTY_ADD 2 // 0 and 1 are already taken by position and velocity

bool AreCross(const Vector& v11, const Vector& v12, const Vector& v21, const Vector& v22);

class Car;

class Tail : public Entity
{
public:
	Tail(Car* pOwner);
	~Tail(void);
public:
	int CrossIndex(const Vector& testStart, const Vector& testEnd, int which);
	bool Enlarge();
	bool Add(const Vector& pos, const Vector& right, const Vector& up, int timestamp);
	// 1 based (1 to m_tailSize):
	Vertex3f3f GetTail(int side, int index);
	void RenderSide(SnakeRenderer& renderer, int side, const Vector& v1, const Vector& v2, const Vector& norm);
	void RenderShadow(SnakeRenderer& renderer);
	void RenderInternal(SnakeRenderer& renderer);
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
	virtual void Draw2D(SnakeRenderer& renderer);
	virtual bool SetProperty(int index, const Vector& val);
    virtual bool ReactToCar(SnakeGame& game, Car& car);
	static void RenderRect(SnakeRenderer& renderer, const Vector& v1, const Vector& v2, const Vector& v3, const Vector& v4);
public:
	Vertex3f3f m_tail[4][MAX_TAIL];
	Vector m_tailPos[MAX_TAIL];
	int m_nextTail, m_tailSize, m_maxTailSize;
	Vector m_carPos, m_right, m_up;
	Car* m_pOwner;
	int m_lastTailAddTime;
	bool bRenderWithTailPos;
};
