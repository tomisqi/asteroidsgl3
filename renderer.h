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

struct Renderer
{
	int groupCnt;
	RenderGroup renderGroups[MAX_RENDER_GROUPS];

	TextRendering textRendering;
};

extern Renderer* rendererGl_p; // Used for debugging.

void RendererInit(Renderer* renderer_p);
void RendererEndFrame(Renderer* renderer_p);
void SetSpritesOrtographicProj(Renderer* renderer_p, Rect rect);
void SetWireframeOrtographicProj(Renderer* renderer_p, Rect rect);
void PushSprite(Renderer* renderer_p, Vector2 pos, Vector2 size, Vector2 facingV, TextureHandleT textureHandle, Color color = COLOR_WHITE, Rect uvRect = RECT_ONE);
void PushUiRect(Renderer* renderer_p, Rect rect, Color color);
void PushUiRect01(Renderer* renderer_p, Rect rect01, Color color);
void PushText(Renderer* renderer_p, const char* text, Vector2 pos, Color color, float maxX = F32_MAX);
void PushText01(Renderer* renderer_p, const char* text, Vector2 pos01, Color color);
void PushRect(Renderer* renderer_p, Rect rect, Color color, Vector2 facingV  = VECTOR2_UP);
void PushLine(Renderer* renderer_p, Vector2 startPos, Vector2 endPos, Color color, float thickness = 0.1f);
void PushCircle(Renderer* renderer_p, Vector2 centerPos, float radius, Color color, int edges = 16);
void PushVector(Renderer* renderer_p, Vector2 pos, Vector2 v, Color color = COLOR_WHITE);
void PushXCross(Renderer* renderer_p, Vector2 pos, Color color);
float GetCharPosX(stbtt_bakedchar* bakedCharData_p, float startPosX, const char* text, int charIdx);
float GetTextWidth(Renderer* renderer_p, const char* text);

extern Vector2 ScreenDim;
static inline Vector2 Coord01ToScreenCoordText(Vector2 coord01)
{
	Vector2 screenCoord = { -ScreenDim.x / 2 + coord01.x * ScreenDim.x, ScreenDim.y / 2 - coord01.y * ScreenDim.y };
	return screenCoord;
}
static inline Vector2 Coord01ToScreenCoord(Vector2 coord01)
{
	Vector2 screenCoord = { -ScreenDim.x / 2 + coord01.x * ScreenDim.x, -ScreenDim.y / 2 + coord01.y * ScreenDim.y };
	return screenCoord;
}
static inline Vector2 Size01ToScreenSize(Vector2 size01)
{
	Vector2 screenSize = size01 * ScreenDim;
	return screenSize;
}
static inline Vector2 ScreenSizeToSize01(Vector2 screenSize)
{
	return V2(screenSize.x / ScreenDim.x, screenSize.y / ScreenDim.y);
}