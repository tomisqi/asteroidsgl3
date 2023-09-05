#pragma once
#include <stdlib.h>

static inline float Wrapf(float x, float min, float max)
{
	if (x > max) return min;
	if (x < min) return max;
	return x;
}

static inline float Clampf(float x, float min, float max)
{
	if (x > max) return max;
	if (x < min) return min;
	return x;
}

static inline int Clamp(int x, int min, int max)
{
	if (x > max) return max;
	if (x < min) return min;
	return x;
}

static inline int GetRandomValue(int min, int max)
{
	if (min > max)
	{
		int tmp = max;
		int max = min;
		int min = tmp;
	}

	return min + rand() % (abs(max - min) + 1);
}

static inline float GetRandomFloat01()
{
	return (double)rand() / (double)RAND_MAX;
}

static inline int GetRandomSign()
{
	int signs[2] = { -1, 1 };
	int randVal = GetRandomValue(0, 1);
	return signs[randVal];
}

static inline float Lerp(float start, float end, float t)
{
	t = Clampf(t, 0.0f, 1.0f);
	return start * (1 - t) + end * t;
}