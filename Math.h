#pragma once

#include <cmath>
#include <algorithm>
#include <SDL3/SDL_stdinc.h>

// 实现一个二维向量
struct Vec2
{
	float x, y;

	Vec2() : x(0), y(0) {}

	Vec2(float x, float y) : x(x), y(y) {}
};

Vec2 operator+(const Vec2& a, const Vec2& b);
Vec2 operator-(const Vec2& a, const Vec2& b);
Vec2 operator*(const Vec2& v, float scalar);

// 实现一个四维向量
struct Vec4
{
	float x, y, z, w;

	Vec4() : x(0), y(0), z(0), w(1) {}

	Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

// 实现一个三维向量
struct Vec3
{
	float x, y, z;

	Vec3() : x(0), y(0), z(0) {}

	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vec3(const Vec4& v) : x(v.x), y(v.y), z(v.z) {}

	Vec3& operator+=(const Vec3& a)
	{
		x += a.x;
		y += a.y;
		z += a.z;
		return *this;
	}

	Vec3& operator-=(const Vec3& a)
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
		return *this;
	}
};

Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& v, float scalar);
Vec3 operator*(const Vec3& v1, const Vec3& v2);

float dot(const Vec3& a, const Vec3& b);
Vec3 cross(const Vec3& a, const Vec3& b);
float length(const Vec3& v);
Vec3 normalize(const Vec3& v);
Uint32 Vec3ToUint32(const Vec3& color);

//实现一个 4x4 矩阵
struct Mat4
{
	float m[4][4];

	Mat4()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m[i][j] = 0.0f;
			}
		}
	}

	static Mat4 Indentity()
	{
		Mat4 mat;
		mat.m[0][0] = mat.m[1][1] = mat.m[2][2] = mat.m[3][3] = 1.0f;
		return mat;
	}
};

Mat4 operator*(const Mat4& A, const Mat4& B);
Vec4 operator*(const Mat4& M, const Vec3& v);

Mat4 CreateTranslation(const Vec3& t);
Mat4 CreateRotation(const Vec3& axis, float radians);
Mat4 CreateScale(const Vec3& s);
Mat4 CreateView(const Vec3& position, const Vec3& target, const Vec3& worldup);
Mat4 CreatePerspective(float fovY, float aspect, float nearZ, float farZ);

float to_radians(float angle);