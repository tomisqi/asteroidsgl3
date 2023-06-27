#include <stb_image.h>
#include <assert.h>
#include "texture.h"

Texture LoadTexture(const char* filepath)
{
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

	Texture tex = { 0 };
	tex.data_p = stbi_load(filepath, &tex.width, &tex.height, &tex.nrChannels, 0);
	assert(tex.data_p);
	assert(tex.nrChannels == 3 || tex.nrChannels == 4);

	return tex;
}