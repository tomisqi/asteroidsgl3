#pragma once

#include "rect.h"
#include "renderer.h"

#define MAX_UVRECTS_ANIM 16

struct Animation
{
	float enabled;
	float tStart;
	float framerate;
	bool loop;
	TextureHandleT textureHandle;

	int uvRectCurrent;
	int uvRectCount;
	Rect uvRectsAnim[MAX_UVRECTS_ANIM];
};

Animation AnimationBuild(int xCount, int yCount, TextureHandleT textureHandle, float framerate = 24, bool loop = true);

void AnimationUpdate(Animation* animation_p, double time);

Rect AnimationGetCurrentUv(Animation* animation_p);
