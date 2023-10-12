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

void GameInput_NewFrame(ButtonState newButtonStates[], bool mouseIsPressed, Vector2 mousePosScreen, Vector2 screenDim, float deltaT)
{
	gameInput.time += deltaT;

	MouseStateE prevState = gameInput.mouse.state;
	gameInput.mouse.state = MOUSE_RELEASED;
	if (mouseIsPressed)
	{
		gameInput.mouse.state = MOUSE_PRESSED_HOLD;
		if (prevState == MOUSE_RELEASED)
		{
			gameInput.mouse.state = MOUSE_PRESSED;
			if (ELAPSED(gameInput.time, gameInput.mouse.tLastPress) < 0.2f) gameInput.mouse.state = MOUSE_DOUBLECLICK;
			gameInput.mouse.tLastPress = gameInput.time;
		}
	}

	Vector2 prevMousePos = gameInput.mouse.pos;
	gameInput.mouse.pos = V2(mousePosScreen.x, screenDim.y - mousePosScreen.y); // MousePosScreen.x is Ok, but we flip MousePosScreen.y.
	gameInput.mouse.mouseMoved = (prevMousePos != gameInput.mouse.pos);

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
	return ELAPSED(gameInput.time, mouse.tLastPress); // Use gameInput.time here since tLastPressed is based gameInput.time (another "time" might have another starting point)
}
