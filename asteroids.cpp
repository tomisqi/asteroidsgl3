#include "vector.h"
#include "input.h"
#include "renderer.h"

void GameUpdateAndRender(float deltaT, Vector2 screenDim, Renderer* renderer_p)
{
	static Vector2 facingV = V2(0, 1);
	const float rotSpeed = 360 * 1.5f; // DegPerSecond
	float speed = 0;
	static Vector2 vel = V2(0, 0);
	static Vector2 pos = V2(0, 0);

	if (GameInput_Button(BUTTON_RIGHT_ARROW))
	{
		facingV = RotateDeg(facingV, -rotSpeed * deltaT);
	}
	if (GameInput_Button(BUTTON_LEFT_ARROW))
	{
		facingV = RotateDeg(facingV,  rotSpeed * deltaT);
	}
	if (GameInput_Button(BUTTON_UP_ARROW))
	{
		speed = 10.0f;
	}
	if (GameInput_Button(BUTTON_LSHIFT))
	{
		speed *= 2.0f;
	}
	if (GameInput_Button(BUTTON_C))
	{
		// Reset
		pos = VECTOR2_ZERO;
		vel = VECTOR2_ZERO;
		facingV = V2(0, 1);
	}

	vel += speed * facingV;
	if (speed == 0.0f) vel = 0.99f * vel;
	pos += deltaT * vel;

	// Wrap around
	if (pos.x >  screenDim.x / 2.0f) pos.x = -screenDim.x / 2.0f;
	if (pos.x < -screenDim.x / 2.0f) pos.x =  screenDim.x / 2.0f;
	if (pos.x >  screenDim.y / 2.0f) pos.x = -screenDim.y / 2.0f;
	if (pos.x < -screenDim.y / 2.0f) pos.x =  screenDim.y / 2.0f;

	PushSprite(renderer_p, pos, 50.0f * VECTOR2_ONE, facingV, 0);
}