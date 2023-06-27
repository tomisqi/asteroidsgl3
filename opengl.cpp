#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "opengl.h"
#include "vector.h"	

#define MAX_SHADERFILE_SIZE 10 * MB

static bool CompileShader(FILE* fileShader, unsigned int shader)
{
	Buffer buffer = { 0 };
	buffer.buf_p = (U8*)malloc(MAX_SHADERFILE_SIZE); memset(buffer.buf_p, 0, MAX_SHADERFILE_SIZE);
	buffer.bufSize = fread(buffer.buf_p, 1, MAX_SHADERFILE_SIZE, fileShader); assert(buffer.bufSize <= MAX_SHADERFILE_SIZE);
	const char* sourcecode = (const char*)buffer.buf_p;
	glShaderSource(shader, 1, &sourcecode, NULL);
	glCompileShader(shader);

	// Check for shader compile errors
	int compileSuccess;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
	if (!compileSuccess)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		return false;
	}

	free(buffer.buf_p);
	return true;
}

static bool LinkShaders(unsigned int shaderProgram, unsigned int vertexShader, unsigned int fragmentShader)
{
	// Link shaders
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check for linking errors
	int linkSuccess;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
		return false;
	}
	return true;
}

GLuint LoadAndCompileShaders(const char* vsPath, const char* fsPath)
{
	// Open shader files
	FILE* fileVs = fopen(vsPath, "rb"); if (!fileVs) { printf("ERROR: Could not open shader %s\n", vsPath); return -1; }
	FILE* fileFs = fopen(fsPath, "rb"); if (!fileFs) { printf("ERROR: Could not open shader %s\n", fsPath); return -1; }

	// Create and compile shaders
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!CompileShader(fileVs, vertexShader)) { printf("ERROR::SHADER::VERTEX COMPILATION_FAILED\n"); return -1; }
	if (!CompileShader(fileFs, fragmentShader)) { printf("ERROR::SHADER::FRAGMENT COMPILATION_FAILED\n"); return -1; }

	// Link shaders
	unsigned int shaderProgram = glCreateProgram();
	if (!LinkShaders(shaderProgram, vertexShader, fragmentShader)) return -1;

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

void UseShader(unsigned int shaderId)
{
	glUseProgram(shaderId);
}

void SetBool(unsigned int shader, const char* name, bool value)
{
	//glUniform1i(glGetUniformLocation(shader, name, (int)value));
	glUniform1i(glGetUniformLocation(shader, name), (int)value);
}

void SetInt(unsigned int shader, const char* name, int value)
{
	glUniform1i(glGetUniformLocation(shader, name), value);
}

void SetFloat(unsigned int shader, const char* name, float value)
{
	glUniform1f(glGetUniformLocation(shader, name), value);
}

void OpenGLInit(OpenGL* openGL_p)
{
	glGenVertexArrays(1, &openGL_p->VAO);
	glGenBuffers(1, &openGL_p->VBO);
	glGenBuffers(1, &openGL_p->EBO);
	glBindVertexArray(openGL_p->VAO);

	glGenTextures(1, &openGL_p->texture);
	glBindTexture(GL_TEXTURE_2D, openGL_p->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Set texture wrapping.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);      // Set texture filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void OpenGLEndFrame(OpenGL* openGl_p, Renderer* renderer_p, Texture textures[], Vector2 screenDim)
{
	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < renderer_p->groupCnt; i++)
	{
		RenderGroup* rendGrp_p = &renderer_p->renderGroups[i];
		UseShader(rendGrp_p->shaderProgram);

		glm::mat4 transMatrix = glm::mat4(1.0f);
		transMatrix = glm::scale(transMatrix, glm::vec3(2.0f / screenDim.x, 2.0f / screenDim.y, 1.0f));
		unsigned int transformLoc = glGetUniformLocation(rendGrp_p->shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transMatrix));

		RenderCommands* renderCmds_p = &rendGrp_p->renderCommands;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, openGl_p->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderCmds_p->indexCount * sizeof(U32), renderCmds_p->indexArray, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, openGl_p->VBO);
		glBufferData(GL_ARRAY_BUFFER, renderCmds_p->vertexCount * sizeof(TexturedVertex), renderCmds_p->vertexArray, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)OFFSET_OF(TexturedVertex, pos)); // position attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)OFFSET_OF(TexturedVertex, color)); // color attribute
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)OFFSET_OF(TexturedVertex, uv)); // uv attribute
		glEnableVertexAttribArray(2);

		switch (rendGrp_p->renderGroupType)
		{
		case RENDER_GROUP_SPRITES_DEFAULT:
		{
			TextureHandleT textureHandle = -1;
			int quadCount = renderCmds_p->vertexCount / 4;
			int indexIndex = 0;
			for (int quadIndex = 0; quadIndex < quadCount; quadIndex++)
			{
				glBindTexture(GL_TEXTURE_2D, openGl_p->texture);
				TexturedVertex* vert_p = &renderCmds_p->vertexArray[quadIndex * 4];
				if (vert_p->textureHandle != textureHandle)
				{
					textureHandle = vert_p->textureHandle;
					Texture* texture_p = &textures[textureHandle];
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_p->width, texture_p->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_p->data_p);
				}
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawElementsBaseVertex(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)(indexIndex * sizeof(U16)), 0);
				indexIndex += 6;
			}
		}
			break;
		case RENDER_GROUP_TEXT_DEFAULT:
		{
			Texture* textTexture_p = &renderer_p->textRendering.textTexture;
			int quadCount = renderCmds_p->vertexCount / 4;
			int indexIndex = 0;
			for (int quadIndex = 0; quadIndex < quadCount; quadIndex++)
			{
				glBindTexture(GL_TEXTURE_2D, openGl_p->texture);
				TexturedVertex* vert_p = &renderCmds_p->vertexArray[quadIndex * 4];
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textTexture_p->width, textTexture_p->height, 0, GL_RED, GL_UNSIGNED_BYTE, textTexture_p->data_p);
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawElementsBaseVertex(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)(indexIndex * sizeof(U16)), 0);
				indexIndex += 6;
			}
		}
			break;
		default:
			assert(false);
			break;
		};


	}
}
