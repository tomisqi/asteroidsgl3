#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "vector.h"
#include "rect.h"
#include "renderer.h"
#include "common.h"
#include "ui.h"
#include "stdio.h"
#include "hash.h"
#include "input.h"
#include "utils.h"

#define TEXT_CURSOR_BLINKING_PERIOD 1.2f // The following periodicity: WHITE->TRANSPARENT->WHITE->TRANSPARENT-> etc..
#define MAX_TEXT_INPUTS             10
#define MAX_LAYOUTS                 16


enum UIDirectionE : U8
{
	UI_DOWN,
	UI_LEFT,
	UI_UP,
	UI_RIGHT,
};

struct TextInput
{
	int cursorIdx;
	int selectionEndIdx;
	char* textBuf_p;
	Rect rect;
};

struct Layout
{
	UILayoutE uiLayout;
	bool keyboardNavigate;

	Vector2 nextUiPos;
};

struct UiContext
{
	Renderer* renderer_p;
	S64 frameCnt;
	float deltaT;
	double time;
	Vector2 screenDim;

	int textInputActive;
	double tLastInput;
	int textInputsCount;
	TextInput textInputs[MAX_TEXT_INPUTS];

	int buttonActive;
	int buttonIdx;
	int buttonsCount;

	int activeLayoutIdx;
	Layout layouts[MAX_LAYOUTS];
};

static UiContext ui;

void UIInit(Renderer* renderer_p)
{
	memset(&ui, 0, sizeof(ui));
	ui.renderer_p = renderer_p;
	ui.time = 0;
	ui.textInputActive = -1;
	ui.buttonActive = -1;

	Layout layoutDefault = {0};
	layoutDefault.uiLayout = UI_VERTICAL;
	layoutDefault.keyboardNavigate = false;
	layoutDefault.nextUiPos = V2(0.5f, 0.5f);

	ui.layouts[0] = layoutDefault; // If no layout is passed, default is used.
}

static Mouse GetUiMouse()
{
	Mouse mouse = GameInput_GetMouse();
	mouse.pos = V2(mouse.pos.x/ui.screenDim.x, mouse.pos.y/ui.screenDim.y); // Fix position to be 01
	return mouse;
}

//#include "renderer.h"
//extern Renderer* rendererGl_p;
void UINewFrame(float deltaT, Vector2 screenDim)
{
	ui.frameCnt++;
	ui.deltaT = deltaT;
	ui.time += deltaT;
	ui.screenDim = screenDim;

	Mouse mouse = GetUiMouse();

	if (mouse.leftButton == MOUSE_PRESSED)
	{
		ui.textInputActive = -1;
		for (int i = 0; i < ui.textInputsCount; i++)
		{
			Rect rect = ui.textInputs[i].rect;
			if (RectContains(rect, mouse.pos)) 
			{ 
				ui.textInputActive = i; 
				break; 
			}
		}
	}
	ui.textInputsCount = 0;
	
	if (ui.buttonIdx != ui.buttonsCount) ui.buttonsCount = ui.buttonIdx;
	ui.buttonIdx = 0;
	if (mouse.mouseMoved) ui.buttonActive = -1;
	if (GameInput_ButtonDown(BUTTON_DOWN_ARROW) && ui.buttonsCount)
	{
		ui.buttonActive = (ui.buttonActive + 1) % ui.buttonsCount;
	}
	if (GameInput_ButtonDown(BUTTON_UP_ARROW) && ui.buttonsCount)
	{
		ui.buttonActive = ui.buttonActive > 0 ? (ui.buttonActive - 1) : ui.buttonsCount - 1;
	}

	ui.activeLayoutIdx = 0;

#if 0
	char buf[32] = { 0 };
	sprintf(buf, "(%.2f, %.2f)", mouse.pos.x, mouse.pos.y);
	PushText(rendererGl_p, buf, V2(-50, -200), COLOR_WHITE);

	char mousepos[32] = { 0 };
	sprintf(mousepos, "(%.2f, %.2f)", ui.mouse.pos.x, ui.mouse.pos.y);
	PushText(ui.renderer_p, mousepos, VECTOR2_ZERO);

	char mousestate[32] = { 0 };
	if (mouse.state == MOUSE_RELEASED) sprintf(mousestate, "%s", "RELEASED");
	if (mouse.state == MOUSE_PRESSED_HOLD) sprintf(mousestate, "%s", "HOLD");
	if (mouse.state == MOUSE_PRESSED) sprintf(mousestate, "%s", "PRESSED");
	if (mouse.state == MOUSE_DOUBLECLICK) sprintf(mousestate, "%s", "\n\nDOUBLECLICK\n\n");
	printf("%s ", mousestate);
#endif
}

