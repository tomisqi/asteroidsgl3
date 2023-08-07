#pragma once
#include "common.h"
#include "vector.h"
#include "texture.h"
#include "rect.h"
#include "color.h"
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
	Color color;
	Vector2 uv;
	TextureHandleT textureHandle;
};

struct ColoredVertex
{
	Vector3 pos;
	Color color;
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

struct TextCursor
{
	int index;
	float xpos;
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
void PushSprite(Renderer* renderer_p, Vector2 pos, Vector2 size, Vector2 facingV, TextureHandleT textureHandle, Color color = COLOR_WHITE);
void PushUiRect(Renderer* renderer_p, Rect rect, Color color);
void PushText(Renderer* renderer_p, const char* text, Vector2 pos, Color color, TextCursor* textCursor_p = nullptr);
void PushRect(Renderer* renderer_p, Rect rect, Color color, Vector2 facingV  = VECTOR2_UP);
void PushLine(Renderer* renderer_p, Vector2 startPos, Vector2 endPos, Color color, float thickness = 0.1f);
void PushCircle(Renderer* renderer_p, Vector2 centerPos, float radius, Color color, int edges = 16);
void PushVector(Renderer* renderer_p, Vector2 pos, Vector2 v, Color color = COLOR_WHITE);