//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Entity.hpp"

enum WallType
{
	WallType_Circle, WallType_Line
};

class Wall : public Entity
{
public:
	Wall(void);
	Wall(Vector p1, Vector p2);
	~Wall(void);
public:
	virtual bool Think(SnakeGame& game);
	virtual void Render(SnakeRenderer& renderer);
	virtual void Draw2D(SnakeRenderer& renderer);
private:
	void RenderLine(SnakeRenderer& renderer);
	void CreateLine();
	void RenderCircle(SnakeRenderer& renderer);
	void CreateCircle();
	
private:
	WallType m_wallType;
	double m_radius, m_innerRadius;
	double m_height;
	double m_width;
	bool m_bPassThrough;
	bool m_bShrinking;
	Vector m_p1, m_p2;
	int m_nLineVertices;
	Vertex3f3f* m_pLineVertices[3];
	int m_nCircleVertices;
	Vector* m_pCircleVertices[3];
	Vector* m_pCircleNormals;
};
