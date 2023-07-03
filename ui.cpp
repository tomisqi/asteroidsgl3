#include <string.h>
#include "vector.h"
#include "rect.h"
#include "renderer.h"
#include "common.h"
#include "ui.h"
#include "stdio.h"

struct Mouse
{
	MouseStateE state;
	Vector2 pos;
};

struct UiContext
{
	Mouse mouse;
	Renderer* renderer_p;
};

static UiContext ui;

void UI_Init(Renderer* renderer_p)
{
	memset(&ui, 0, sizeof(ui));
	ui.renderer_p = renderer_p;
}

void UI_NewFrame(Vector2 mousePosScreen, bool mouseIsPressed, Vector2 screenDim)
{
	// MousePosScreen comes as (0,0) to (screnDim.x, screenDim.y). Transform into worldPos
	ui.mouse.pos = V2(mousePosScreen.x - screenDim.x / 2, screenDim.x / 2 - mousePosScreen.y );

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

bool Button(const char* text, Rect rect)
{
	Vector2 centerPos = V2(rect.pos.x + rect.size.x / 2, rect.pos.y + rect.size.y / 2);	
	if (RectContains(rect, ui.mouse.pos))
	{
		PushSprite(ui.renderer_p, centerPos, rect.size, VECTOR2_UP, 3, V3(0,0,1)); // Blue
		PushText(ui.renderer_p, text, V2(centerPos.x - rect.size.x / 4 + 20.0f, -centerPos.y), VECTOR3_ONE);
		if (ui.mouse.state == MOUSE_PRESSED) return true;
	}
	else
	{
		PushSprite(ui.renderer_p, centerPos, rect.size, VECTOR2_UP, 3);
		PushText(ui.renderer_p, text, V2(centerPos.x - rect.size.x / 4 + 20.0f, -centerPos.y), VECTOR3_ZERO);
	}
	return false;
}