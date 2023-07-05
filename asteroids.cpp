#include "vector.h"
#include "input.h"
#include "renderer.h"
#include "string.h"
#include "rect.h"
#include "ui.h"

#define SHIP_ROTATION_SPEED 360 * 1.5f // Degrees per second.
#define SHIP_ACCELERATION   1000.0f
#define BULLET_SPEED        800.0f
#define MAX_BULLETS         10

enum SceneE : U8
{
	SCENE_NONE,
	SCENE_MAIN_MENU,
	SCENE_GAME,
};

enum MenuScreenE: U8
{
	MENU_NONE,
	MENU_MAIN,
	MENU_SETTINGS,
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
static Entity bullets[MAX_BULLETS];
static int bulletIdx;
static bool paused;
static MenuScreenE mainMenuScreen;

void GameInit()
{
	scene = SCENE_MAIN_MENU;
	mainMenuScreen = MENU_MAIN;
}

static void GameStart()
{
	memset(&ship, 0, sizeof(ship));
	ship.facingV = VECTOR2_UP;
	ship.enabled = true;

	memset(&bullets, 0, sizeof(bullets));
	for (int i = 0; i < MAX_BULLETS; i++) bullets[i].facingV = VECTOR2_UP;

	paused = false;
}

static void Game(float deltaT, Vector2 screenDim, Renderer* renderer_p)
{
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
		float acceleration = SHIP_ACCELERATION;
		if (GameInput_Button(BUTTON_LSHIFT))
		{
			acceleration *= 2.0f;
		}
		ship.vel += acceleration * deltaT * Normalize(ship.facingV);
	}
	else
	{
		ship.vel = 0.97f * ship.vel;
	}

	if (GameInput_ButtonDown(BUTTON_X))
	{
		Entity* bullet_p = &bullets[(bulletIdx++) % MAX_BULLETS];
		bullet_p->pos = ship.pos;
		bullet_p->facingV = ship.facingV;
		bullet_p->vel = ship.vel + BULLET_SPEED * Normalize(ship.facingV);
		bullet_p->enabled = true;
	}
	if (GameInput_Button(BUTTON_A))
	{
		Vector2 leftFacingV = RotateDeg(ship.facingV, 90);
		ship.vel += 2 * 20.0f * leftFacingV;
	}
	if (GameInput_Button(BUTTON_D))
	{
		Vector2 rightFacingV = RotateDeg(ship.facingV, -90);
		ship.vel += 2 * 20.0f * rightFacingV;
	}

	ship.pos += deltaT * ship.vel;

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Entity* bullet_p = &bullets[i];
		bullet_p->pos += deltaT * bullet_p->vel;
	}

	// Wrap around
	if (ship.pos.x > screenDim.x / 2.0f) ship.pos.x = -screenDim.x / 2.0f;
	if (ship.pos.x < -screenDim.x / 2.0f) ship.pos.x = screenDim.x / 2.0f;
	if (ship.pos.y > screenDim.y / 2.0f) ship.pos.y = -screenDim.y / 2.0f;
	if (ship.pos.y < -screenDim.y / 2.0f) ship.pos.y = screenDim.y / 2.0f;

GAMEUPDATE_END:

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Entity* bullet_p = &bullets[i];
		if (bullet_p->enabled)
		{
			PushSprite(renderer_p, bullet_p->pos, 50.0f * VECTOR2_ONE, bullet_p->facingV, 1);
		}
	}
	if (ship.enabled)   PushSprite(renderer_p, ship.pos, 75.0f * VECTOR2_ONE, ship.facingV, 0);

	if (paused)
	{
		UILayout("PausedMenu");
		{
			if (UIButton("Continue", NewRect(VECTOR2_ZERO + V2(-120.0f, 100.0f), V2(250.0f, 50.0f))))
			{
				printf("Continue\n");
				paused = false;
			}
			if (UIButton("Main Menu", NewRect(VECTOR2_ZERO + V2(-120.0f, 0.0f), V2(250.0f, 50.0f))))
			{
				printf("Main Menu\n");
				scene = SCENE_MAIN_MENU;
			}
		}
	}

	if (GameInput_ButtonDown(BUTTON_ESC))
	{
		paused = !paused;
	}
}

static bool MainMenuMain()
{
	bool quitGame = false;
	UILayout("MainMenu");
	{
		if (UIButton("Start Game", NewRect(VECTOR2_ZERO + V2(-120.0f, 100.0f), V2(250.0f, 50.0f))))
		{
			printf("Start Game\n");
			GameStart();
			scene = SCENE_GAME;
		}
		if (UIButton("Settings", NewRect(VECTOR2_ZERO + V2(-120.0f, 0.0f), V2(250.0f, 50.0f))))
		{
			printf("Settings\n");
			mainMenuScreen = MENU_SETTINGS;
		}
		if (UIButton("Quit Game", NewRect(VECTOR2_ZERO + V2(-120.0f, -100.0f), V2(250.0f, 50.0f))))
		{
			printf("Quit Game\n");
			quitGame = true;
		}
	}
	return quitGame;
}

static void Settings()
{
	UILayout("Settings");
	{
		if (UIButton("Go Back", NewRect(VECTOR2_ZERO + V2(-120.0f, 100.0f), V2(250.0f, 50.0f))))
		{
			printf("Go Back\n");
			mainMenuScreen = MENU_MAIN;
		}
	}
}

static bool MainMenu()
{
	bool quitGame = false;
	switch (mainMenuScreen)
	{
	case MENU_MAIN:
		quitGame = MainMenuMain();
		break;
	case MENU_SETTINGS:
		Settings();
		break;
	case NONE:
	default:
		break;
	}
	return quitGame;
}

bool GameUpdateAndRender(float deltaT, Vector2 screenDim, Renderer* renderer_p)
{
	bool quitGame = false;
	switch (scene)
	{
	case SCENE_MAIN_MENU:
		quitGame = MainMenu();
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
