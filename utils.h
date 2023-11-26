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

static inline void InsertChar(char* str, int strLen, int index, char c)
{
	int charCnt = strLen - index;
	char* new_p = &str[strLen];
	char* old_p = &str[strLen - 1];
	for (int i = 0; i < charCnt; i++)
	{
		*new_p = *old_p;
		new_p--;
		old_p--;
	}
	str[index] = c;
}


static inline void RemoveChar(char* str, int strLen, int index)
{
	int charCnt = strLen - index;
	char* new_p = &str[index];
	char* old_p = &str[index + 1];
	for (int i = 0; i < charCnt; i++)
	{
		*new_p = *old_p;
		new_p++;
		old_p++;
	}
	if (strLen) str[strLen - 1] = '\0';
}

static inline void RemoveChars(char* str, int strLen, int startIdx, int endIdx)
{
	int charCnt = strLen - endIdx;
	char* new_p = &str[startIdx];
	char* old_p = &str[endIdx];
	for (int i = 0; i < charCnt; i++)
	{
		*new_p = *old_p;
		new_p++;
		old_p++;
	}
	while (*new_p) { *new_p = '\0'; new_p++; }
}