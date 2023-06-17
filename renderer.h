#pragma once
#include "common.h"
#include "vector.h"

typedef S16 TextureHandleT;

struct TexturedVertex
{
	Vector3 pos;
	Vector3 color;
	Vector2 uv;
	TextureHandleT textureHandle;
};

struct RenderCommands
{
	U32 maxVertexCount;
	U32 vertexCount;
	TexturedVertex* vertexArray;

	U32 maxIndexCount;
	U32 indexCount;
	U16* indexArray;
};

struct Renderer
{
	RenderCommands renderCommands;
};

void RendererInit(Renderer* renderer_p);
void RendererEndFrame(Renderer* renderer_p);
void PushSprite(Renderer* renderer_p, Vector2 pos, Vector2 size, Vector2 facingV, TextureHandleT textureHandle);