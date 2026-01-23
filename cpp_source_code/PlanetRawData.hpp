#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
//#include <iostream>

#include "const_value.hpp"
#include "Vector3.hpp"
#include "util.hpp"

#pragma warning(disable:4244)

class PlanetRawData
{
public:
	std::vector<unsigned short> heightData;
	//std::vector<float> debugData;
	//unsigned short heightData[DATALENGTH] = {0};
	static Vector3 vertices[161604];
	static int indexMap[60000];
	
	static int trans(float x,int pr)
	{
		int num = (int)((Mathf.Sqrt(x + 0.23f) - 0.4795832f) / 0.6294705f * (float)pr);
		if(num >= pr)
		{
			num = pr - 1;
		}
		return num;
	}

	static int PositionHash(Vector3 v,int corner = 0)
	{
		if(corner == 0)
		{
			corner = ((v.x > 0.0f) ? 1 : 0) + ((v.y > 0.0f) ? 2 : 0) + ((v.z > 0.0f) ? 4 : 0);
		}
		if(v.x < 0.0f)
		{
			v.x = 0.0f - v.x;
		}
		if(v.y < 0.0f)
		{
			v.y = 0.0f - v.y;
		}
		if(v.z < 0.0f)
		{
			v.z = 0.0f - v.z;
		}
		if((double)v.x < 1E-06 && (double)v.y < 1E-06 && (double)v.z < 1E-06)
		{
			return 0;
		}
		int num = 0;
		int num2 = 0;
		int num3 = 0;
		if(v.x >= v.y && v.x >= v.z)
		{
			num = 0;
			num2 = trans(v.z / v.x,INDEXMAP_PRECISION);
			num3 = trans(v.y / v.x,INDEXMAP_PRECISION);
		} else if(v.y >= v.x && v.y >= v.z)
		{
			num = 1;
			num2 = trans(v.x / v.y,INDEXMAP_PRECISION);
			num3 = trans(v.z / v.y,INDEXMAP_PRECISION);
		} else
		{
			num = 2;
			num2 = trans(v.x / v.z,INDEXMAP_PRECISION);
			num3 = trans(v.y / v.z,INDEXMAP_PRECISION);
		}
		return num2 + num3 * INDEXMAP_PRECISION + num * INDEXMAP_FACE_STRIDE + corner * INDEXMAP_CORNER_STRIDE;
	};

	float QueryHeight(Vector3 vpos) const
	{
		vpos.Normalize();
		int num = PositionHash(vpos);
		int num2 = indexMap[num];
		float num3 = Mathf.PI / (float)(PRECISION * 2) * 1.2f;
		float num4 = num3 * num3;
		float num5 = 0.0f;
		float num6 = 0.0f;
		int num7 = STRIDE;
		for(int i = -1; i <= 3; i++)
		{
			for(int j = -1; j <= 3; j++)
			{
				int num8 = num2 + i + j * num7;
				if((unsigned int)num8 < DATALENGTH)
				{
					float sqrMagnitude = (vertices[num8] - vpos).sqrMagnitude();
					if(!(sqrMagnitude > num4))
					{
						float num9 = 1.0f - Mathf.Sqrt(sqrMagnitude) / num3;
						float num10 = (int)heightData[num8];
						num5 += num9;
						num6 += num10 * num9;
					}
				}
			}
		}
		if(num5 == 0.0f)
		{
			return (float)(int)heightData[0] * 0.01f;
		}
		return num6 / num5 * 0.01f;
	};

	static void ReadData()
	{
		static bool is_init = false;
		if(is_init)
			return;
		is_init = true;
		ReadVerts();
		ReadIndex();
	}