static Vector2 GetTextPos(const char* text, Rect rect, UITextAlignmentE textAlignment)
{
	Vector2 rectCenter = GetRectCenter(rect);
	float textWidth = ScreenSizeToSize01(V2(GetTextWidth(ui.renderer_p, text), 0)).x;
	float textXPos = 0;
	switch (textAlignment)
	{
	case TEXT_ALIGN_CENTER:
		textXPos = rectCenter.x - (textWidth / 2.0f);
		break;
	case TEXT_ALIGN_LEFT:
		textXPos = rect.pos.x;
		break;
	case TEXT_ALIGN_RIGHT:
		textXPos = rect.pos.x + rect.size.x - textWidth;
		break;
	default:
		assert(false);
		break;
	}

	return V2(textXPos, rectCenter.y - 0.005f);
}

void UILabel(const char* text, Rect rect, UITextAlignmentE textAlignment, Color color)
{
	Vector2 textPos = GetTextPos(text, rect, textAlignment);
	PushText01(ui.renderer_p, text, textPos, color);
}

void UILabel(const char* text, Vector2 pos, UITextAlignmentE textAlignment, Color color)
{
	UILabel(text, NewRect(pos, VECTOR2_ZERO), textAlignment, color);
}

void UIRect(Rect rect, Color color)
{
	PushUiRect01(ui.renderer_p, rect, color);
}

static bool UIButton(const char* text, Rect rect, UITextAlignmentE textAlignment, Layout* layout_p)
{
	Mouse mouse = GetUiMouse();
	int thisButton = ui.buttonIdx++;

	if (layout_p->uiLayout == UI_VERTICAL)   layout_p->nextUiPos = rect.pos + rect.size.y * VECTOR2_DOWN + 0.005f * VECTOR2_DOWN;
	if (layout_p->uiLayout == UI_HORIZONTAL) layout_p->nextUiPos = rect.pos + rect.size.x * VECTOR2_RIGHT + 0.005f * VECTOR2_RIGHT;

	if (RectContains(rect, mouse.pos) || (layout_p->keyboardNavigate && (ui.buttonActive == thisButton)))
	{
		UIRect(rect, COLOR_BLUE);
		UILabel(text, rect, textAlignment, COLOR_WHITE);
		if ((mouse.leftButton == MOUSE_PRESSED) || (mouse.leftButton == MOUSE_DOUBLECLICK) || GameInput_ButtonDown(BUTTON_ENTER))
		{
			return true;
		}
	}
	else
	{
		UIRect(rect, Col(0.804f, 0.667f, 1.0f));
		UILabel(text, rect, textAlignment, COLOR_BLACK);
	}

	return false;
}

bool UIButton(const char* text, Rect rect, UITextAlignmentE textAlignment)
{
	Layout* layout_p = &ui.layouts[ui.activeLayoutIdx];
	return UIButton(text, rect, textAlignment, layout_p);
}

bool UIButton(const char* text, Vector2 size, UITextAlignmentE textAlignment)
{
	Layout* layout_p = &ui.layouts[ui.activeLayoutIdx];
	Vector2 pos = layout_p->nextUiPos;
	Rect rect = NewRect(pos, size);
	return UIButton(text, rect, textAlignment, layout_p);
}

void UILayout_(int id, bool keyboardNavigate, UILayoutE uiLayout, Vector2 pos)
{
	printf("%d\n", id);
	assert(id < MAX_LAYOUTS);
	ui.activeLayoutIdx = id;
	Layout* layout_p = &ui.layouts[ui.activeLayoutIdx];
	layout_p->keyboardNavigate = keyboardNavigate;
	layout_p->uiLayout = uiLayout;
	layout_p->nextUiPos = pos;
}

static int FindClosestCharIdx(char* textBuf, int textLen, float startPosX, float xpos)
{
	float closestDist = F32_MAX;
	int charIdx = 0;
	for (int i = 0; i <= textLen; i++)
	{
		float xposChar = GetCharPosX(ui.renderer_p->textRendering.charUvData, startPosX, textBuf, i);
		float dist = fabs(xposChar - xpos);
		if (dist < closestDist) { closestDist = dist; charIdx = i; }
		else break;
	}
	return charIdx;
}

