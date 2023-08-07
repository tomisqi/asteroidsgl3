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

void UIInit(Renderer* renderer_p);

void UINewFrame(Vector2 mousePosScreen, bool mouseIsPressed, Vector2 screenDim, float deltaT);

bool UIButton(const char* text, Rect rect);

void UILayout(const char* name);

void UITextInput(Rect rect);

void UICharCallback(unsigned int codepoint);