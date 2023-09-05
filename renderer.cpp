#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <glad/glad.h>
#include <math.h>
#include <stb_truetype.h>
#include <stdlib.h>
#include "common.h"
#include "vector.h"
#include "renderer.h"
#include "opengl.h"
#include "rect.h"
#include "color.h"

#define MAX_SPRITE_QUADS    (1 << 8)
#define MAX_TEXT_QUADS      (1 << 8)
#define MAX_UI_QUADS        (1 << 8)
#define MAX_WIREFRAME_QUADS (1 << 12)

static U8 ttfBuffer[1 << 20];

static RenderGroup CreateRendererGroup(RenderGroupTypeE rendererGroupType, int shaderProgram, int maxQuads, bool onlyColored = false);
static RenderGroup* FindRenderGroup(Renderer* renderer_p, RenderGroupTypeE rendererGroupType);

Renderer* rendererGl_p = nullptr;

void RendererInit(Renderer* renderer_p)
{
	memset(renderer_p, 0, sizeof(*renderer_p));
	rendererGl_p = renderer_p;

	//
	// Render groups
	//
	int wireframeShaderProgram = LoadAndCompileShaders("../shaders/wireframe_shader.vs", "../shaders/wireframe_shader.fs");
	assert(wireframeShaderProgram >= 0);
	renderer_p->renderGroups[0] = CreateRendererGroup(RENDER_GROUP_WIREFRAME, wireframeShaderProgram, MAX_WIREFRAME_QUADS, true);

	int spriteShaderProgram = LoadAndCompileShaders("../shaders/vertex_shader.vs", "../shaders/sprites_shader.fs");
	assert(spriteShaderProgram >= 0);
	renderer_p->renderGroups[1] = CreateRendererGroup(RENDER_GROUP_SPRITES_DEFAULT, spriteShaderProgram, MAX_SPRITE_QUADS);

	int uiShaderProgram = LoadAndCompileShaders("../shaders/wireframe_shader.vs", "../shaders/wireframe_shader.fs");
	assert(uiShaderProgram >= 0);
	renderer_p->renderGroups[2] = CreateRendererGroup(RENDER_GROUP_UI, uiShaderProgram, MAX_UI_QUADS, true);

	int textShaderProgram = LoadAndCompileShaders("../shaders/vertex_shader.vs", "../shaders/text_shader.fs");
	assert(textShaderProgram >= 0);
	renderer_p->renderGroups[3] = CreateRendererGroup(RENDER_GROUP_TEXT_DEFAULT, textShaderProgram, MAX_TEXT_QUADS);

	renderer_p->groupCnt = 4;

	//
	// Text Textures
	// 
	Texture* textTexture_p = &renderer_p->textRendering.textTexture;
	textTexture_p->data_p = (U8*)malloc(512 * 512);
	textTexture_p->width = 512;
	textTexture_p->height = 512;

	fread(ttfBuffer, 1, 1 << 20, fopen("C:/Windows/Fonts/consola.ttf", "rb"));
	stbtt_BakeFontBitmap(ttfBuffer, 0, 16.0f, textTexture_p->data_p, 512, 512, 32, 96, renderer_p->textRendering.charUvData);

	assert(renderer_p->groupCnt < MAX_RENDER_GROUPS);
}

void RendererEndFrame(Renderer* renderer_p)
{
	for (int i = 0; i < renderer_p->groupCnt; i++)
	{
		RenderGroup* rendGrp_p = &renderer_p->renderGroups[i];
		rendGrp_p->renderCommands.vertexCount = 0;
		rendGrp_p->renderCommands.indexCount = 0;
	}
}

static void SetOrtographicProj(RenderGroup* rendGrp_p, Rect rect)
{
	Vector2 max = RectMaxXMaxY(rect);
	Vector2 min = RectMinXMinY(rect);
	rendGrp_p->ortoProj.xmin = min.x;
	rendGrp_p->ortoProj.ymin = min.y;
	rendGrp_p->ortoProj.xmax = max.x;
	rendGrp_p->ortoProj.ymax = max.y;
}

