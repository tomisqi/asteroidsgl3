#include <string.h>
#include <assert.h>
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
#define TEXT_INPUT_MAX_LENGTH       1 << 10
#define MAX_TEXT_INPUTS             10

#define ELAPSED(t2, t1) (t2 - t1)

enum UIDirectionE : U8
{
	UI_DOWN,
	UI_LEFT,
	UI_UP,
	UI_RIGHT,
};

struct Mouse
{
	MouseStateE state;
	Vector2 pos;
};

struct TextInputText
{
	int cursorIdx;
	int selectionEndIdx;
	int textBufLen;
	char textBuf[TEXT_INPUT_MAX_LENGTH + 1];
};

struct LayoutData
{
	bool layoutActive;
	bool layoutDone;

	S16 buttonIdx;
	int buttonsCount;
	S16 highlightedButtonIdx;
	bool highlightedButtonConfirm;

	int textInputActive;
	double tLastInput;
	TextInputText textInputs[MAX_TEXT_INPUTS];
};

struct UiContext
{
	Mouse mouse;
	Renderer* renderer_p;
	S64 frameCnt;
	float deltaT;
	double time;

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

	for (int i = 0; i < MAX_LAYOUTS; i++)
	{
		ui.layouts[i].highlightedButtonIdx = -1;
		ui.layouts[i].textInputActive = 0;
	}
}

