#include "vector.h"
#include "input.h"
#include "renderer.h"
#include "string.h"

extern stbtt_bakedchar cdata[96]; //@nocommit

#define SHIP_ROTATION_SPEED 360 * 1.5f // Degrees per second.
#define SHIP_SPEED          20.0f
#define BULLET_SPEED        800.0f

struct Entity
{
	bool enabled;
	Vector2 pos;
	Vector2 facingV;
	Vector2 vel;
};

static Entity ship;
static Entity bullet;
static bool paused;

void GameInit()
{
	memset(&ship, 0, sizeof(ship));
	ship.facingV = VECTOR2_UP;
	ship.enabled = true;

	memset(&bullet, 0, sizeof(bullet));
	bullet.facingV = VECTOR2_UP;

	paused = false;
}

void GameUpdateAndRender(float deltaT, Vector2 screenDim, Renderer* renderer_p)
{
	float speed = 0;

	if (paused) goto GAMEUPDATE_END;

	if (GameInput_Button(BUTTON_RIGHT_ARROW))
	{
		ship.facingV = RotateDeg(ship.facingV, -SHIP_ROTATION_SPEED * deltaT);
	}
	if (GameInput_Button(BUTTON_LEFT_ARROW))
	{
		ship.facingV = RotateDeg(ship.facingV,  SHIP_ROTATION_SPEED * deltaT);
	}
	if (GameInput_Button(BUTTON_UP_ARROW))
	{
		speed = SHIP_SPEED;
	}
	if (GameInput_Button(BUTTON_LSHIFT))
	{
		speed *= 2.0f;
	}
	if (GameInput_ButtonDown(BUTTON_X))
	{
		bullet.pos = ship.pos;
		bullet.facingV = ship.facingV;
		bullet.vel = BULLET_SPEED * Normalize(ship.facingV);
		bullet.enabled = true;
	}
	if (GameInput_Button(BUTTON_A))
	{
		Vector2 leftFacingV = RotateDeg(ship.facingV, 90);
		ship.vel += 2* SHIP_SPEED * leftFacingV;
	}
	if (GameInput_Button(BUTTON_D))
	{
		Vector2 rightFacingV = RotateDeg(ship.facingV, -90);
		ship.vel += 2 * SHIP_SPEED * rightFacingV;
	}

	ship.vel += speed * ship.facingV;
	if (speed == 0.0f) ship.vel = 0.95f * ship.vel;
	ship.pos += deltaT * ship.vel;

	bullet.pos += deltaT * bullet.vel;

GAMEUPDATE_END:

	if (GameInput_ButtonDown(BUTTON_ESC))
	{
		paused = !paused;
	}

	// Wrap around
	if (ship.pos.x >  screenDim.x / 2.0f) ship.pos.x = -screenDim.x / 2.0f;
	if (ship.pos.x < -screenDim.x / 2.0f) ship.pos.x =  screenDim.x / 2.0f;
	if (ship.pos.y >  screenDim.y / 2.0f) ship.pos.y = -screenDim.y / 2.0f;
	if (ship.pos.y < -screenDim.y / 2.0f) ship.pos.y =  screenDim.y / 2.0f;

	if (bullet.enabled) PushSprite(renderer_p, bullet.pos, 50.0f * VECTOR2_ONE, bullet.facingV, 1);
	if (ship.enabled)   PushSprite(renderer_p, ship.pos, 75.0f * VECTOR2_ONE, ship.facingV, 0);

	char velStr[32] = { 0 };
	sprintf(velStr, "vel=%.02f", Magnitude(ship.vel));
	PushText(renderer_p, velStr, VECTOR2_ZERO, &cdata[0], 3);
}