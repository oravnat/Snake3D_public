//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#include "Tail.hpp"
#include "defines.hpp"
#include "SnakeGame.hpp"
#include "SnakeRenderer.hpp"

const float h = 1.1f;
const float w = 0.11f;

// For line P(s) = start + s * dir, return s
float LinePlaneIntersection(const Vector& start, const Vector& dir, const Vector& p1, const Vector& p2, const Vector& p3)
{
	Vector d1 = p2 - p1;
	Vector d2 = p3 - p1;
	Vector normal = d1.CrossProduct(d2);
	Vector w = p1 - start;
	return normal.DotProduct(w) / normal.DotProduct(dir);
}

int Tail::CrossIndex(const Vector& testStart, const Vector& testEnd, int which)
{
	Vector dir = testEnd - testStart;
	int prevInd = (m_nextTail + MAX_TAIL - 1) % MAX_TAIL;
	for (int i = 2; i <= m_tailSize; i++)
	{
		int ind = (m_nextTail + MAX_TAIL - i) % MAX_TAIL;
		Vector p1 = m_tail[which][prevInd].pos;
		Vector p2 = m_tail[which][ind].pos;
		Vector p3 = m_tail[which + 1][ind].pos;

		float s = LinePlaneIntersection(testStart, dir, p1, p2, p3);
		// If the segment is crossing the plane:
		if (s >= 0 && s <= 1)
		{
			// The intersection point of the segment and the plane:
			Vector intersection = testStart + s * dir;
			// TODO: this is a bad math, improve that!
			bool inx = (((intersection.v[0] - p1[0]) * (intersection.v[0] - p2[0])) <= 0);
			bool inz = (((intersection.v[2] - p1[2]) * (intersection.v[2] - p2[2])) <= 0);
			bool iny = (((intersection.v[1] - p1[1]) * (intersection.v[1] - p3[1])) <= 0);
			// We are intersecting:
			if (inx && inz && iny)
				return ind;
		}

		//if (AreCross(start, end, testStart, testEnd))
			//return ind;
		// For now only check for distance. 
		// TODO: change to real check if the testStart->testEnd Vector cross any triangle in the tail.
		/*if ((testStart - start).Length2() < SMALL_HIT_DISTANCE2 || (testEnd - end).Length2() < SMALL_HIT_DISTANCE2
			|| (testStart - end).Length2() < SMALL_HIT_DISTANCE2 || (testEnd - start).Length2() < SMALL_HIT_DISTANCE2)
			return ind;*/
		prevInd = ind;
	}
	return -1;
}

Tail::Tail(Car* pOwner) : m_pOwner(pOwner), bRenderWithTailPos(false)
{
	m_type = EntityType_Tail;
	pOwner->m_pTail = this;
	m_nextTail = 0;
	m_tailSize = 0;
	m_maxTailSize = TAIL_START_SIZE;
	m_lastTailAddTime = 0;
}

Tail::~Tail(void)
{
}

void Tail::RenderRect(SnakeRenderer& renderer, const Vector& v1, const Vector& v2, const Vector& v3, const Vector& v4)
{
	Vector a[4] = {v1, v2, v3, v4};
	Vector d1 = v2 - v1;
	Vector d2 = v3 - v1;
	Vector normal = d1.CrossProduct(d2);
	normal.Normalize();
	Vertex3f3f verts[4];

	for (int  i = 0;  i <  4; ++ i) {
		verts[i].pos = a[i];
		verts[i].normal = normal;
	}

	renderer.DrawVertex3f3f(verts, 4, SNAKE_TRIANGLE_STRIP);
}

bool Tail::Enlarge()
{
	if (m_maxTailSize < MAX_TAIL)
	{
		m_maxTailSize += TAIL_ADDITION;
		if (m_maxTailSize > MAX_TAIL)
			m_maxTailSize = MAX_TAIL;
		return true;
	}
	return false;
}

// m_tail:
// [0] - bottom right
// [1] - upper right
// [2] - upper left
// [3] - bottom left
bool Tail::Add(const Vector& pos, const Vector& right, const Vector& up, int timestamp)
{
	// Add tail:
	if (m_lastTailAddTime != 0 && (timestamp - m_lastTailAddTime <= TAIL_RATE))
		return false;

	m_lastTailAddTime = timestamp;

	// The right side:
	Vertex3f3f tail;
	tail.pos = pos + w * right;
	tail.normal = right;
	m_tail[0][m_nextTail] = tail;
	tail.pos = pos + w * right + h * up;
	tail.normal = right;
	m_tail[1][m_nextTail] = tail;
	// The left side:
	tail.pos = pos - w * right + h * up;
	tail.normal = -right;
	m_tail[2][m_nextTail] = tail;
	tail.pos = pos - w * right;
	tail.normal = -right;
	m_tail[3][m_nextTail] = tail;
	m_nextTail++;
	m_nextTail = m_nextTail % MAX_TAIL;
	if (m_tailSize < m_maxTailSize)
		m_tailSize++;
	return true;
}

Vertex3f3f Tail::GetTail(int side, int index)
{
	int ind = (m_nextTail + MAX_TAIL - index) % MAX_TAIL;
	return m_tail[side][ind];
}

