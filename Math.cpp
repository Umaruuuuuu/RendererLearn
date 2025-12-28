#include "Math.h"

constexpr double PI = 3.14159265358979323846;

// Vec2
Vec2 operator+(const Vec2& a, const Vec2& b)
{
	return Vec2(a.x + b.x, a.y + b.y);
}

Vec2 operator-(const Vec2& a, const Vec2& b)
{
	return Vec2(a.x - b.x, a.y - b.y);
}

Vec2 operator*(const Vec2& v, float scalar)
{
	return Vec2(v.x * scalar, v.y * scalar);
}

// Vec3
Vec3 operator+(const Vec3& a, const Vec3& b)
{
	return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 operator-(const Vec3& a, const Vec3& b)
{
	return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 operator*(const Vec3& v, float scalar)
{
	return Vec3(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vec3 operator*(const Vec3& v1, const Vec3& v2)
{
	return Vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

float dot(const Vec3& a, const Vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 cross(const Vec3& a, const Vec3& b)
{
	return Vec3(a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x);
}

float length(const Vec3& v)
{
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 normalize(const Vec3& v)
{
	float len = length(v);

	//当len接近0时，不予计算，返回空向量
	if (len > 1e-6)
	{
		return v * (1.0f / len);//为提升性能，应多用倒数乘法
	}

	return Vec3();
}

Uint32 Vec3ToUint32(const Vec3& color)
{
	//防止颜色溢出
	float r = std::clamp(color.x, 0.0f, 1.0f);
	float g = std::clamp(color.y, 0.0f, 1.0f);
	float b = std::clamp(color.z, 0.0f, 1.0f);

	Uint8 ir = (Uint8)(r * 255);
	Uint8 ig = (Uint8)(g * 255);
	Uint8 ib = (Uint8)(b * 255);
	Uint8 ia = 255;

	return (ia << 24) | (ir << 16) | (ig << 8) | ib;
}

// Mat4 
Mat4 operator*(const Mat4& A, const Mat4& B)
{
	Mat4 result;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = 0.0f;
			for (int k = 0; k < 4; k++)
			{
				result.m[i][j] += A.m[i][k] * B.m[k][j];
			}
		}
	}

	return result;
}

Vec4 operator*(const Mat4& M, const Vec3& v)
{
	float x = M.m[0][0] * v.x + M.m[0][1] * v.y + M.m[0][2] * v.z + M.m[0][3] * 1.0f;
	float y = M.m[1][0] * v.x + M.m[1][1] * v.y + M.m[1][2] * v.z + M.m[1][3] * 1.0f;
	float z = M.m[2][0] * v.x + M.m[2][1] * v.y + M.m[2][2] * v.z + M.m[2][3] * 1.0f;
	float w = M.m[3][0] * v.x + M.m[3][1] * v.y + M.m[3][2] * v.z + M.m[3][3] * 1.0f;

	//if (std::abs(w) > 1e-6)
	//{
	//	return Vec3(x / w, y / w, z / w);//当有连续除法出现，编译期会自动优化成倒数乘法
	//}

	return Vec4(x, y, z, w);
}

Mat4 CreateTranslation(const Vec3& t)
{
	Mat4 result = Mat4::Indentity();
	result.m[0][3] = t.x;
	result.m[1][3] = t.y;
	result.m[2][3] = t.z;

	return result;
}


Mat4 CreateRotation(const Vec3& axis, float radians) 
{
	Vec3 a = normalize(axis);
	float c = cos(radians), s = sin(radians);
	float ic = 1.0f - c;

	Mat4 res = Mat4::Indentity();
	res.m[0][0] = a.x * a.x * ic + c;
	res.m[0][1] = a.x * a.y * ic - a.z * s;
	res.m[0][2] = a.x * a.z * ic + a.y * s;

	res.m[1][0] = a.y * a.x * ic + a.z * s;
	res.m[1][1] = a.y * a.y * ic + c;
	res.m[1][2] = a.y * a.z * ic - a.x * s;

	res.m[2][0] = a.z * a.x * ic - a.y * s;
	res.m[2][1] = a.z * a.y * ic + a.x * s;
	res.m[2][2] = a.z * a.z * ic + c;

	return res;
}

Mat4 CreateScale(const Vec3& s)
{
	Mat4 res = Mat4::Indentity();

	res.m[0][0] = s.x;
	res.m[1][1] = s.y;
	res.m[2][2] = s.z;
	res.m[3][3] = 1.0f;

	return res;
}

Mat4 CreateView(const Vec3& position, const Vec3& target, const Vec3& worldup)
{
	Vec3 front = normalize(target - position);
	Vec3 right = normalize(cross(front, worldup));
	Vec3 cameraup = normalize(cross(right, front));

	Mat4 Rview = Mat4::Indentity();
	Rview.m[0][0] = right.x;	 Rview.m[0][1] = right.y;		Rview.m[0][2] = right.z;
	Rview.m[1][0] = cameraup.x;	 Rview.m[1][1] = cameraup.y;	Rview.m[1][2] = cameraup.z;
	Rview.m[2][0] = -front.x;    Rview.m[2][1] = -front.y;		Rview.m[2][2] = -front.z;

	Mat4 Tview = Mat4::Indentity();
	Tview.m[0][3] = -position.x;
	Tview.m[1][3] = -position.y;
	Tview.m[2][3] = -position.z;

	return Rview * Tview;
}

Mat4 CreatePerspective(float fovY, float aspect, float nearZ, float farZ)
{
	Mat4 perspective;

	float f = 1.0f / std::tan(fovY / 2.0f); //焦距

	perspective.m[0][0] = f / aspect;
	perspective.m[1][1] = f;
	perspective.m[2][2] = (nearZ + farZ) / (nearZ - farZ);
	perspective.m[2][3] = (2.0f * nearZ * farZ) / (nearZ - farZ);
	perspective.m[3][2] = -1.0f;

	return perspective;
}

float to_radians(float angle)
{
	return angle * PI / 180;
}