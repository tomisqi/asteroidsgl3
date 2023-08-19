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

#define MAX_LAYOUTS				    256
#define TEXT_CURSOR_BLINKING_PERIOD 1.2f // The following periodicity: WHITE->TRANSPARENT->WHITE->TRANSPARENT-> etc..
#define MAX_TEXT_INPUTS             10

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

struct LayoutData
{
	bool layoutActive;
	bool layoutDone;

	S16 buttonIdx;
	int buttonsCount;
	S16 highlightedButtonIdx;
	bool highlightedButtonConfirm;
};

struct UiContext
{
	Renderer* renderer_p;
	S64 frameCnt;
	float deltaT;
	double time;

	int textInputActive;
	double tLastInput;
	int textInputsCount;
	TextInput textInputs[MAX_TEXT_INPUTS];

	S64 activeLayoutId;
	LayoutData layouts[MAX_LAYOUTS];
};

static UiContext ui;

void UIInit(Renderer* renderer_p)
{
	memset(&ui, 0, sizeof(ui));
	ui.renderer_p = renderer_p;
	ui.activeLayoutId = S64_MAX;
	ui.time = 0;
	ui.textInputActive = -1;

	for (int i = 0; i < MAX_LAYOUTS; i++)
	{
		ui.layouts[i].highlightedButtonIdx = -1;
	}
}
//#include "renderer.h"
//extern Renderer* rendererGl_p;
void UINewFrame(float deltaT)
{
	ui.frameCnt++;
	ui.deltaT = deltaT;
	ui.time += deltaT;

	Mouse mouse = GameInput_GetMouse();
	for (int i = 0; i < MAX_LAYOUTS; i++)
	{
		if (ui.layouts[i].layoutActive)
		{
			ui.layouts[i].layoutDone = true;
			ui.layouts[i].buttonIdx = 0;
			ui.layouts[i].highlightedButtonConfirm = false;

			S16 prevHighlight = ui.layouts[i].highlightedButtonIdx;
			ui.layouts[i].highlightedButtonIdx = -1;
			if (!mouse.mouseMoved) ui.layouts[i].highlightedButtonIdx = prevHighlight; // If mouse hasn't moved, keep the prev highlight.
		}
	}

	if (mouse.state == MOUSE_PRESSED)
	{
		ui.textInputActive = -1;
		for (int i = 0; i < ui.textInputsCount; i++)
		{
			Rect rect = ui.textInputs[i].rect;
			if (RectContains(rect, mouse.pos)) 
			{ 
				ui.textInputActive = i; 
			break; }
		}
	}
	ui.textInputsCount = 0;
	

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

static void UIMoveInLayout(LayoutData* layout_p, UIDirectionE direction)
{
	if (direction == UI_DOWN) layout_p->highlightedButtonIdx += 1;
	if (direction == UI_UP)   layout_p->highlightedButtonIdx -= 1;

	if (layout_p->highlightedButtonIdx >= layout_p->buttonsCount) layout_p->highlightedButtonIdx = 0;
	if (layout_p->highlightedButtonIdx < 0)                       layout_p->highlightedButtonIdx = layout_p->buttonsCount - 1;
}

void UILayout(const char* name)
{
	S64 id = HashString((const unsigned char*)name); // @todo: Check for collisions.
	ui.activeLayoutId = id;

	LayoutData* layout_p = &ui.layouts[id % MAX_LAYOUTS];
	layout_p->layoutActive = true;

	if (!layout_p->layoutDone) return;

	Mouse mouse = GameInput_GetMouse();
	if (mouse.state == MOUSE_PRESSED || GameInput_ButtonDown(BUTTON_ENTER))
	{
		layout_p->highlightedButtonConfirm = true;
	}

	if (GameInput_ButtonDown(BUTTON_DOWN_ARROW))  UIMoveInLayout(layout_p, UI_DOWN);
	if (GameInput_ButtonDown(BUTTON_UP_ARROW))    UIMoveInLayout(layout_p, UI_UP);
}

bool UIButton(const char* text, Rect rect)
{
	assert(ui.activeLayoutId != S64_MAX);

    LayoutData* layout_p = &ui.layouts[ui.activeLayoutId % MAX_LAYOUTS]; 
	if (!layout_p->layoutDone)
	{
		// We're still discovering the layout. We need a full frame to do that.
		layout_p->buttonsCount++;
		return false;
	}
	
	S16 buttonIdx = layout_p->buttonIdx++;

	Mouse mouse = GameInput_GetMouse();
	if (RectContains(rect, mouse.pos))
	{
		layout_p->highlightedButtonIdx = buttonIdx;
	}

	Vector2 rectCenterPos = GetRectCenter(rect);
	if (layout_p->highlightedButtonIdx == buttonIdx)
	{
		PushUiRect(ui.renderer_p, rect, COLOR_BLUE);
		PushText(ui.renderer_p, text, V2(rectCenterPos.x - rect.size.x / 4 + 20.0f, -rectCenterPos.y), COLOR_WHITE);
		if (layout_p->highlightedButtonConfirm) return true;
	}
	else
	{
		PushUiRect(ui.renderer_p, rect, Col(0.804f, 0.667f, 1.0f));
		PushText(ui.renderer_p, text, V2(rectCenterPos.x - rect.size.x / 4 + 20.0f, -rectCenterPos.y), COLOR_BLACK);
	}

	// If this is the last button, reset the activeLayoutId to make sure other buttons are part of another layout.
	if (layout_p->buttonIdx == layout_p->buttonsCount) ui.activeLayoutId = S64_MAX;

	return false;
}

static void InsertChar(char* str, int strLen, int index, char c)
{
	int charCnt = strLen - index;
	char* new_p = &str[strLen];
	char* old_p = &str[strLen-1];
	for (int i = 0; i < charCnt; i++)
	{
		*new_p = *old_p;
		new_p--;
		old_p--;
	}
	str[index] = c;
}


static void RemoveChar(char* str, int strLen, int index)
{
	int charCnt = strLen - index;
	char* new_p = &str[index];
	char* old_p = &str[index + 1];
	for (int i = 0; i < charCnt; i++)
	{
		*new_p = *old_p;
		new_p++;
		old_p++;
	}
	if (strLen) str[strLen-1] = '\0';
}

static void RemoveChars(char* str, int strLen, int startIdx, int endIdx)
{
	int charCnt = strLen - endIdx;
	char* new_p = &str[startIdx];
	char* old_p = &str[endIdx];
	for (int i = 0; i < charCnt; i++)
	{
		*new_p = *old_p;
		new_p++;
		old_p++;
	}
	while (*new_p) {*new_p = '\0'; new_p++;}
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
	PushText(ui.renderer_p, textBuf, V2(rect.pos.x + 6, -(rect.pos.y + 6)), COLOR_WHITE);
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

	Mouse mouse = GameInput_GetMouse();
	if (mouse.state == MOUSE_PRESSED)
	{
		int cursorIdx = FindClosestCharIdx(textBuf, textLen, rect.pos.x + 6, mouse.pos.x);
		textInputText_p->cursorIdx = cursorIdx;
		textInputText_p->selectionEndIdx = cursorIdx;
	}	
	if (mouse.state == MOUSE_PRESSED_HOLD)
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
