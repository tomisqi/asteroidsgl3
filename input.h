#pragma once
#include <GLFW/glfw3.h>
#include <stdio.h>

enum ButtonState
{
	RELEASED = 0,
	PRESSED,
};

enum ButtonVal
{
	BUTTON_X = 0,
	BUTTON_Q,
	BUTTON_C,
	BUTTON_S,
	BUTTON_A,
	BUTTON_D,
	BUTTON_UP_ARROW,
	BUTTON_LEFT_ARROW,
	BUTTON_RIGHT_ARROW,
	BUTTON_DOWN_ARROW,
	BUTTON_LSHIFT,
	BUTTON_ENTER,
	BUTTON_ESC,
	MAX_BUTTONS,
};

struct GameButton
{
	ButtonState prevState;
	ButtonState state;
};

struct GameInput
{
	GameButton buttons[MAX_BUTTONS];
	int buttonBindings[MAX_BUTTONS];
};


bool GameInput_ButtonDown(ButtonVal buttonVal);
bool GameInput_Button(ButtonVal buttonVal);
void GameInput_Init();
void GameInput_NewFrame(ButtonState newButtonStates[]);
void GameInput_BindButton(ButtonVal buttonVal, int platformVal);
int GameInput_GetBinding(int buttonIdx);