// side: 0 - right, 2 - left, -1 - top
void Tail::RenderSide(SnakeRenderer& renderer, int side, const Vector& v1, const Vector& v2, const Vector& norm)
{
	Vertex3f3f a[2 * MAX_TAIL + 2];

	a[0].pos = v1;
	a[1].pos = v2;
	a[0].normal = norm;
	a[1].normal = norm;
	for (int i = 1; i <= m_tailSize; i++)
	{
	    if (side == -1) //Top side
        {
            Vertex3f3f ver = GetTail(1, i);
            a[2 * i].pos = ver.pos;
            ver = GetTail(2, i);
            a[2 * i + 1].pos = ver.pos;
            Vector diff = GetTail(1, i).pos - GetTail(0, i).pos;
            a[2 * i].normal = diff;
            a[2 * i + 1].normal = diff;
        }
	    else
	    {
            Vertex3f3f ver = GetTail(side, i);
            a[2 * i] = ver;
            a[2 * i + 1] = ver;
            ver = GetTail(side + 1, i);
            a[2 * i + 1].pos = ver.pos;
        }
	}
	renderer.DrawVertex3f3f(a, 2 * m_tailSize + 2, SNAKE_TRIANGLE_STRIP);
}

bool Tail::Think(SnakeGame& game)
{
	// on a multiplayer game, only the server decides if a Tail kill a Car
	if (!game.m_bGameServer)
	{
		bRenderWithTailPos = true;
		return false;
	}
    
	return false;
}

void Tail::Render(SnakeRenderer& renderer)
{
	renderer.Color(m_pOwner->m_hullColor.c[0], m_pOwner->m_hullColor.c[1], m_pOwner->m_hullColor.c[2], m_pOwner->m_hullColor.c[3]);
	RenderInternal(renderer);
}

void Tail::RenderShadow(SnakeRenderer& renderer)
{
	RenderInternal(renderer);
}

void Tail::RenderInternal(SnakeRenderer& renderer)
{
	if (m_tailSize == 0)
		return;

	if (bRenderWithTailPos)
	{
		return;
	}

	// Render the rectangle near the car:
	Vector ur(m_carPos + w * m_right + h * m_up);
	Vector ul(m_carPos - w * m_right + h * m_up);
	Vector bl(m_carPos - w * m_right);
	Vector br(m_carPos + w * m_right);

	RenderRect(renderer, ul, ur, bl, br);

	// Render the right side:
	RenderSide(renderer, 0, br, ur, m_right);

	// Render the left side:
	RenderSide(renderer, 2, ul, bl, -m_right);
	// Render the top side:
	RenderSide(renderer, -1, ur, ul, Vector(0.0f, 1.0f, 0.0f));
	// Render the last rectangle:
	int ind = (m_nextTail + MAX_TAIL - m_tailSize) % MAX_TAIL;
	Vector v1 = m_tail[0][ind].pos;
	Vector v2 = m_tail[1][ind].pos;
	Vector v3 = m_tail[2][ind].pos;
	Vector v4 = m_tail[3][ind].pos;
	RenderRect(renderer, v3, v4, v2, v1);
}

void Tail::Draw2D(SnakeRenderer& renderer)
{
	Vector a[MAX_TAIL];
	renderer.Color(255, 0, 0, 255);
	if (!bRenderWithTailPos)
	{
		for (int i = 1; i <= m_tailSize; i++)
		{
			int ind = (m_nextTail + MAX_TAIL - i) % MAX_TAIL;
			a[i - 1] = m_tail[0][ind].pos;
		}
		renderer.DrawVectors(a, m_tailSize, SNAKE_LINE_STRIP);
	}
	else
	{
		for (int i = 1; i <= m_tailSize; i++)
		{
			int ind = (m_nextTail + MAX_TAIL - i) % MAX_TAIL;
			a[i - 1] = m_tailPos[ind];
		}
		renderer.DrawVectors(a, m_tailSize, SNAKE_LINE_STRIP);
	}

}

bool Tail::SetProperty(int index, const Vector& val)
{
	bool bResult = Entity::SetProperty(index, val);
	if (index == TAIL_VECTOR_PROPERTY_ADD)
	{
		m_tailPos[m_nextTail] = val;
		m_nextTail++;
		m_nextTail = m_nextTail % MAX_TAIL;
		if (m_tailSize < m_maxTailSize)
			m_tailSize++;
	}
	return bResult;
}

bool Tail::ReactToCar(SnakeGame& game, Car& car)
{
    // Check if the car cross this tail:
    if (car.bFirst || car.m_undeadUntil != 0)
        return false;
    for (int j = 0; j < 4; j += 2)
    {
        int crossIndex = CrossIndex(car.m_prevFront, car.m_carFront, j);
        if (crossIndex != -1)
        {
            if (game.m_bGameServer)
            {
                car.Kill();
                game.m_network.AddScalarPacket(Command_SetIntProperty, car.m_index, CAR_INT_PROPERTY_COMMAND, CarCommand_Killed);
                if (&car != m_pOwner)
                {
                    m_pOwner->m_pPlayer->m_points += TAIL_KILL_POINTS;
                    game.m_bUpdateStatus = true;
                    return false;
                }
            }
        }
    }
    return false;
}