void SetSpritesOrtographicProj(Renderer* renderer_p, Rect rect)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_SPRITES_DEFAULT);
	assert(rendGrp_p);
	SetOrtographicProj(rendGrp_p, rect);
}

void SetWireframeOrtographicProj(Renderer* renderer_p, Rect rect)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_WIREFRAME);
	assert(rendGrp_p);
	SetOrtographicProj(rendGrp_p, rect);
}

void PushSprite(Renderer* renderer_p, Vector2 pos, Vector2 size, Vector2 facingV, TextureHandleT textureHandle, Color color, Rect uvRect)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_SPRITES_DEFAULT);
	assert(rendGrp_p);

	RenderCommands* renderCmds_p = &rendGrp_p->renderCommands;
	assert(renderCmds_p->vertexCount < renderCmds_p->maxVertexCount);
	assert(renderCmds_p->indexCount < renderCmds_p->maxIndexCount);

	float angleRad = atan2(-facingV.x, facingV.y);
	Vector2 MaxXMaxY = pos + RotateRad(V2( 0.5f * size.x,  0.5f * size.y), angleRad);
	Vector2 MaxXMinY = pos + RotateRad(V2( 0.5f * size.x, -0.5f * size.y), angleRad);
	Vector2 MinXMinY = pos + RotateRad(V2(-0.5f * size.x, -0.5f * size.y), angleRad);
	Vector2 MinXMaxY = pos + RotateRad(V2(-0.5f * size.x,  0.5f * size.y), angleRad);

	TexturedVertex* vert_p = &renderCmds_p->vertexArray[renderCmds_p->vertexCount];
	vert_p[0].pos = V3(MaxXMaxY);
	vert_p[0].color = color;
	vert_p[0].uv = RectMaxXMaxY(uvRect);
	vert_p[0].textureHandle = textureHandle;
	vert_p[1].pos = V3(MaxXMinY);
	vert_p[1].color = color;
	vert_p[1].uv = RectMaxXMinY(uvRect);
	vert_p[1].textureHandle = textureHandle;
	vert_p[2].pos = V3(MinXMinY);
	vert_p[2].color = color;
	vert_p[2].uv = RectMinXMinY(uvRect);
	vert_p[2].textureHandle = textureHandle;
	vert_p[3].pos = V3(MinXMaxY);
	vert_p[3].color = color;
	vert_p[3].uv = RectMinXMaxY(uvRect);
	vert_p[3].textureHandle = textureHandle;

	U16 baseIndex = renderCmds_p->vertexCount;
	U16* index_p = &renderCmds_p->indexArray[renderCmds_p->indexCount];
	index_p[0] = baseIndex + 0;
	index_p[1] = baseIndex + 1;
	index_p[2] = baseIndex + 3;
	index_p[3] = baseIndex + 1;
	index_p[4] = baseIndex + 2;
	index_p[5] = baseIndex + 3;

	renderCmds_p->vertexCount += 4;
	renderCmds_p->indexCount += 6;
}

