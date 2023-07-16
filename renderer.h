#pragma once
#include "common.h"
#include "vector.h"
#include "texture.h"
#include "rect.h"
#include <stb_truetype.h>

#define MAX_RENDER_GROUPS 16

typedef S16 TextureHandleT;

enum RenderGroupTypeE
{
	NONE,
	RENDER_GROUP_SPRITES_DEFAULT,
	RENDER_GROUP_TEXT_DEFAULT,
	RENDER_GROUP_UI,
	RENDER_GROUP_WIREFRAME,
};

struct TexturedVertex
{
	Vector3 pos;
	Vector3 color;
	Vector2 uv;
	TextureHandleT textureHandle;
};

struct ColoredVertex
{
	Vector3 pos;
	Vector4 color;
};

struct OrtographicProj
{
	float xmin;
	float xmax;
	float ymin;
	float ymax;
};

struct RenderCommands
{
	U32 maxVertexCount;
	U32 vertexCount;
	TexturedVertex* vertexArray;
	ColoredVertex* onlyColoredVertexArray;

	U32 maxIndexCount;
	U32 indexCount;
	U16* indexArray;
};

struct RenderGroup
{
	RenderGroupTypeE renderGroupType;
	int shaderProgram;
	RenderCommands renderCommands;
	OrtographicProj ortoProj;
};

struct TextRendering
{
	Texture textTexture;
	stbtt_bakedchar charUvData[96]; // ASCII 32..126 is 95 glyphs
};

struct Renderer
{
	int groupCnt;
	RenderGroup renderGroups[MAX_RENDER_GROUPS];

	TextRendering textRendering;
};

void RendererInit(Renderer* renderer_p);
void RendererEndFrame(Renderer* renderer_p);
void SetSpritesOrtographicProj(Renderer* renderer_p, Rect rect);
void SetWireframeOrtographicProj(Renderer* renderer_p, Rect rect);
void PushSprite(Renderer* renderer_p, Vector2 pos, Vector2 size, Vector2 facingV, TextureHandleT textureHandle, Vector3 color = VECTOR3_ONE);
void PushUiRect(Renderer* renderer_p, Rect rect, TextureHandleT textureHandle, Vector3 color = VECTOR3_ONE);
void PushText(Renderer* renderer_p, const char* text, Vector2 pos, Vector3 color = VECTOR3_ONE);
void PushRect(Renderer* renderer_p, Rect rect, Vector3 color, Vector2 facingV  = VECTOR2_UP);
void PushLine(Renderer* renderer_p, Vector2 startPos, Vector2 endPos, Vector3 color, float thickness = 0.1f);
void PushCircle(Renderer* renderer_p, Vector2 centerPos, float radius, Vector3 color, int edges = 16);
void PushVector(Renderer* renderer_p, Vector2 pos, Vector2 v, Vector3 color = VECTOR3_ONE);