	static void ReadVerts()
	{
		std::ifstream file("assets/vertices.data");  // 打开txt文件
		if(!file.is_open()) {
			throw std::runtime_error("无法打开vertices.data文件");
		}

		std::string line;

		int line_index = 0;
		while(std::getline(file,line)) {
			std::stringstream ss(line);
			std::string item;
			std::vector<float> values;

			// 按逗号分割每行
			while(std::getline(ss,item,',')) {
				values.push_back(std::stof(item));
			}
			vertices[line_index] = Vector3(values[0],values[1],values[2]);
			line_index++;
		}
		if(line_index != 161604) {
			throw std::runtime_error("vertices.data数据异常");
		}
		file.close();
	};

	static void ReadIndex()
	{
		std::ifstream file("assets/indexMap.data");  // 打开txt文件
		if(!file.is_open()) {
			throw std::runtime_error("无法打开indexMap.data文件");
		}

		std::string line;

		int line_index = 0;
		while(std::getline(file,line)) {
			indexMap[line_index] = std::stoi(line);
			line_index++;
		}
		if(line_index != 60000) {
			throw std::runtime_error("indexMap.data数据异常");
		}
		file.close();
	};

	static void CalcVerts()
	{
		static bool is_init = false;
		if(is_init)
			return;
		is_init = true;
		int num = (PRECISION + 1) * 2;
		int num2 = PRECISION + 1;
		std::vector<Vector3> poles = {
			Vector3::right(),
			Vector3::left(),
			Vector3::up(),
			Vector3::down(),
			Vector3::forward(),
			Vector3::back()
		};
		for(int j = 0; j < DATALENGTH; j++)
		{
			int num3 = j % num;
			int num4 = j / num;
			int num5 = num3 % num2;
			int num6 = num4 % num2;
			int num7 = (((num3 >= num2) ? 1 : 0) + ((num4 >= num2) ? 1 : 0) * 2) * 2 + ((num5 < num6) ? 1 : 0);
			float num8 = ((num5 >= num6) ? (PRECISION - num5) : num5);
			float num9 = ((num5 >= num6) ? num6 : (PRECISION - num6));
			float num10 = (float)PRECISION - num9;
			num9 /= (float)PRECISION;
			num8 = ((num10 > 0.0f) ? (num8 / num10) : 0.0f);
			int num11 = 0;
			Vector3 a;
			Vector3 a2;
			Vector3 b;
			switch(num7)
			{
				case 0:
					a = poles[2];
					a2 = poles[0];
					b = poles[4];
					num11 = 7;
					break;
				case 1:
					a = poles[3];
					a2 = poles[4];
					b = poles[0];
					num11 = 5;
					break;
				case 2:
					a = poles[2];
					a2 = poles[4];
					b = poles[1];
					num11 = 6;
					break;
				case 3:
					a = poles[3];
					a2 = poles[1];
					b = poles[4];
					num11 = 4;
					break;
				case 4:
					a = poles[2];
					a2 = poles[1];
					b = poles[5];
					num11 = 2;
					break;
				case 5:
					a = poles[3];
					a2 = poles[5];
					b = poles[1];
					num11 = 0;
					break;
				case 6:
					a = poles[2];
					a2 = poles[5];
					b = poles[0];
					num11 = 3;
					break;
				case 7:
					a = poles[3];
					a2 = poles[0];
					b = poles[5];
					num11 = 1;
					break;
				default:
					a = poles[2];
					a2 = poles[0];
					b = poles[4];
					num11 = 7;
					break;
			}
			vertices[j] = Vector3::Slerp(Vector3::Slerp(a,b,num9),Vector3::Slerp(a2,b,num9),num8);
			//int num12 = PositionHash(vertices[j],num11);
			//if(indexMap[num12] == -1)
			//{
			//	indexMap[num12] = j;
			//}
		}
		//int num13 = 0;
		//for(int k = 1; k < INDEXMAP_DATA_LENGTH; k++)
		//{
		//	if(indexMap[k] == -1)
		//	{
		//		indexMap[k] = indexMap[k - 1];
		//		num13++;
		//	}
		//}
	};
};
