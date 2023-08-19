#pragma once
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "common.h"
#include "vector.h"

enum ButtonState
{
	RELEASED = 0,
	PRESSED,
};

enum MouseStateE : U8
{
	MOUSE_RELEASED,
	MOUSE_PRESSED,
	MOUSE_PRESSED_HOLD,
	MOUSE_DOUBLECLICK,
};

struct Mouse
{
	MouseStateE state;
	Vector2 pos;
	double tLastPress;
	bool mouseMoved;
};

enum ButtonVal
{
	BUTTON_X = 0,
	BUTTON_Q,
	BUTTON_C,
	BUTTON_S,
	BUTTON_A,
	BUTTON_D,
	BUTTON_W,
	BUTTON_UP_ARROW,
	BUTTON_LEFT_ARROW,
	BUTTON_RIGHT_ARROW,
	BUTTON_DOWN_ARROW,
	BUTTON_LSHIFT,
	BUTTON_ENTER,
	BUTTON_ESC,
	BUTTON_BACKSPACE,
	BUTTON_HOME,
	BUTTON_END,
	BUTTON_DEL,
	BUTTON_LCTRL,
	MAX_BUTTONS,
};

struct GameButton
{
	ButtonState prevState;
	ButtonState state;
};

struct GameInput
{
	double time;
	Mouse mouse;
	GameButton buttons[MAX_BUTTONS];
	int buttonBindings[MAX_BUTTONS];
};


bool GameInput_ButtonDown(ButtonVal buttonVal);
bool GameInput_Button(ButtonVal buttonVal);
void GameInput_Init();
void GameInput_NewFrame(ButtonState newButtonStates[], bool mouseIsPressed, Vector2 mousePosScreen, Vector2 screenDim, float deltaT);
void GameInput_BindButton(ButtonVal buttonVal, int platformVal);
int GameInput_GetBinding(int buttonIdx);
Mouse GameInput_GetMouse();