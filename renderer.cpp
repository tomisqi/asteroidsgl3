#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <glad/glad.h>
#include <math.h>
#include <stb_truetype.h>
#include "common.h"
#include "vector.h"
#include "renderer.h"
#include "opengl.h"
#include <stdlib.h>

#define MAX_SPRITE_QUADS  (1 << 8)
#define MAX_TEXT_QUADS    (1 << 10)

static U8 ttfBuffer[1 << 20];

static RenderGroup CreateRendererGroup(RenderGroupTypeE rendererGroupType, int shaderProgram, int maxQuads);
static RenderGroup* FindRenderGroup(Renderer* renderer_p, RenderGroupTypeE rendererGroupType);

void RendererInit(Renderer* renderer_p)
{
	memset(renderer_p, 0, sizeof(*renderer_p));

	//
	// Render groups
	//

	GLuint spriteShaderProgram = LoadAndCompileShaders("../shaders/vertex_shader.vs", "../shaders/sprites_shader.fs"); 
	assert(spriteShaderProgram >= 0);
	renderer_p->renderGroups[0] = CreateRendererGroup(RENDER_GROUP_SPRITES_DEFAULT, spriteShaderProgram, MAX_SPRITE_QUADS);

	GLuint textShaderProgram = LoadAndCompileShaders("../shaders/vertex_shader.vs", "../shaders/text_shader.fs"); 
	assert(textShaderProgram >= 0);
	renderer_p->renderGroups[1] = CreateRendererGroup(RENDER_GROUP_TEXT_DEFAULT, textShaderProgram, MAX_TEXT_QUADS);

	renderer_p->groupCnt = 2;

	//
	// Text Textures
	// 

	Texture* textTexture_p = &renderer_p->textRendering.textTexture;
	textTexture_p->data_p = (U8*)malloc(512 * 512);
	textTexture_p->width = 512;
	textTexture_p->height = 512;

	fread(ttfBuffer, 1, 1 << 20, fopen("C:/Windows/Fonts/consola.ttf", "rb"));
	stbtt_BakeFontBitmap(ttfBuffer, 0, 16.0, textTexture_p->data_p, 512, 512, 32, 96, renderer_p->textRendering.charUvData);
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

void PushSprite(Renderer* renderer_p, Vector2 pos, Vector2 size, Vector2 facingV = V2(0, 1), TextureHandleT textureHandle = 0xffff)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_SPRITES_DEFAULT);
	assert(rendGrp_p);

	RenderCommands* renderCmds_p = &rendGrp_p->renderCommands;
	assert(renderCmds_p->vertexCount < renderCmds_p->maxVertexCount);
	assert(renderCmds_p->indexCount < renderCmds_p->maxIndexCount);

	float angleRad = atan2(-facingV.x, facingV.y);
	Vector2 MaxXMaxY = pos + RotateRad(V2( 0.5f * size.x,  0.5 * size.y), angleRad);
	Vector2 MaxXMinY = pos + RotateRad(V2( 0.5f * size.x, -0.5 * size.y), angleRad);
	Vector2 MinXMinY = pos + RotateRad(V2(-0.5f * size.x, -0.5 * size.y), angleRad);
	Vector2 MinXMaxY = pos + RotateRad(V2(-0.5f * size.x,  0.5 * size.y), angleRad);

	TexturedVertex* vert_p = &renderCmds_p->vertexArray[renderCmds_p->vertexCount];
	vert_p[0].pos = V3(MaxXMaxY);
	vert_p[0].color = VECTOR3_ONE;
	vert_p[0].uv = V2(1.0f, 1.0f);
	vert_p[0].textureHandle = textureHandle;
	vert_p[1].pos = V3(MaxXMinY);
	vert_p[1].color = VECTOR3_ONE;
	vert_p[1].uv = V2(1.0f, 0.0f);
	vert_p[1].textureHandle = textureHandle;
	vert_p[2].pos = V3(MinXMinY);
	vert_p[2].color = VECTOR3_ONE;
	vert_p[2].uv = V2(0.0f, 0.0f);
	vert_p[2].textureHandle = textureHandle;
	vert_p[3].pos = V3(MinXMaxY);
	vert_p[3].color = VECTOR3_ONE;
	vert_p[3].uv = V2(0.0f, 1.0f);
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

void PushText(Renderer* renderer_p, char* text, Vector2 pos)
{
	RenderGroup* rendGrp_p = FindRenderGroup(renderer_p, RENDER_GROUP_TEXT_DEFAULT);
	assert(rendGrp_p);

	stbtt_bakedchar* bakedCharData_p = renderer_p->textRendering.charUvData;

	while (*text) 
	{
		if (*text >= 32 && *text < 128) 
		{
			RenderCommands* renderCmds_p = &rendGrp_p->renderCommands;
			assert(renderCmds_p->vertexCount < renderCmds_p->maxVertexCount);
			assert(renderCmds_p->indexCount < renderCmds_p->maxIndexCount);

			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(bakedCharData_p, 512, 512, *text - 32, &pos.x, &pos.y, &q, 1);//1=opengl & d3d10+,0=d3d9
			TexturedVertex* vert_p = &renderCmds_p->vertexArray[renderCmds_p->vertexCount];
			
			// Note: The q.y0 and q.y1 are negative due to the fact that stb assumes y increasing down but we assume the opposite
			vert_p[0].pos = V3(q.x0, -q.y0, 0.0f);
			vert_p[0].color = VECTOR3_ONE;
			vert_p[0].uv = V2(q.s0, q.t0);
			vert_p[1].pos = V3(q.x1, -q.y0, 0.0f);
			vert_p[1].color = VECTOR3_ONE;
			vert_p[1].uv = V2(q.s1, q.t0);
			vert_p[2].pos = V3(q.x1, -q.y1, 0.0f);
			vert_p[2].color = VECTOR3_ONE;
			vert_p[2].uv = V2(q.s1, q.t1);
			vert_p[3].pos = V3(q.x0, -q.y1, 0.0f);
			vert_p[3].color = VECTOR3_ONE;
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
		++text;
	}
}


static RenderGroup CreateRendererGroup(RenderGroupTypeE rendererGroupType, int shaderProgram, int maxQuads)
{
	RenderGroup rendGrp;	
	rendGrp.renderGroupType = rendererGroupType;
	rendGrp.shaderProgram = shaderProgram;
	rendGrp.renderCommands.maxVertexCount = maxQuads * 4;
	rendGrp.renderCommands.vertexArray = (TexturedVertex*)malloc(maxQuads * 4 * sizeof(TexturedVertex));
	rendGrp.renderCommands.vertexCount = 0;
	rendGrp.renderCommands.maxIndexCount = maxQuads * 6;
	rendGrp.renderCommands.indexArray = (U16*)malloc(maxQuads * 6 * sizeof(U32));
	rendGrp.renderCommands.indexCount = 0;

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
