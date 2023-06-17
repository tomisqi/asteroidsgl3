#pragma once

#include <glad/glad.h>
#include "texture.h"
#include "renderer.h"

struct OpenGL
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint texture;
};

void OpenGLInit(OpenGL* openGL_p);
GLuint LoadAndCompileShaders(const char* vsPath, const char* fsPath);
void UseShader(unsigned int shaderId);
void OpenGLEndFrame(OpenGL* openGl_p, RenderCommands* renderCmds_p, Texture textures[], Vector2 screenDim, GLuint shaderProgram);
