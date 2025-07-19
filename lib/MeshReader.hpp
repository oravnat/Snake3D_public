//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <vector>
#include <map>
#include "Vector.hpp"

//using namespace std;

struct FaceIdcs
{
	int v[4];
	int vn[4];
	int vt[4];

	FaceIdcs()
	{
		for (int i = 0; i < 4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}

	FaceIdcs(std::istream & aStream)
	{
		for (int i = 0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;

		char c;
		for (int i = 0; i < 3; i++)
		{
			aStream >> std::ws >> v[i] >> std::ws;
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> std::ws;
			if (aStream.peek() == '/')
			{
				aStream >> c >> std::ws >> vn[i];
				continue;
			}
			else
				aStream >> vt[i];
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> vn[i];
		}

		aStream >> std::ws;
		// 4th vertex?
		if (!aStream.eof())
		{
			int i = 3;
			aStream >> std::ws >> v[i] >> std::ws;
			aStream >> c >> std::ws;
			if (aStream.peek() != '/')
				aStream >> vt[i];
			aStream >> c >> std::ws >> vn[i];
		}
	}
};

inline Vector vec2fFromStream(std::istream& aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return Vector(x, y, 0.0f);
}

inline Vector vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return Vector(x, y, z);
}

class MeshReader
{
public:
	std::vector<FaceIdcs> faces;
	std::vector<Vector> vertices;
	std::vector<Vector> normals;
	std::vector<Vector> textures;
	std::vector<int> materialIndices;
	std::vector<std::string> materialNames;
	// this value counts 4 vertex polygons as 2 triangles, so it may be bigger than faces.size()
	int nTriangles;
	std::string group; // if defined, load onlty specific group, TODO!

	MeshReader() : nTriangles(0)
	{
	}

	~MeshReader()
	{
	}

	void loadObj(std::istream & aStream)
	{
		faces.clear();
		vertices.clear();
		normals.clear();
		textures.clear();
		materialIndices.clear();
		nTriangles = 0; 
		materialNames.clear();
		std::string currentGroup;
		bool bReadFaces = (group == "");

		// while not end of stream:
		while (!aStream.eof())
		{
			// get line
			std::string curLine;
			getline(aStream, curLine);

			// read type of the line
			std::istringstream issLine(curLine);
			std::string lineType;

			issLine >> std::ws >> lineType;

			// based on the type parse data
			if (lineType == "v")
				vertices.push_back(vec3fFromStream(issLine));
			else if (lineType == "f")
			{
				if (bReadFaces)
				{
					FaceIdcs face = issLine;
					faces.push_back(face);
					if (face.v[3] > 0) // if 2 triangles in the face
						nTriangles += 2;
					else
						nTriangles++;
				}
			}
			else if (lineType == "vn")
				normals.push_back(vec3fFromStream(issLine));
			else if (lineType == "vt")
				textures.push_back(vec2fFromStream(issLine));
			else if (lineType == "usemtl")
			{
				std::string matName;
				materialIndices.push_back(nTriangles);
				issLine >> std::ws >> matName;
				materialNames.push_back(matName);
			}
			else if (lineType == "g")
			{
				issLine >> currentGroup;
				if (group != "")
					bReadFaces = (currentGroup == group);
				//if (currentGroup == "bombs")
					//cout << nTriangles << ", ";
			}
			else if (lineType == "#" || lineType == "")
			{
				// comment / empty line
			}
			else
			{
				//cout << "Found unknown line Type \"" << lineType << "\"";
			}
		}
	}
};

class MaterialReader
{
public:
	std::vector<std::string> materialNames;
	std::vector<Vector> diffuseColors;
	std::map<std::string, Vector> materials;

	MaterialReader()
	{
	}

	~MaterialReader()
	{
	}

	void loadMtl(std::istream& aStream)
	{
		std::string matName;
		materialNames.clear();
		diffuseColors.clear();

		// while not end of file
		while (!aStream.eof())
		{
			// get line
			std::string curLine;
			getline(aStream, curLine);

			// read type of the line
			std::istringstream issLine(curLine);
			std::string lineType;

			issLine >> std::ws >> lineType;

			if (lineType == "newmtl")
			{
				issLine >> std::ws >> matName;
				materialNames.push_back(matName);
			}
			if (lineType == "Kd")
			{
				Vector color = vec3fFromStream(issLine);
				diffuseColors.push_back(color);
				materials[matName] = color;
			}
		}
	}
};
