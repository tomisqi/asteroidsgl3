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

#define MAX_LAYOUTS 256

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
	Mouse mouse;
	Renderer* renderer_p;

	S64 activeLayoutId;
	LayoutData layouts[MAX_LAYOUTS];
};

static UiContext ui;

void UIInit(Renderer* renderer_p)
{
	memset(&ui, 0, sizeof(ui));
	ui.renderer_p = renderer_p;
	ui.activeLayoutId = S64_MAX;

	for (int i = 0; i < MAX_LAYOUTS; i++)
	{
		ui.layouts[i].highlightedButtonIdx = -1;
	}
}

void UINewFrame(Vector2 mousePosScreen, bool mouseIsPressed, Vector2 screenDim)
{
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
		PushUiRect(ui.renderer_p, rect, 3, V3(0, 0, 1)); // Blue
		PushText(ui.renderer_p, text, V2(rectCenterPos.x - rect.size.x / 4 + 20.0f, -rectCenterPos.y), VECTOR3_ONE);
		if (layout_p->highlightedButtonConfirm) return true;
	}
	else
	{
		PushUiRect(ui.renderer_p, rect, 3);
		PushText(ui.renderer_p, text, V2(rectCenterPos.x - rect.size.x / 4 + 20.0f, -rectCenterPos.y), VECTOR3_ZERO);
	}

	// If this is the last button, reset the activeLayoutId to make sure other buttons are part of another layout.
	if (layout_p->buttonIdx == layout_p->buttonsCount) ui.activeLayoutId = S64_MAX;

	return false;
}