#pragma once

#define COLOR_WHITE		Col(1.0f, 1.0f, 1.0f)
#define COLOR_BLACK		Col(0.0f, 0.0f, 0.0f)
#define COLOR_RED		Col(1.0f, 0.0f, 0.0f)
#define COLOR_GREEN		Col(0.0f, 1.0f, 0.0f)
#define COLOR_BLUE		Col(0.0f, 0.0f, 1.0f)
#define COLOR_YELLOW	Col(1.0f, 1.0f, 0.0f)
#define COLOR_MAGENTA	Col(1.0f, 0.0f, 1.0f)
#define COLOR_BLACK		Col(0.0f, 0.0f, 0.0f)

#define COL32(R,G,B)	(((R & 0xff) << 0) | ((G & 0xff) << 8) | ((B & 0xff) << 16) |  (0xff) << 24)
#define COL32A(R,G,B,A) (((R & 0xff) << 0) | ((G & 0xff) << 8) | ((B & 0xff) << 16) | (A & 0xff) << 24)
#define COL32_WHITE		COL32(255,255,255)
#define COL32_RED		COL32(255,0,0)
#define COL32_GREEN		COL32(0,255,0)
#define COL32_BLUE		COL32(0,0,255)
#define COL32_YELLOW	COL32(255,255,0)
#define COL32_MAGENTA	COL32(255,0,255)
#define COL32_GRAY		COL32(255/2,255/2,255/2)
#define COL32_BLACK		COL32(0,0,0)
#define COL32_ALPHAZERO COL32A(0xff,0xff,0xff,0)
#define COL32_ALPHAONE  COL32A(0xff,0xff,0xff,1)

typedef unsigned int Color32;

struct Color
{
	float r;
	float g;
	float b;
	float a;
};

struct ColorHSV
{
	float h;
	float s;
	float v;
};

static inline Color Col(float r, float g, float b, float a = 1.0f)
{
	Color result = {r, g, b, a};
	return result;
}

static inline Color Col(int r, int g, int b, int a = 255)
{
	return Col((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255);
}

static inline Color Col(Color32 color32)
{
	int a = (color32 >> 24) & 0xff;
	int b = (color32 >> 16) & 0xff;
	int g = (color32 >> 8) & 0xff;
	int r = (color32 >> 0) & 0xff;
	return Col( r, g, b, a );
}

static inline Color32 ToColor32(Color color)
{
	int r = (int)(color.r * 255);
	int g = (int)(color.g * 255);
	int b = (int)(color.b * 255);
	int a = (int)(color.a * 255);
	return COL32A(r,g,b,a);
}

static inline Color32 SetAlpha(Color32 color, float alpha)
{
    int a = (int)(alpha * 255);
	Color32 mask = (1 << 24) - 1;
	return (color & mask) | a << 24;
}

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV
static inline Color32 ColorHSVToColor32(float h, float s, float v)
{
	Color color;
	color.a = 1.0f;
	if (s == 0.0f)
	{
		// gray
		color.r = color.g = color.b = v;
		return ToColor32(color);
	}

	h = fmod(h, 1.0f) / (60.0f / 360.0f);
	int   i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
	case 0: color.r = v; color.g = t; color.b = p; break;
	case 1: color.r = q; color.g = v; color.b = p; break;
	case 2: color.r = p; color.g = v; color.b = t; break;
	case 3: color.r = p; color.g = q; color.b = v; break;
	case 4: color.r = t; color.g = p; color.b = v; break;
	case 5: default: color.r = v; color.g = p; color.b = q; break;
	}

	return ToColor32(color);
}

static inline Color32 ColorHSVToColor32(ColorHSV colorHSV)
{
	return ColorHSVToColor32(colorHSV.h, colorHSV.s, colorHSV.v);
}

// Convert rgb floats ([0-1],[0-1],[0-1]) to hsv floats ([0-1],[0-1],[0-1]), from Foley & van Dam p592
// Optimized http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
static inline ColorHSV ColorToHSV(Color color)
{
	float K = 0.f;
	if (color.g < color.b)
	{
		float tmp = color.g;
		color.g = color.b;
		color.b = tmp;
		K = -1.f;
	}
	if (color.r < color.g)
	{
		float tmp = color.r;
		color.r = color.g;
		color.g = tmp;
		K = -2.f / 6.f - K;
	}

	const float chroma = color.r - (color.g < color.b ? color.g : color.b);
	float h = fabs(K + (color.g - color.b) / (6.f * chroma + 1e-20f));
	float s = chroma / (color.r + 1e-20f);
	float v = color.r;

	return ColorHSV{ h, s, v };
}

static inline ColorHSV Color32ToHSV(Color32 color32)
{
	return ColorToHSV(Col(color32));
}