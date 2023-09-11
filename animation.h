#pragma once

#include "rect.h"

#define MAX_UVRECTS_ANIM 16

struct Animation
{
	float tStart;
	float framerate;
	bool loop;

	int uvRectCurrent;
	int uvCount;
	Rect uvRectsAnim[MAX_UVRECTS_ANIM];
};

Animation AnimationBuild(int xCount, int yCount, int uvCount, float framerate = 24, bool loop = true);

bool AnimationUpdate(Animation* animation_p, double time);

Rect AnimationGetCurrentUv(Animation* animation_p);
