#pragma once
#include "common.h"
#include "vector.h"
#include <stb_truetype.h>

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
	// @todo: Use render group where each render group uses it's own shader.
	RenderCommands renderCommands;
};

void RendererInit(Renderer* renderer_p);
void RendererEndFrame(Renderer* renderer_p);
void PushSprite(Renderer* renderer_p, Vector2 pos, Vector2 size, Vector2 facingV, TextureHandleT textureHandle);
void PushText(Renderer* renderer_p, char* text, Vector2 pos, stbtt_bakedchar* bakedCharData_p, TextureHandleT textTextureHandle);