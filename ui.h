#pragma once

#include "vector.h"
#include "rect.h"
#include "renderer.h"

enum MouseStateE : U8
{
	MOUSE_RELEASED,
	MOUSE_PRESSED,
	MOUSE_PRESSED_HOLD,
};

void UI_Init(Renderer* renderer_p);

void UI_NewFrame(Vector2 mousePosScreen, bool mouseIsPressed, Vector2 screenDim);

bool Button(const char* text, Rect rect);