void PushUiRect(Renderer* renderer_p, Rect rect, Color color)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_UI);
	assert(rendGrp_p);

	RenderCommands* renderCmds_p = &rendGrp_p->renderCommands;
	assert(renderCmds_p->vertexCount < renderCmds_p->maxVertexCount);
	assert(renderCmds_p->indexCount < renderCmds_p->maxIndexCount);

	Vector2 facingV = VECTOR2_UP; //@nocommit

	float angleRad = atan2(-facingV.x, facingV.y);
	Vector2 rectCenter = GetRectCenter(rect); // Rotates around center
	Vector2 MaxXMaxY = rectCenter + RotateRad(V2(0.5f * rect.size.x, 0.5f * rect.size.y), angleRad);
	Vector2 MaxXMinY = rectCenter + RotateRad(V2(0.5f * rect.size.x, -0.5f * rect.size.y), angleRad);
	Vector2 MinXMinY = rectCenter + RotateRad(V2(-0.5f * rect.size.x, -0.5f * rect.size.y), angleRad);
	Vector2 MinXMaxY = rectCenter + RotateRad(V2(-0.5f * rect.size.x, 0.5f * rect.size.y), angleRad);

	ColoredVertex* vert_p = &renderCmds_p->onlyColoredVertexArray[renderCmds_p->vertexCount];
	vert_p[0].pos = V3(MaxXMaxY);
	vert_p[0].color = color;
	vert_p[1].pos = V3(MaxXMinY);
	vert_p[1].color = color;
	vert_p[2].pos = V3(MinXMinY);
	vert_p[2].color = color;
	vert_p[3].pos = V3(MinXMaxY);
	vert_p[3].color = color;

	U16 baseIndex = renderCmds_p->vertexCount;
	U16* index_p = &renderCmds_p->indexArray[renderCmds_p->indexCount];
	index_p[0] = baseIndex + 0;
	index_p[1] = baseIndex + 1;
	index_p[2] = baseIndex + 3;
	index_p[3] = baseIndex + 1;
	index_p[4] = baseIndex + 2;
	index_p[5] = baseIndex + 3;

	renderCmds_p->vertexCount += 4;
	renderCmds_p->indexCount += 6;
}

float GetCharPosX(stbtt_bakedchar* bakedCharData_p, float startPosX, const char* text, int charIdx)
{
	float posX = startPosX;
	float posY = 0; // Doesn't matter.
	int idx = 0;
	while (*text)
	{
		if (*text >= 32 && *text < 128)
		{
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(bakedCharData_p, 512, 512, *text - 32, &posX, &posY, &q, 1);//1=opengl & d3d10+,0=d3d9
			if ((idx == (charIdx - 1))) return posX;
		}
		++idx;
		++text;
	}
	return startPosX;
}

void PushText(Renderer* renderer_p, const char* text, Vector2 pos, Color color, float maxX)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_TEXT_DEFAULT);
	assert(rendGrp_p);

	stbtt_bakedchar* bakedCharData_p = renderer_p->textRendering.charUvData;

	int idx = 0;
	while (*text && (pos.x < maxX))
	{
		if (*text >= 32 && *text < 128) 
		{
			RenderCommands* renderCmds_p = &rendGrp_p->renderCommands;
			assert(renderCmds_p->vertexCount < renderCmds_p->maxVertexCount);
			assert(renderCmds_p->indexCount < renderCmds_p->maxIndexCount);

			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(bakedCharData_p, 512, 512, *text - 32, &pos.x, &pos.y, &q, 1);//1=opengl & d3d10+,0=d3d9
			TexturedVertex* vert_p = &renderCmds_p->vertexArray[renderCmds_p->vertexCount];
			
			// Note this is flipped but we fix it in the shader.
            vert_p[0].pos = V3(q.x0, q.y0, 0.0f);
 			vert_p[0].color = color;
			vert_p[0].uv = V2(q.s0, q.t0);
			vert_p[1].pos = V3(q.x1, q.y0, 0.0f);
			vert_p[1].color = color;
			vert_p[1].uv = V2(q.s1, q.t0);
			vert_p[2].pos = V3(q.x1, q.y1, 0.0f);
			vert_p[2].color = color;
			vert_p[2].uv = V2(q.s1, q.t1);
			vert_p[3].pos = V3(q.x0, q.y1, 0.0f);
			vert_p[3].color = color;
			vert_p[3].uv = V2(q.s0, q.t1);

			U16 baseIndex = renderCmds_p->vertexCount;
			U16* index_p = &renderCmds_p->indexArray[renderCmds_p->indexCount];
			index_p[0] = baseIndex + 0;
			index_p[1] = baseIndex + 1;
			index_p[2] = baseIndex + 3;
			index_p[3] = baseIndex + 1;
			index_p[4] = baseIndex + 2;
			index_p[5] = baseIndex + 3;

			renderCmds_p->vertexCount += 4;
			renderCmds_p->indexCount += 6;
		}
		++idx;
		++text;
	}
}

