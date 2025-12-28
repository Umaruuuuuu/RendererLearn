#pragma once

#include "Math.h"

struct Vertex
{
	Vec3 position;
	float inv_w;//用于透视矫正
	Vec3 color;
	Vec2 texcoord;
	Vec3 normal;
	Vec3 world_pos;
	float pos_clip_w;

	Vertex() : position(), inv_w(), color(), texcoord(), normal(), world_pos(), pos_clip_w() {}
	Vertex(const Vec3& p, const float w, const Vec3& c, const Vec2& t, const Vec3& n, const Vec3& wp, const float pcw) : position(p), inv_w(w), color(c), texcoord(t), normal(n), world_pos(wp), pos_clip_w(pcw) {}

	static Vertex lerp(const Vertex& a, const Vertex& b, float t)
	{
		Vertex v;
		v.position = a.position + (b.position - a.position) * t;
		v.color = a.color + (b.color - a.color) * t;
		v.texcoord = a.texcoord + (b.texcoord - a.texcoord) * t;
		v.normal = normalize(a.normal + (b.normal - a.normal) * t);
		v.world_pos = a.world_pos + (b.world_pos - a.world_pos) * t;
		v.pos_clip_w = a.pos_clip_w + (b.pos_clip_w - a.pos_clip_w) * t;
		v.inv_w = 1.0f / v.pos_clip_w;

		return v;
	}
};

struct Triangle
{
	Vertex v[3];

	Triangle() : v() {}
};

struct Camera
{
	Vec3 position;
	float speed;
	float yaw;
	float pitch;
	Vec3 target;

	static Camera* CreateCamera(Vec3 position,float speed, float yaw, float pitch)
	{
		Camera* c = new Camera();
		c->position = position;
		c->speed = speed;
		c->yaw = yaw;
		c->pitch = pitch;
		return c;
	}
};

enum LightType { Directional = 0, Point = 1 };

struct Light
{
	LightType type;

	Vec3 direction;
	Vec3 dir_inv;//光照反向
	Vec3 color;
	float intensity;

	Vec3 position;
	float Kc, Kl, Kq;

	static Light Directional(Vec3 dir, Vec3 col, float intens)
	{
		Light l;
		l.type = LightType::Directional;
		l.direction = normalize(dir);
		l.dir_inv = dir * -1.0f;
		l.color = col;
		l.intensity = intens;
		return l;
	}

	static Light Point(Vec3 pos, Vec3 col, float intens, float kl = 0.09f, float kq = 0.032f)
	{
		Light l;
		l.type = LightType::Point;
		l.position = pos;
		l.color = col;
		l.intensity = intens;
		l.Kc = 1.0f;
		l.Kl = kl;
		l.Kq = kq;
		return l;
	}
};

struct Material
{
	float ambient;
	float diffuse;
	float specular;
	float shininess;//高光指数
};