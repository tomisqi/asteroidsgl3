#include <assert.h>
#include "texture.h"
#include "animation.h"

Animation AnimationBuild(int xCount, int yCount, TextureHandleT textureHandle, float framerate, bool loop)
{
	Animation animation = { 0 };
	animation.enabled = false;
	animation.framerate = framerate;
	animation.loop = loop;
	animation.uvRectCount = xCount * yCount;
	animation.textureHandle = textureHandle;
	assert(animation.uvRectCount  <= MAX_UVRECTS_ANIM);

	Vector2 uvSize = V2(1.0f / xCount, 1.0f / yCount);
	for (int y = 0; y < yCount; y++)
	{
		for (int x = 0; x < xCount; x++)
		{
			animation.uvRectsAnim[y * xCount + x] = NewRect(V2(x*uvSize.x, uvSize.y * (yCount - 1) - y*uvSize.y), uvSize);
		}
	}

	animation.uvRectCurrent = 0;

	return animation;
}

void AnimationUpdate(Animation* animation_p, double time)
{
	float frameDuration = 1.0f / animation_p->framerate;
	float tNext = animation_p->tStart + (animation_p->uvRectCurrent + 1) * frameDuration;
	if (time >= tNext)
	{
		animation_p->uvRectCurrent = (animation_p->uvRectCurrent + 1) % animation_p->uvRectCount;
		if (animation_p->uvRectCurrent == 0)
		{
			animation_p->tStart = time;
			if (!animation_p->loop) animation_p->enabled = false;
		}
	}
}

Rect AnimationGetCurrentUv(Animation* animation_p)
{
	return animation_p->uvRectsAnim[animation_p->uvRectCurrent];
}