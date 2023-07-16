#pragma once
#include "vector.h"

struct Rect
{
	Vector2 pos;
	Vector2 size;
};

static inline Rect NewRect(Vector2 pos, Vector2 size)
{
	Rect rect = { pos, size };
	return rect;
}

static inline Rect NewRectCenterPos(Vector2 centerPos, Vector2 size)
{
	Rect rect = { centerPos - 0.5f*size, size };
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

static inline Vector2 GetRectCenter(Rect rect)
{
	return V2(rect.pos.x + rect.size.x / 2, rect.pos.y + rect.size.y / 2);
}

static inline Rect ExpandRect(Rect rect, Vector2 expandSize)
{
	return NewRect(V2(rect.pos.x - expandSize.x / 2, rect.pos.y - expandSize.y / 2), rect.size + expandSize);
}

static inline Rect ExpandRect(Rect rect, float size)
{
	return ExpandRect(rect, V2(size, size));
}

static inline Vector2 RectMinXMinY(Rect rect)
{
	return rect.pos;
}

static inline Vector2 RectMaxXMaxY(Rect rect)
{
	return rect.pos + rect.size;
}

static inline Vector2 RectMaxXMinY(Rect rect)
{
	return V2(rect.pos.x + rect.size.x, rect.pos.y);
}

static inline Vector2 RectMinXMaxY(Rect rect)
{
	return V2(rect.pos.x, rect.pos.y + rect.size.y);
}