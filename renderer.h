#pragma once
#include "common.h"
#include "vector.h"
#include <stb_truetype.h>

#define MAX_RENDER_GROUPS 2

typedef S16 TextureHandleT;

enum RenderGroupTypeE
{
	NONE,
	RENDER_GROUP_SPRITES_DEFAULT,
	RENDER_GROUP_TEXT_DEFAULT,
};

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

struct RenderGroup
{
	RenderGroupTypeE renderGroupType;
	int shaderProgram;
	RenderCommands renderCommands;
};

struct Renderer
{
	int groupCnt;
	RenderGroup renderGroups[MAX_RENDER_GROUPS];
};

void RendererInit(Renderer* renderer_p);
void RendererEndFrame(Renderer* renderer_p);
void PushSprite(Renderer* renderer_p, Vector2 pos, Vector2 size, Vector2 facingV, TextureHandleT textureHandle);
void PushText(Renderer* renderer_p, char* text, Vector2 pos, stbtt_bakedchar* bakedCharData_p, TextureHandleT textTextureHandle);