void PushRect(Renderer* renderer_p, Rect rect, Color color, Vector2 facingV)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_WIREFRAME);
	assert(rendGrp_p);

	RenderCommands* renderCmds_p = &rendGrp_p->renderCommands;
	assert(renderCmds_p->vertexCount < renderCmds_p->maxVertexCount);
	assert(renderCmds_p->indexCount < renderCmds_p->maxIndexCount);

	float angleRad = atan2(-facingV.x, facingV.y);
	Vector2 rectCenter = GetRectCenter(rect); // Rotates around center
	Vector2 MaxXMaxY = rectCenter + RotateRad(V2(0.5f * rect.size.x, 0.5f * rect.size.y), angleRad);
	Vector2 MaxXMinY = rectCenter + RotateRad(V2(0.5f * rect.size.x, -0.5f * rect.size.y), angleRad);
	Vector2 MinXMinY = rectCenter + RotateRad(V2(-0.5f * rect.size.x, -0.5f * rect.size.y), angleRad);
	Vector2 MinXMaxY = rectCenter + RotateRad(V2(-0.5f * rect.size.x, 0.5f * rect.size.y), angleRad);

	ColoredVertex* vert_p = &renderCmds_p->onlyColoredVertexArray[renderCmds_p->vertexCount];
	vert_p[0].pos = V3(MaxXMaxY);
	vert_p[0].color = color;
	vert_p[1].pos = V3(MaxXMinY);
	vert_p[1].color = color;
	vert_p[2].pos = V3(MinXMinY);
	vert_p[2].color = color; 
	vert_p[3].pos = V3(MinXMaxY);
	vert_p[3].color = color; 

	U16 baseIndex = renderCmds_p->vertexCount;
	U16* index_p = &renderCmds_p->indexArray[renderCmds_p->indexCount];
	index_p[0] = baseIndex + 0;
	index_p[1] = baseIndex + 1;
	index_p[2] = baseIndex + 3;
	index_p[3] = baseIndex + 1;
	index_p[4] = baseIndex + 2;
	index_p[5] = baseIndex + 3;

	renderCmds_p->vertexCount += 4;
	renderCmds_p->indexCount += 6;
}

void PushLine(Renderer* renderer_p, Vector2 startPos, Vector2 endPos, Color color, float thickness)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_WIREFRAME);
	assert(rendGrp_p);

	RenderCommands* renderCmds_p = &rendGrp_p->renderCommands;
	assert(renderCmds_p->vertexCount < renderCmds_p->maxVertexCount);
	assert(renderCmds_p->indexCount < renderCmds_p->maxIndexCount);

	Vector2 v = Normalize(endPos - startPos);
	Vector2 MaxXMaxY = endPos   + thickness * RotateDeg(v, +90);
	Vector2 MaxXMinY = endPos   + thickness * RotateDeg(v, -90);
	Vector2 MinXMinY = startPos + thickness * RotateDeg(-v, +90);
	Vector2 MinXMaxY = startPos + thickness * RotateDeg(-v, -90);

	ColoredVertex* vert_p = &renderCmds_p->onlyColoredVertexArray[renderCmds_p->vertexCount];
	vert_p[0].pos = V3(MaxXMaxY);
	vert_p[0].color = color; 
	vert_p[1].pos = V3(MaxXMinY);
	vert_p[1].color = color; 
	vert_p[2].pos = V3(MinXMinY);
	vert_p[2].color = color; 
	vert_p[3].pos = V3(MinXMaxY);
	vert_p[3].color = color; 

	U16 baseIndex = renderCmds_p->vertexCount;
	U16* index_p = &renderCmds_p->indexArray[renderCmds_p->indexCount];
	index_p[0] = baseIndex + 0;
	index_p[1] = baseIndex + 1;
	index_p[2] = baseIndex + 3;
	index_p[3] = baseIndex + 1;
	index_p[4] = baseIndex + 2;
	index_p[5] = baseIndex + 3;

	renderCmds_p->vertexCount += 4;
	renderCmds_p->indexCount += 6;
}

