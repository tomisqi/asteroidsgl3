#include "input.h"

static GameInput gameInput;

bool GameInput_ButtonDown(ButtonVal buttonVal)
{
	return
		gameInput.buttons[buttonVal].prevState == RELEASED &&
		gameInput.buttons[buttonVal].state == PRESSED;
}

bool GameInput_Button(ButtonVal buttonVal)
{
	return gameInput.buttons[buttonVal].state == PRESSED;
}

void GameInput_Init()
{
	for (int i = 0; i < MAX_BUTTONS; i++)
	{
		gameInput.buttons[i].state = RELEASED;
	}
}

void GameInput_NewFrame(ButtonState newButtonStates[])
{
	for (int i = 0; i < MAX_BUTTONS; i++)
	{
		gameInput.buttons[i].prevState = gameInput.buttons[i].state;
		gameInput.buttons[i].state = newButtonStates[i];
	}
}

// 		gameInput.buttons[i].state = (ButtonState)glfwGetKey(window, input.buttons[i].keyVal);

void GameInput_BindButton(ButtonVal buttonVal, int platformVal)
{
	gameInput.buttonBindings[buttonVal] = platformVal;
}

int GameInput_GetBinding(int buttonIdx)
{
	return gameInput.buttonBindings[buttonIdx];
}