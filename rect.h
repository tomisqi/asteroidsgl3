#pragma once
#include "vector.h"

struct Rect
{
	Vector2 pos;
	Vector2 size;
};

static inline Rect NewRect(Vector2 pos, Vector2 size)
{
	Rect rect = { {pos}, {size} };
	return rect;
}

static bool RectContains(Rect rect, Vector2 pos)
{
	float xMin = rect.pos.x;
	float xMax = rect.pos.x + rect.size.x;
	float yMin = rect.pos.y;
	float yMax = rect.pos.y + rect.size.y;

	return pos.x >= xMin && pos.x <= xMax && pos.y >= yMin && pos.y <= yMax;
}

static Vector2 GetRectCenter(Rect rect)
{
	return V2(rect.pos.x + rect.size.x / 2, rect.pos.y + rect.size.y / 2);
}