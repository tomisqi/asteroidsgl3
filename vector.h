#pragma once
#include "math.h"

#define PI 3.1415926535897932384626433832795f

#define VECTOR2_ZERO	V2(0.0f, 0.0f)
#define VECTOR2_ONE		V2(1.0f, 1.0f)
#define VECTOR2_RIGHT	V2(1.0f, 0.0f)
#define VECTOR2_UP		V2(0.0f, 1.0f)
#define VECTOR2_LEFT	V2(-1.0f, 0.0f)
#define VECTOR2_DOWN	V2(0.0f, -1.0f)

#define VECTOR3_ZERO	V3(0.0f, 0.0f, 0.0f)
#define VECTOR3_ONE	    V3(1.0f, 1.0f, 1.0f)

struct Vector2
{
	float x;
	float y;
};

struct Vector3
{
	float x;
	float y;
	float z;
};

static inline Vector2 V2(float x, float y)
{
	Vector2 result = {x, y};
	return result;
}

static inline Vector3 V3(float x, float y, float z)
{
	Vector3 result = { x, y, z };
	return result;
}

static inline Vector3 V3(Vector2 v2)
{
	Vector3 result = {v2.x, v2.y, 0};
	return result;
}

static inline Vector2 operator+(Vector2 a, Vector2 b)
{
	Vector2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;

	return result;
}

static inline Vector2 operator-(Vector2 a, Vector2 b)
{
	Vector2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;

	return result;
}

static inline Vector2 operator*(float s, Vector2 a)
{
	Vector2 result;
	result.x = s * a.x;
	result.y = s * a.y;

	return result;
}

static inline Vector2 & operator+=(Vector2 &a, Vector2 b)
{
	a = a + b;
	return a;
}

static inline Vector2 & operator-=(Vector2 &a, Vector2 b)
{
	a = a - b;
	return a;
}

static inline bool operator==(Vector2 a, Vector2 b)
{
	return (a.x == b.x) && (a.y == b.y);
}

static inline Vector2 Scale(Vector2 a, Vector2 b)
{
	return V2(a.x*b.x, a.y*b.y);
}

static inline float MagnitudeSq(Vector2 v)
{
	return v.x*v.x + v.y*v.y;
}

static inline float Magnitude(Vector2 v)
{
	return sqrtf(MagnitudeSq(v));
}

static inline float Distance(Vector2 a, Vector2 b)
{
	return Magnitude(b - a);
}

static inline Vector2 Normalize(Vector2 v)
{
	Vector2 result = VECTOR2_ZERO;
	float magnSq = MagnitudeSq(v);
	if (magnSq > 0.0001f * 0.0001f)
	{
		result = (1.0f / sqrtf(magnSq)) * v;
	}
	return result;
}

static inline float DegToRad(float deg)
{
	return (PI / 180.0f) * deg;
}

static inline Vector2 RotateRad(Vector2 v, float rad)
{
	return v.x * V2(cosf(rad), sinf(rad)) + v.y * V2(-sinf(rad), cosf(rad));
}

static inline Vector2 RotateDeg(Vector2 v, float deg)
{
	float rad = DegToRad(deg);
	return RotateRad(v, rad);
//	return v.x*V2(cosf(rad), sinf(rad)) + v.y*V2(-sinf(rad), cosf(rad));
}

static inline float Dot(Vector2 v1, Vector2 v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}