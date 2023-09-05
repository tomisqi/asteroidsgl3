#pragma once
#include "common.h"

#define TEXTURE_SPACECRAFT    0
#define TEXTURE_REDSHOT       1
#define TEXTURE_ELI           2
#define TEXTURE_ASTEROID      3
#define TEXTURE_CHARGEDBULLET 4
#define TEXTURE_SHIPEXHAUST   5
#define TEXTURES_COUNT        6

struct Texture
{
	int width;
	int height;
	int nrChannels;
	U8* data_p;
};

Texture LoadTexture(const char* filepath);