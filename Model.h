#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "Math.h"

struct Face
{
	std::vector<int> v;
	std::vector<int> vt;
	std::vector<int> vn;
};

class Model
{
public:
	std::vector<Vec3> vertices;
	std::vector<Vec2> verticesTexture;
	std::vector<Vec3> verticesNormal;
	std::vector<Face> faces;



	Model(const char* filename)
	{
		Vec3 lightDir(0, 0, -1);

		std::ifstream in(filename);
		std::string line;
		while (std::getline(in, line))
		{
			if (line.compare(0, 2, "v ") == 0)
			{
				std::istringstream iss(line.substr(2));

				float x, y, z;
				if (!(iss >> x >> y >> z))
				{
					continue;
				}

				vertices.push_back(Vec3(x, y, z));
			}
			else if (line.compare(0, 3, "vt ") == 0)
			{
				std::istringstream iss(line.substr(3));

				float x, y;
				if (!(iss >> x >> y))
				{
					continue;
				}

				verticesTexture.push_back(Vec2(x, y));
			}
			else if (line.compare(0, 3, "vn ") == 0)
			{
				std::istringstream iss(line.substr(3));

				float x, y, z;
				if (!(iss >> x >> y >> z))
				{
					continue;
				}

				verticesNormal.push_back(Vec3(x, y, z));
			}
			else if(line.compare(0, 2, "f ") == 0)
			{
				std::istringstream iss(line.substr(2));
				std::vector<int> v_indices;
				std::vector<int> vt_indices;
				std::vector<int> vn_indices;

				std::string vertexStr;
				while (iss >> vertexStr)
				{
					std::istringstream vss(vertexStr);
					int v, vt, vn;
					char slash;

					vss >> v;

					if (vss.peek() == '/')
					{
						vss >> slash;
						if (vss.peek() != '/')
						{
							vss >> vt;
							vt_indices.push_back(vt - 1);
						}
						if (vss.peek() == '/')
						{
							vss >> slash;
							vss >> vn;
							vn_indices.push_back(vn - 1);
						}
					}

					v_indices.push_back(v - 1);//obj的索引从1开始，我们要从0开始
				}

				//如果读取到四个顶点就拆成两个三角形
				for (int i = 1; i < (int)v_indices.size() - 1; i++)
				{
					Face face;

					face.v.push_back(v_indices[0]);
					face.v.push_back(v_indices[i]);
					face.v.push_back(v_indices[i+1]);

					face.vt.push_back(vt_indices[0]);
					face.vt.push_back(vt_indices[i]);
					face.vt.push_back(vt_indices[i + 1]);

					face.vn.push_back(vn_indices[0]);
					face.vn.push_back(vn_indices[i]);
					face.vn.push_back(vn_indices[i + 1]);

					faces.push_back(face);
				}
			}
		}
	}

	int nverts()
	{
		return vertices.size();
	}

	int nfaces()
	{
		return faces.size();
	}

	Vec3 vert(int i)
	{
		return i < nverts() ? vertices[i] : Vec3(0, 0, 0);
	}

	Vec2 vertTex(int i)
	{
		return i < verticesTexture.size() ? verticesTexture[i] : Vec2(0, 0);
	}

	Vec3 vertNor(int i)
	{
		return i < verticesNormal.size() ? verticesNormal[i] : Vec3(0, 0, 0);
	}

	Face face(int idx)
	{
		return faces[idx];
	}
};