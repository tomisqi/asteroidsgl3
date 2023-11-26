#pragma once

#include "vector.h"
#include "rect.h"
#include "renderer.h"
#include "vector.h"

enum UITextAlignmentE : U8
{
	TEXT_ALIGN_CENTER,
	TEXT_ALIGN_LEFT,
	TEXT_ALIGN_RIGHT,
};

enum UILayoutE : U8
{
	UI_NONE,
	UI_VERTICAL,
	UI_HORIZONTAL,
};

void UIInit(Renderer* renderer_p);

void UINewFrame(float deltaT, Vector2 screenDim);

bool UIButton(const char* text, Rect rect, UITextAlignmentE textAlignment = TEXT_ALIGN_CENTER);

bool UIButton(const char* text, Vector2 size, UITextAlignmentE textAlignment = TEXT_ALIGN_CENTER);

void UITextInput(Rect rect, char* textBuf);

void UILabel(const char* text, Rect rect, UITextAlignmentE textAlignment = TEXT_ALIGN_CENTER, Color color = COLOR_WHITE);

void UILabel(const char* text, Vector2 pos, UITextAlignmentE textAlignment = TEXT_ALIGN_CENTER, Color color = COLOR_WHITE);

void UIRect(Rect rect, Color color);

void UICharCallback(unsigned int codepoint);

void UILayout_(int id, bool keyboardNavigate, UILayoutE uiLayout, Vector2 pos);

#define UILayout(keyboardNavigate) UILayout_(__COUNTER__ + 1, keyboardNavigate, UI_NONE, VECTOR2_ZERO)

#define UILayoutVertical(pos) UILayout_(__COUNTER__ + 1, false, UI_VERTICAL, pos)