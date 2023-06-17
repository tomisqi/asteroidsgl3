#pragma once

#include <glad/glad.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "common.h"

#define MAX_SHADERFILE_SIZE 10 * MB

namespace Shaders 
{

static inline bool CompileShader(FILE* fileShader, unsigned int shader)
{
	Buffer buffer = {0};
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

static inline bool LinkShaders(unsigned int shaderProgram, unsigned int vertexShader, unsigned int fragmentShader)
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

static inline unsigned int LoadAndCompileShaders(const char* vsPath, const char* fsPath)
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

static inline void UseShader(unsigned int shaderId)
{
	glUseProgram(shaderId);
}

static inline void SetBool(unsigned int shader, const char* name, bool value)
{
	//glUniform1i(glGetUniformLocation(shader, name, (int)value));
	glUniform1i(glGetUniformLocation(shader, name), (int)value);
}

static inline void SetInt(unsigned int shader, const char* name, int value)
{
	glUniform1i(glGetUniformLocation(shader, name), value);
}

static inline void SetFloat(unsigned int shader, const char* name, float value)
{
	glUniform1f(glGetUniformLocation(shader, name), value);
}

};