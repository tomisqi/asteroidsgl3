#pragma once
#include "common.h"

struct Texture
{
	int width;
	int height;
	int nrChannels;
	U8* data;
};

Texture LoadTexture(const char* filepath);