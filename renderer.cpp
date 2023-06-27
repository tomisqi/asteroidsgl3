#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <glad/glad.h>
#include <math.h>
#include <stb_truetype.h>
#include "common.h"
#include "vector.h"
#include "renderer.h"

#define NO_TEXTURE 0xffff
#define MAX_QUADS  256

void RendererInit(Renderer* renderer_p)
{
	memset(renderer_p, 0, sizeof(*renderer_p));

	renderer_p->renderCommands.maxVertexCount = MAX_QUADS * 4;
	renderer_p->renderCommands.vertexArray = (TexturedVertex*)malloc(MAX_QUADS * 4 * sizeof(TexturedVertex));
	renderer_p->renderCommands.vertexCount = 0;
	
	renderer_p->renderCommands.maxIndexCount = MAX_QUADS * 6;
	renderer_p->renderCommands.indexArray = (U16*)malloc(MAX_QUADS * 6 * sizeof(U32));
	renderer_p->renderCommands.indexCount = 0;
}

void RendererEndFrame(Renderer* renderer_p)
{
	renderer_p->renderCommands.vertexCount = 0;
	renderer_p->renderCommands.indexCount = 0;
}

void PushSprite(Renderer* renderer_p, Vector2 pos, Vector2 size, Vector2 facingV = V2(0, 1), TextureHandleT textureHandle = 0xffff)
{
	RenderCommands* renderCmds_p = &renderer_p->renderCommands;
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

void PushText(Renderer* renderer_p, char* text, Vector2 pos, stbtt_bakedchar* bakedCharData_p, TextureHandleT textTextureHandle = 0xffff)
{
	while (*text) 
	{
		if (*text >= 32 && *text < 128) 
		{
			RenderCommands* renderCmds_p = &renderer_p->renderCommands;
			assert(renderCmds_p->vertexCount < renderCmds_p->maxVertexCount);
			assert(renderCmds_p->indexCount < renderCmds_p->maxIndexCount);

			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(bakedCharData_p, 512, 512, *text - 32, &pos.x, &pos.y, &q, 1);//1=opengl & d3d10+,0=d3d9
			TexturedVertex* vert_p = &renderCmds_p->vertexArray[renderCmds_p->vertexCount];
			
			// Note: The q.y0 and q.y1 are negative due to the fact that stb assumes y increasing down but we assume the opposite
			vert_p[0].pos = V3(q.x0, -q.y0, 0.0f);
			vert_p[0].color = VECTOR3_ONE;
			vert_p[0].uv = V2(q.s0, q.t0);
			vert_p[0].textureHandle = textTextureHandle;
			vert_p[1].pos = V3(q.x1, -q.y0, 0.0f);
			vert_p[1].color = VECTOR3_ONE;
			vert_p[1].uv = V2(q.s1, q.t0);
			vert_p[1].textureHandle = textTextureHandle;
			vert_p[2].pos = V3(q.x1, -q.y1, 0.0f);
			vert_p[2].color = VECTOR3_ONE;
			vert_p[2].uv = V2(q.s1, q.t1);
			vert_p[2].textureHandle = textTextureHandle;
			vert_p[3].pos = V3(q.x0, -q.y1, 0.0f);
			vert_p[3].color = VECTOR3_ONE;
			vert_p[3].uv = V2(q.s0, q.t1);
			vert_p[3].textureHandle = textTextureHandle;

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

