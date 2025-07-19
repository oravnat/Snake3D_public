//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include "Vector.hpp"
#include "SnakeRenderer.hpp"
#include <vector>

struct Material
{
    // ambient:
    //Color m_aColor;
    // diffuse:
    BColor m_dColor;
    //float diffuseColor[3];
    // specualr:
    //Color m_sColor;
    // emission:
    //Color m_eColor;
    //float m_shininess;

    int m_startIndex;
    int m_count;
};

class Model
{
public:
    Model(void) : mode(SNAKE_TRIANGLES) {}
    ~Model(void) { }
public:
    void Render(SnakeRenderer& renderer)
    {
        if (m_materials.size() > 0 )
        {
        	renderer.SetData(m_vertex3f3f.data(), m_vertex3f3f.size());
            for (int i = 0; i < m_materials.size(); i++)
            {
                BColor diffuse = m_materials[i].m_dColor;
                renderer.Color(diffuse.c[0], diffuse.c[1], diffuse.c[2], 255);
                renderer.DrawArrays(mode, m_materials[i].m_startIndex, m_materials[i].m_count);
            }
        }
        else if (m_vertex3f3f.size() > 0)
        {
            renderer.DrawVertex3f3f(m_vertex3f3f.data(), m_vertex3f3f.size(), mode);
        }
        else if (m_vertex3f3f2f.size() > 0)
        {
            //renderer.UseTexture(m_texture);
            renderer.DrawVertex3f3f2f(m_vertex3f3f2f.data(), m_vertex3f3f2f.size());
            renderer.Use3DProgram();

        }
    }

    void Boom(Model& dest, float r)
    {
        dest.mode = mode;
        if (dest.m_vertex3f3f.size() != m_vertex3f3f.size())
            dest.m_vertex3f3f.resize(m_vertex3f3f.size());
        r /= 3.0f;
        std::vector<Vertex3f3f>& p = dest.m_vertex3f3f;
        size_t n = m_vertex3f3f.size();
        for (size_t i = 0; i < n; i += 3)
        {
            // Zero vector after construction:
            Vector tranlsateBy;
            // Assuming triangles only:
            for (int j = 0; j < 3; j++)
                tranlsateBy += m_vertex3f3f[i + j].pos;
            tranlsateBy *= r;
            for (int j = 0; j < 3; j++)
            {
                p[i + j] = m_vertex3f3f[i + j];
                p[i + j].pos += tranlsateBy;
            }
        }
    }

    void Ball(Model& dest, float w, float r)
    {
        dest.mode = mode;
        if (dest.m_vertex3f3f.size() != m_vertex3f3f.size())
            dest.m_vertex3f3f.resize(m_vertex3f3f.size());
        r /= 3.0f;
        size_t n = m_vertex3f3f.size();
        for (size_t i = 0; i < n; i += 3)
        {
		// Assuming triangles only:
		for (int j = 0; j < 3; j++)
		{
			Vector v = m_vertex3f3f[i + j].pos;
			Vector vr = (r / v.Length()) * v;
			dest.m_vertex3f3f[i + j].pos = w * v + (1 - w) * vr;
			dest.m_vertex3f3f[i + j].normal = m_vertex3f3f[i + j].normal;
		}
		/*Vector a = dest.m_pVertexArray[i + 1] - dest.m_pVertexArray[i];
		Vector b = dest.m_pVertexArray[i + 2] - dest.m_pVertexArray[i];
		Vector normal = a.CrossProduct(b);
		normal.Normalize();
		// Same normal to all vectors (face normal):
		for (int j = 0; j < 3; j++)
			dest.m_pNormalArray[i + j] = normal;*/
        }
    }

    void BoundBoxFromArray(Vector& mins, Vector& maxs)
    {
        if (m_vertex3f3f.size() > 0)
        {
            mins = m_vertex3f3f[0].pos;
            maxs = m_vertex3f3f[0].pos;
        }
        for (size_t i = 0; i < m_vertex3f3f.size(); i++)
        {
            Vector v = m_vertex3f3f[i].pos;
            for (int j = 0; j < 3; j++)
            {
                if (v[j] < mins[j])
                    mins[j] = v[j];
                if (v[j] > maxs[j])
                    maxs[j] = v[j];
            }
        }
    }
    void AveragePosition(Vector& avg)
    {
        Vector sum;
        if (m_vertex3f3f.size() > 0)
        {
            for (size_t i = 0; i < m_vertex3f3f.size(); i++)
                sum += m_vertex3f3f[i].pos;
            avg = (1.0f/m_vertex3f3f.size()) * sum;
        }
        else if (m_vertex3f3f2f.size() > 0)
        {
            for (size_t i = 0; i < m_vertex3f3f2f.size(); i++)
                sum += m_vertex3f3f2f[i].pos;
            avg = (1.0f/m_vertex3f3f2f.size()) * sum;
        }
    }

public:
    std::vector<Vertex3f3f> m_vertex3f3f;
    std::vector<Vertex3f3f2f> m_vertex3f3f2f;
    std::vector<Material> m_materials;
    unsigned mode;
    Texture m_texture;
};
