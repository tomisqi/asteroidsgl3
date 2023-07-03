#include "vector.h"
#include "input.h"
#include "renderer.h"
#include "string.h"
#include "rect.h"
#include "ui.h"

#define SHIP_ROTATION_SPEED 360 * 1.5f // Degrees per second.
#define SHIP_SPEED          20.0f
#define BULLET_SPEED        800.0f

enum SceneE : U8
{
	SCENE_NONE,
	SCENE_MAIN_MENU,
	SCENE_GAME,
};

struct Entity
{
	bool enabled;
	Vector2 pos;
	Vector2 facingV;
	Vector2 vel;
};

static SceneE scene;
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

	scene = SCENE_MAIN_MENU;
}

static void Game(float deltaT, Vector2 screenDim, Renderer* renderer_p)
{
	if (GameInput_ButtonDown(BUTTON_ESC))
	{
		paused = !paused;
	}

	if (paused) goto GAMEUPDATE_END;

	if (GameInput_Button(BUTTON_RIGHT_ARROW))
	{
		ship.facingV = RotateDeg(ship.facingV, -SHIP_ROTATION_SPEED * deltaT);
	}
	if (GameInput_Button(BUTTON_LEFT_ARROW))
	{
		ship.facingV = RotateDeg(ship.facingV, SHIP_ROTATION_SPEED * deltaT);
	}
	if (GameInput_Button(BUTTON_UP_ARROW))
	{
		float speed = SHIP_SPEED;
		if (GameInput_Button(BUTTON_LSHIFT))
		{
			speed *= 2.0f;
		}
		ship.vel += speed * ship.facingV;
	}
	else
	{
		ship.vel = 0.95f * ship.vel;
	}

	if (GameInput_ButtonDown(BUTTON_X))
	{
		bullet.pos = ship.pos;
		bullet.facingV = ship.facingV;
		bullet.vel = ship.vel + BULLET_SPEED * Normalize(ship.facingV);
		bullet.enabled = true;
	}
	if (GameInput_Button(BUTTON_A))
	{
		Vector2 leftFacingV = RotateDeg(ship.facingV, 90);
		ship.vel += 2 * SHIP_SPEED * leftFacingV;
	}
	if (GameInput_Button(BUTTON_D))
	{
		Vector2 rightFacingV = RotateDeg(ship.facingV, -90);
		ship.vel += 2 * SHIP_SPEED * rightFacingV;
	}

	ship.pos += deltaT * ship.vel;
	bullet.pos += deltaT * bullet.vel;

	// Wrap around
	if (ship.pos.x > screenDim.x / 2.0f) ship.pos.x = -screenDim.x / 2.0f;
	if (ship.pos.x < -screenDim.x / 2.0f) ship.pos.x = screenDim.x / 2.0f;
	if (ship.pos.y > screenDim.y / 2.0f) ship.pos.y = -screenDim.y / 2.0f;
	if (ship.pos.y < -screenDim.y / 2.0f) ship.pos.y = screenDim.y / 2.0f;

GAMEUPDATE_END:

	if (bullet.enabled) PushSprite(renderer_p, bullet.pos, 50.0f * VECTOR2_ONE, bullet.facingV, 1);
	if (ship.enabled)   PushSprite(renderer_p, ship.pos, 75.0f * VECTOR2_ONE, ship.facingV, 0);

	if (paused)
	{
		if (Button("Continue", NewRect(VECTOR2_ZERO + V2(-120.0f, 100.0f), V2(250.0f, 50.0f))))
		{
			paused = false;
		}
		if (Button("Main Menu", NewRect(VECTOR2_ZERO + V2(-120.0f, 0.0f), V2(250.0f, 50.0f))))
		{
			GameInit();
			scene = SCENE_MAIN_MENU;
		}
	}
}

static bool MainMenu(Renderer* renderer_p)
{
	bool quitGame = false;
	if (Button("Start Game", NewRect(VECTOR2_ZERO + V2(-120.0f, 100.0f), V2(250.0f, 50.0f))))
	{
		scene = SCENE_GAME;
	}
	if (Button("Quit Game", NewRect(VECTOR2_ZERO + V2(-120.0f, 0.0f), V2(250.0f, 50.0f))))
	{
		quitGame = true;
	}
	return quitGame;
}

bool GameUpdateAndRender(float deltaT, Vector2 screenDim, Renderer* renderer_p)
{
	bool quitGame = false;
	switch (scene)
	{
	case SCENE_MAIN_MENU:
		quitGame = MainMenu(renderer_p);
		break;
	case SCENE_GAME:
		Game(deltaT, screenDim, renderer_p);
		break;
	case NONE:
	default:
		break;
	}

	return quitGame;
}
