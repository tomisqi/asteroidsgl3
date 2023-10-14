#pragma once

#include "vector.h"
#include "rect.h"
#include "renderer.h"

enum UITextAlignmentE : U8
{
	TEXT_ALIGN_CENTER,
	TEXT_ALIGN_LEFT,
	TEXT_ALIGN_RIGHT,
};

void UIInit(Renderer* renderer_p);

void UINewFrame(float deltaT, Vector2 screenDim);

bool UIButton(const char* text, Rect rect, UITextAlignmentE textAlignment = TEXT_ALIGN_CENTER);

void UITextInput(Rect rect, char* textBuf);

void UILabel(const char* text, Rect rect, UITextAlignmentE textAlignment = TEXT_ALIGN_CENTER, Color color = COLOR_WHITE);
void UILabel(const char* text, Vector2 pos, UITextAlignmentE textAlignment = TEXT_ALIGN_CENTER, Color color = COLOR_WHITE);

void UICharCallback(unsigned int codepoint);