void UICharCallback(unsigned int codepoint)
{
	char c = (char)codepoint;

	if (ui.textInputActive >= 0)
	{
		TextInput* textInputText_p = &ui.textInputs[ui.textInputActive];
		int textBufLen = strlen(textInputText_p->textBuf_p);

		InsertChar(textInputText_p->textBuf_p, textBufLen, textInputText_p->cursorIdx, c);
		textInputText_p->cursorIdx++;
		textInputText_p->selectionEndIdx = textInputText_p->cursorIdx;
	}
}

void UITextInput(Rect rect, char* textBuf)
{
	// Text and textbox graphics
	PushText(ui.renderer_p, textBuf, V2(rect.pos.x + 6, -(rect.pos.y + 6)), COLOR_WHITE, rect.pos.x + rect.size.x - 10);
	PushUiRect(ui.renderer_p, ContractRect(rect, 5), Col(0.3f, 0.3f, 0.3f));

	int thisone = ui.textInputsCount++;
	TextInput* textInputText_p = &ui.textInputs[thisone];
	textInputText_p->textBuf_p = textBuf;
	textInputText_p->rect = rect;

	if (ui.textInputActive != thisone) return;

	int prevCursorIdx = textInputText_p->cursorIdx;
	int textLen = strlen(textBuf);

	// Backspace
	bool holdingBackspace = GameInput_Button(BUTTON_BACKSPACE) && ELAPSED(ui.time, ui.tLastInput) >= 0.5f;
	if ((GameInput_ButtonDown(BUTTON_BACKSPACE) || holdingBackspace) && (textLen > 0) && (textInputText_p->cursorIdx > 0))
	{
		if (textInputText_p->cursorIdx == textInputText_p->selectionEndIdx)
		{
			RemoveChar(textBuf, textLen, textInputText_p->cursorIdx - 1);
			textInputText_p->cursorIdx--;
		}
		else
		{
			int startIdx = __min(textInputText_p->cursorIdx, textInputText_p->selectionEndIdx);
			int endIdx = __max(textInputText_p->cursorIdx, textInputText_p->selectionEndIdx);
			RemoveChars(textBuf, textLen, startIdx, endIdx);
			textInputText_p->cursorIdx = startIdx;
		}
		textInputText_p->selectionEndIdx = textInputText_p->cursorIdx;
	}

	// Del
	bool holdingDel = GameInput_Button(BUTTON_DEL) && ELAPSED(ui.time, ui.tLastInput) >= 0.5f;
	if ((GameInput_ButtonDown(BUTTON_DEL) || holdingDel) && (textLen > 0) && (textInputText_p->cursorIdx <= textLen))
	{
		if (textInputText_p->cursorIdx == textInputText_p->selectionEndIdx)
		{
			RemoveChar(textBuf, textLen, textInputText_p->cursorIdx);
		}
		else
		{
			int startIdx = __min(textInputText_p->cursorIdx, textInputText_p->selectionEndIdx);
			int endIdx = __max(textInputText_p->cursorIdx, textInputText_p->selectionEndIdx);
			RemoveChars(textBuf, textLen, startIdx, endIdx);			
			textInputText_p->cursorIdx = startIdx;
			textInputText_p->selectionEndIdx = startIdx;
		}
		// Don't move cursor here.
	}

	// Left arrow
	bool holdingLeftArrow = GameInput_Button(BUTTON_LEFT_ARROW) && ELAPSED(ui.time, ui.tLastInput) >= 0.5f;
	if (GameInput_ButtonDown(BUTTON_LEFT_ARROW) || holdingLeftArrow)
	{
		int cursorIdx = textInputText_p->cursorIdx - 1;
		if (GameInput_Button(BUTTON_LCTRL))
		{
			while (cursorIdx > 0 && isalnum(textBuf[cursorIdx-1])) { cursorIdx--; }
		}
		textInputText_p->cursorIdx = cursorIdx;
	}

	// Right arrow
	bool holdingRightArrow = GameInput_Button(BUTTON_RIGHT_ARROW) && ELAPSED(ui.time, ui.tLastInput) >= 0.5f; 
	if (GameInput_ButtonDown(BUTTON_RIGHT_ARROW) || holdingRightArrow)
	{
		int cursorIdx = textInputText_p->cursorIdx + 1;
		if (GameInput_Button(BUTTON_LCTRL))
		{
			while (cursorIdx < textLen && isalnum(textBuf[cursorIdx])) { cursorIdx++; }
		}
		textInputText_p->cursorIdx = cursorIdx;
	}

	// Home
	if (GameInput_ButtonDown(BUTTON_HOME))
	{
		textInputText_p->cursorIdx = 0;
	}

	// End
	if (GameInput_ButtonDown(BUTTON_END))
	{
		textInputText_p->cursorIdx = textLen;
	}

	Mouse mouse = GetUiMouse();
	if (mouse.leftButton == MOUSE_PRESSED)
	{
		int cursorIdx = FindClosestCharIdx(textBuf, textLen, rect.pos.x + 6, mouse.pos.x);
		textInputText_p->cursorIdx = cursorIdx;
		textInputText_p->selectionEndIdx = cursorIdx;
	}	
	if (mouse.leftButton == MOUSE_PRESSED_HOLD)
	{
		int cursorIdx = FindClosestCharIdx(textBuf, textLen, rect.pos.x + 6, mouse.pos.x);
		textInputText_p->cursorIdx = cursorIdx;
	}

#if 0
	if (ui.mouse.state == MOUSE_DOUBLECLICK)
	{
		int cursorIdx = FindClosestCharIdx(textBuf, textLen, rect.pos.x + 6, ui.mouse.pos.x);
		int selectionEndIdx = cursorIdx;
		while (cursorIdx > 0 && isalnum(textBuf[cursorIdx - 1])) { cursorIdx--; }
		while (selectionEndIdx < textLen && isalnum(textBuf[selectionEndIdx])) { selectionEndIdx++; }
		textInputText_p->cursorIdx = cursorIdx;
		textInputText_p->selectionEndIdx = selectionEndIdx;
	}
#endif
	textInputText_p->cursorIdx = Clamp(textInputText_p->cursorIdx, 0, textLen);

	// Holding shift
	if (!GameInput_Button(BUTTON_LSHIFT) && 
		(GameInput_ButtonDown(BUTTON_LEFT_ARROW) || GameInput_ButtonDown(BUTTON_RIGHT_ARROW) || GameInput_ButtonDown(BUTTON_HOME) || GameInput_ButtonDown(BUTTON_END) || holdingLeftArrow || holdingRightArrow))
	{
		textInputText_p->selectionEndIdx = textInputText_p->cursorIdx;
	}

	// Last time pressed
	if (GameInput_ButtonDown(BUTTON_BACKSPACE))	  ui.tLastInput = ui.time;
	if (GameInput_ButtonDown(BUTTON_DEL))         ui.tLastInput = ui.time;
	if (GameInput_ButtonDown(BUTTON_LEFT_ARROW))  ui.tLastInput = ui.time;
	if (GameInput_ButtonDown(BUTTON_RIGHT_ARROW)) ui.tLastInput = ui.time;

	// TextBox and outline
	PushUiRect(ui.renderer_p, rect, COLOR_CYAN);
	PushUiRect(ui.renderer_p, ContractRect(rect, 5), Col(0.3f, 0.3f, 0.3f));

	// Text cursor graphics
	int cursorPeriodInFrames = TEXT_CURSOR_BLINKING_PERIOD / ui.deltaT;
	float xpos = GetCharPosX(ui.renderer_p->textRendering.charUvData, rect.pos.x + 6, textBuf, textInputText_p->cursorIdx);
	bool cursorMoved = prevCursorIdx != textInputText_p->cursorIdx;
	if (ui.frameCnt % cursorPeriodInFrames < (cursorPeriodInFrames / 2) || cursorMoved)
	{
		PushUiRect(ui.renderer_p, NewRect(V2(xpos, rect.pos.y + 6), V2(1, rect.size.y - 12)), COLOR_WHITE);
	}

	// Selection graphics
	if (textInputText_p->cursorIdx != textInputText_p->selectionEndIdx)
	{
		float endxpos = GetCharPosX(ui.renderer_p->textRendering.charUvData, rect.pos.x + 6, textBuf, textInputText_p->selectionEndIdx);;
		float minx = fmin(xpos, endxpos);
		float maxx = fmax(xpos, endxpos);
		float xsize = maxx - minx;
		PushUiRect(ui.renderer_p, NewRect(V2(minx, rect.pos.y + 3), V2(xsize, rect.size.y - 9)), COLOR_BLUE);
	}

}