#define ARROW_LEG_LENGTH 10
void PushVector(Renderer* renderer_p, Vector2 pos, Vector2 v, Color color)
{
	Vector2 normV = Normalize(v);
	Vector2 leg1V = RotateDeg(-normV, -25);
	Vector2 leg2V = RotateDeg(-normV, +25);

	Vector2 tip = pos + v;
	PushLine(renderer_p, pos, tip, color);
	PushLine(renderer_p, tip, tip + ARROW_LEG_LENGTH * leg1V, color);
	PushLine(renderer_p, tip, tip + ARROW_LEG_LENGTH * leg2V, color);
}

#define XCROSS_SIZE 10
void PushXCross(Renderer* renderer_p, Vector2 pos, Color color)
{
	Vector2 p1; 
	Vector2 p2;

	p1 = pos - XCROSS_SIZE * VECTOR2_ONE;
	p2= pos + XCROSS_SIZE * VECTOR2_ONE;
	PushLine(renderer_p, p1, p2, color);

	p1 = pos + V2(-XCROSS_SIZE, XCROSS_SIZE);
	p2 = pos + V2(XCROSS_SIZE, -XCROSS_SIZE);
	PushLine(renderer_p, p1, p2, color);
}

void PushCircle(Renderer* renderer_p, Vector2 centerPos, float radius, Color color, int edges)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_WIREFRAME);
	assert(rendGrp_p);

	RenderCommands* renderCmds_p = &rendGrp_p->renderCommands;

	float deltaAngle = 360.0f/edges;
	Vector2 radialV = VECTOR2_RIGHT;
	for (int i = 0; i < edges; i++)
	{
		assert(renderCmds_p->vertexCount < renderCmds_p->maxVertexCount);
		assert(renderCmds_p->indexCount < renderCmds_p->maxIndexCount);

		Vector2 pos1 = centerPos + radius * radialV;
		radialV = RotateDeg(radialV, deltaAngle);
		Vector2 pos2 = centerPos + radius * radialV;

		ColoredVertex* vert_p = &renderCmds_p->onlyColoredVertexArray[renderCmds_p->vertexCount];
		vert_p[0].pos = V3(centerPos);
		vert_p[0].color = Col(color.r, color.g, color.b, 0.0f);
		vert_p[1].pos = V3(pos1);
		vert_p[1].color = color;
		vert_p[2].pos = V3(pos2);
		vert_p[2].color = color;

		U16 baseIndex = renderCmds_p->vertexCount;
		U16* index_p = &renderCmds_p->indexArray[renderCmds_p->indexCount];
		index_p[0] = baseIndex + 0;
		index_p[1] = baseIndex + 1;
		index_p[2] = baseIndex + 2;

		renderCmds_p->vertexCount += 3;
		renderCmds_p->indexCount += 3;
	}
}

static RenderGroup CreateRendererGroup(RenderGroupTypeE rendererGroupType, int shaderProgram, int maxQuads, bool onlyColored)
{
	RenderGroup rendGrp = {0};
	rendGrp.renderGroupType = rendererGroupType;
	rendGrp.shaderProgram = shaderProgram;
	rendGrp.renderCommands.maxIndexCount = maxQuads * 6;
	rendGrp.renderCommands.indexArray = (U16*)malloc(maxQuads * 6 * sizeof(U32));
	rendGrp.renderCommands.indexCount = 0;

	rendGrp.renderCommands.maxVertexCount = maxQuads * 4;
	rendGrp.renderCommands.vertexCount = 0;
	if (onlyColored)
	{
		rendGrp.renderCommands.onlyColoredVertexArray = (ColoredVertex*)malloc(maxQuads * 4 * sizeof(ColoredVertex));
	}
	else
	{
		rendGrp.renderCommands.vertexArray = (TexturedVertex*)malloc(maxQuads * 4 * sizeof(TexturedVertex));
	}

	return rendGrp;
}

static RenderGroup* FindRenderGroup(Renderer* renderer_p, RenderGroupTypeE rendererGroupType)
{
	for (size_t i = 0; i < renderer_p->groupCnt; i++)
	{
		if (renderer_p->renderGroups[i].renderGroupType == rendererGroupType) return &renderer_p->renderGroups[i];
	}
	return nullptr;
}
