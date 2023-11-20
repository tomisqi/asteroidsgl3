#include "input.h"
#include "common.h"
#include "string.h"

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
	memset(&gameInput, 0, sizeof(gameInput));
	gameInput.time = 0;
	for (int i = 0; i < MAX_BUTTONS; i++)
	{
		gameInput.buttons[i].state = RELEASED;
	}
}

void GameInput_NewFrame(ButtonState newButtonStates[], bool mouseLeftPressed, bool mouseRightPressed, Vector2 mousePosScreen, Vector2 screenDim, float deltaT)
{
	gameInput.time += deltaT;

	// Mouse Left Button
	MouseStateE prev = gameInput.mouse.leftButton;
	gameInput.mouse.leftButton = MOUSE_RELEASED;
	if (mouseLeftPressed)
	{
		gameInput.mouse.leftButton = MOUSE_PRESSED_HOLD;
		if (prev == MOUSE_RELEASED)
		{
			gameInput.mouse.leftButton = MOUSE_PRESSED;
			if (ELAPSED(gameInput.time, gameInput.mouse.tLastLeftPress) < 0.2f) gameInput.mouse.leftButton = MOUSE_DOUBLECLICK;
			gameInput.mouse.tLastLeftPress = gameInput.time;
		}
	}

	// Mouse Right Button
	prev = gameInput.mouse.rightButton;
	gameInput.mouse.rightButton = MOUSE_RELEASED;
	if (mouseRightPressed)
	{
		gameInput.mouse.rightButton = MOUSE_PRESSED_HOLD;
		if (prev == MOUSE_RELEASED)
		{
			gameInput.mouse.rightButton = MOUSE_PRESSED;
		}
	}

	// Mouse move
	Vector2 prevMousePos = gameInput.mouse.pos;
	gameInput.mouse.pos = V2(mousePosScreen.x, screenDim.y - mousePosScreen.y); // MousePosScreen.x is Ok, but we flip MousePosScreen.y.
	gameInput.mouse.mouseMoved = (prevMousePos != gameInput.mouse.pos);

	// Keyboard
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

Mouse GameInput_GetMouse()
{
	return gameInput.mouse;
}

float GetMouseHoldTime(Mouse mouse)
{
	return ELAPSED(gameInput.time, mouse.tLastLeftPress); // Use gameInput.time here since tLastPressed is based gameInput.time (another "time" might have another starting point)
}