void UINewFrame(Vector2 mousePosScreen, bool mouseIsPressed, Vector2 screenDim, float deltaT)
{
	ui.frameCnt++;
	ui.deltaT = deltaT;
	ui.time += deltaT;

	MouseStateE prevState = ui.mouse.state;
	ui.mouse.state = MOUSE_RELEASED;
	if (mouseIsPressed)
	{ 
		ui.mouse.state = MOUSE_PRESSED_HOLD;
		if (prevState == MOUSE_RELEASED)
		{
			ui.mouse.state = MOUSE_PRESSED;
		}
	}

	// MousePosScreen comes as (0,0) to (screnDim.x, screenDim.y). Transform into worldPos
	Vector2 prevMousePos = ui.mouse.pos;
	ui.mouse.pos = V2(mousePosScreen.x - screenDim.x / 2, screenDim.y / 2 - mousePosScreen.y);
	bool mouseMoved = (prevMousePos != ui.mouse.pos);

	for (int i = 0; i < MAX_LAYOUTS; i++)
	{
		if (ui.layouts[i].layoutActive)
		{
			ui.layouts[i].layoutDone = true;
			ui.layouts[i].buttonIdx = 0;
			ui.layouts[i].highlightedButtonConfirm = false;

			S16 prevHighlight = ui.layouts[i].highlightedButtonIdx;
			ui.layouts[i].highlightedButtonIdx = -1;
			if (!mouseMoved) ui.layouts[i].highlightedButtonIdx = prevHighlight; // If mouse hasn't moved, keep the prev highlight.
		}
	}

#if 0
	char mousepos[32] = { 0 };
	sprintf(mousepos, "(%.2f, %.2f)", ui.mouse.pos.x, ui.mouse.pos.y);
	PushText(ui.renderer_p, mousepos, VECTOR2_ZERO);

	char mousestate[32] = { 0 };
	if (ui.mouse.state == MOUSE_RELEASED) sprintf(mousestate, "%s", "RELEASED");
	if (ui.mouse.state == MOUSE_PRESSED_HOLD) sprintf(mousestate, "%s", "HOLD");
	if (ui.mouse.state == MOUSE_PRESSED) sprintf(mousestate, "%s", "PRESSED");
	PushText(ui.renderer_p, mousestate, VECTOR2_ZERO);
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

	if (ui.mouse.state == MOUSE_PRESSED || GameInput_ButtonDown(BUTTON_ENTER))
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

	if (RectContains(rect, ui.mouse.pos))
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

static void ShiftStringToRight(char* s, int strLen, int index)
{
	int charCnt = strLen - index;
	char* new_p = &s[strLen];
	char* old_p = &s[strLen-1];
	for (int i = 0; i < charCnt; i++)
	{
		*new_p = *old_p;
		new_p--;
		old_p--;
	}
}


static void ShiftStringToLeft(char* s, int strLen, int index)
{
	int charCnt = strLen - index;
	char* new_p = &s[index];
	char* old_p = &s[index + 1];
	for (int i = 0; i < charCnt; i++)
	{
		*new_p = *old_p;
		new_p++;
		old_p++;
	}
}

void UICharCallback(unsigned int codepoint)
{
	char c = (char)codepoint;

	int textInputActive = ui.layouts[0].textInputActive;
	TextInputText* textInputText_p = &ui.layouts[0].textInputs[textInputActive];

	int idx = textInputText_p->textBufLen++;
	if (textInputText_p->cursorIdx != textInputText_p->textBufLen)
	{
		ShiftStringToRight(textInputText_p->textBuf, textInputText_p->textBufLen, textInputText_p->cursorIdx);
		idx = textInputText_p->cursorIdx;
	}

	textInputText_p->textBuf[idx] = c;	
	textInputText_p->cursorIdx++;
}

void UITextInput(Rect rect)
{
	PushUiRect(ui.renderer_p, rect, COLOR_CYAN);
	PushUiRect(ui.renderer_p, ContractRect(rect, 5), Col(0.3f, 0.3f, 0.3f));

	int textInputActive = ui.layouts[0].textInputActive;
	TextInputText* textInputText_p = &ui.layouts[0].textInputs[textInputActive];

	int prevCursorIdx = textInputText_p->cursorIdx;

	bool holdingBackspace = GameInput_Button(BUTTON_BACKSPACE) && ELAPSED(ui.time, ui.layouts[0].tLastInput) >= 0.5f;
	if ((GameInput_ButtonDown(BUTTON_BACKSPACE) || holdingBackspace) && (textInputText_p->textBufLen > 0) && (textInputText_p->cursorIdx > 0))
	{
		int idx = --textInputText_p->textBufLen;
		if (textInputText_p->cursorIdx != textInputText_p->textBufLen)
		{
			ShiftStringToLeft(textInputText_p->textBuf, textInputText_p->textBufLen, textInputText_p->cursorIdx-1);
		}
		textInputText_p->textBuf[idx] = '\0';
		textInputText_p->cursorIdx--;
	}	

	bool holdingLeftArrow  = GameInput_Button(BUTTON_LEFT_ARROW) && ELAPSED(ui.time, ui.layouts[0].tLastInput) >= 0.5f;
	bool holdingRightArrow = GameInput_Button(BUTTON_RIGHT_ARROW) && ELAPSED(ui.time, ui.layouts[0].tLastInput) >= 0.5f;
	if (GameInput_ButtonDown(BUTTON_HOME))                             textInputText_p->cursorIdx = 0;
	if (GameInput_ButtonDown(BUTTON_END))                              textInputText_p->cursorIdx = textInputText_p->textBufLen;
	if (GameInput_ButtonDown(BUTTON_LEFT_ARROW)  || holdingLeftArrow)  textInputText_p->cursorIdx--;
	if (GameInput_ButtonDown(BUTTON_RIGHT_ARROW) || holdingRightArrow) textInputText_p->cursorIdx++;
	textInputText_p->cursorIdx = Clamp(textInputText_p->cursorIdx, 0, textInputText_p->textBufLen);

	int cursorPeriodInFrames = TEXT_CURSOR_BLINKING_PERIOD / ui.deltaT;
	TextCursor textCursor = { textInputText_p->cursorIdx, rect.pos.x + 6};
	PushText(ui.renderer_p, textInputText_p->textBuf, V2(rect.pos.x + 6, -(rect.pos.y + 6)), COLOR_WHITE, &textCursor);
	bool cursorMoved = prevCursorIdx != textInputText_p->cursorIdx;
	if (ui.frameCnt % cursorPeriodInFrames < (cursorPeriodInFrames / 2) || cursorMoved) 
	{
		PushUiRect(ui.renderer_p, NewRect(V2(textCursor.xpos, rect.pos.y + 6), V2(1, rect.size.y - 12)), COLOR_WHITE); // Show cursor
	}

	if (GameInput_ButtonDown(BUTTON_BACKSPACE))	  ui.layouts[0].tLastInput = ui.time;
	if (GameInput_ButtonDown(BUTTON_LEFT_ARROW))  ui.layouts[0].tLastInput = ui.time;
	if (GameInput_ButtonDown(BUTTON_RIGHT_ARROW)) ui.layouts[0].tLastInput = ui.time;
}
