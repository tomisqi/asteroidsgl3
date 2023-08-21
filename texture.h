#pragma once
#include "common.h"

#define TEXTURE_SPACECRAFT 0
#define TEXTURE_REDSHOT    1
#define TEXTURE_ELI        2
#define TEXTURE_ASTEROID   3
#define TEXTURES_COUNT     4

struct Texture
{
	int width;
	int height;
	int nrChannels;
	U8* data_p;
};

Texture LoadTexture(const char* filepath);