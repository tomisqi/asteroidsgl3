#pragma once

#include "vector.h"
#include "rect.h"
#include "renderer.h"

void UIInit(Renderer* renderer_p);

void UINewFrame(float deltaT);

bool UIButton(const char* text, Rect rect);

void UILayout(const char* name);

void UITextInput(Rect rect, char* textBuf);

void UICharCallback(unsigned int codepoint);