#include "vector.h"
#include "input.h"
#include "renderer.h"
#include "string.h"
#include "rect.h"
#include "ui.h"
#include "timing.h"

#define SHIP_ROTATION_SPEED 360 * 1.5f // Degrees per second.
#define SHIP_ACCELERATION   1000.0f
#define BULLET_SPEED        800.0f
#define MAX_BULLETS         10
#define BULLET_LIFETIME     5.0f
#define MAX_ENTITIES        1<<8
enum SceneE : U8
{
	SCENE_NONE,
	SCENE_MAIN_MENU,
	SCENE_GAME,
};

enum EntityTypeE : U32
{
	ENTITY_NONE            = 0,
	ENTITY_PLAYERSPACESHIP = 1 << 1,
	ENTITY_BULLET          = 1 << 2,
	ENTITY_ASTEROID        = 1 << 3,
};

enum MenuScreenE: U8
{
	MENU_NONE,
	MENU_MAIN,
	MENU_SETTINGS,
};

struct Entity
{
	EntityTypeE type;
	bool enabled;
	double tEnabled;
	float size;
	Vector2 pos;
	Vector2 facingV;
	Vector2 vel;
};

struct CollisionEntities
{
	int count;
	Entity* entities_p[MAX_ENTITIES];
};

static SceneE scene;
static Entity ship;
static Entity asteroid;
static CollisionEntities collisions;
static int bulletIdx;
static Entity bullets[MAX_BULLETS];
static bool paused;
static MenuScreenE mainMenuScreen;
static double time;

void GameInit()
{
	scene = SCENE_MAIN_MENU;
	mainMenuScreen = MENU_MAIN;
	time = 0;
}

static void GameStart()
{
	memset(&ship, 0, sizeof(ship));
	ship.type = ENTITY_PLAYERSPACESHIP;
	ship.size = 75.0f;
	ship.facingV = VECTOR2_UP;
	ship.enabled = true;

	memset(&bullets, 0, sizeof(bullets));
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		bullets[i].type = ENTITY_BULLET;
		bullets[i].size = 50.0f;
		bullets[i].facingV = VECTOR2_UP;
	}

	memset(&asteroid, 0, sizeof(asteroid));
	asteroid.type = ENTITY_ASTEROID;
	asteroid.size = 55.0f;
	//asteroid.enabled = true;

	memset(&collisions, 0, sizeof(collisions));

	paused = false;
}

static void AddToCollisions(CollisionEntities* collisions_p, Entity* entity_p)
{
	collisions_p->entities_p[collisions_p->count++] = entity_p;
}

static void CheckCollisions(CollisionEntities* collisions_p)
{
	for (int i = 0; i < collisions_p->count; i++)
	{
		Entity* entityA_p = collisions_p->entities_p[i];
		float radiusSqA = entityA_p->size * entityA_p->size / 2;
		for (int j = i+1; j < collisions_p->count; j++)
		{
			Entity* entityB_p = collisions_p->entities_p[j];
			float radiusSqB = entityB_p->size * entityB_p->size / 2;

			float distSq = MagnitudeSq(entityA_p->pos - entityB_p->pos);
			if (distSq < radiusSqA || distSq < radiusSqB)
			{
				U32 collision = (U32)entityA_p->type | (U32)entityB_p->type;
				switch (collision)
				{
				case ENTITY_BULLET | ENTITY_ASTEROID:
				{
					Entity* bullet_p = entityA_p->type == ENTITY_BULLET ? entityA_p : entityB_p;
					bullet_p->enabled = false;
				}
				break;
				case ENTITY_PLAYERSPACESHIP | ENTITY_ASTEROID:
				{
					Entity* ship_p = entityA_p->type == ENTITY_PLAYERSPACESHIP ? entityA_p : entityB_p;
					ship_p->pos = VECTOR2_ZERO;
					ship_p->vel = VECTOR2_ZERO;
					ship_p->facingV = VECTOR2_UP;
				}
				break;
				default:
					break;
				}
			}
		}
	}
}

static void Game(float deltaT, Vector2 screenDim, Renderer* renderer_p)
{
	if (paused) goto GAMEUPDATE_END;

	time += deltaT;

	if (GameInput_Button(BUTTON_RIGHT_ARROW))
	{
		ship.facingV = RotateDeg(ship.facingV, -SHIP_ROTATION_SPEED * deltaT);
	}

	if (GameInput_Button(BUTTON_LEFT_ARROW))
	{
		ship.facingV = RotateDeg(ship.facingV, SHIP_ROTATION_SPEED * deltaT);
	}

	Vector2 prevVel = ship.vel;
	if (GameInput_Button(BUTTON_UP_ARROW))
	{
		float acceleration = SHIP_ACCELERATION;
		if (GameInput_Button(BUTTON_LSHIFT))
		{
			acceleration *= 4.0f;
		}
		ship.vel += acceleration * deltaT * Normalize(ship.facingV);
	}
	if (GameInput_Button(BUTTON_A))
	{
		Vector2 leftFacingV = RotateDeg(ship.facingV, 90);
		ship.vel += SHIP_ACCELERATION * deltaT * Normalize(leftFacingV);
	}
	if (GameInput_Button(BUTTON_D))
	{
		Vector2 rightFacingV = RotateDeg(ship.facingV, -90);
		ship.vel += SHIP_ACCELERATION * deltaT * Normalize(rightFacingV);
	}

	if (ship.vel == prevVel)
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
		bullet_p->tEnabled = time;
	}

	collisions.count = 0;

	ship.pos += deltaT * ship.vel;
	AddToCollisions(&collisions, &ship);

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Entity* bullet_p = &bullets[i];
		if (bullet_p->enabled)
		{
			bullet_p->pos += deltaT * bullet_p->vel;

			AddToCollisions(&collisions, bullet_p);

			if ((time - bullet_p->tEnabled) > BULLET_LIFETIME) bullet_p->enabled = false;
		}
	}

	if (asteroid.enabled)
	{
		asteroid.pos = V2(ship.pos.y + 100, ship.pos.x - 100);
		AddToCollisions(&collisions, &asteroid);
	}

	// Wrap around
	if (ship.pos.x > screenDim.x / 2.0f) ship.pos.x = -screenDim.x / 2.0f;
	if (ship.pos.x < -screenDim.x / 2.0f) ship.pos.x = screenDim.x / 2.0f;
	if (ship.pos.y > screenDim.y / 2.0f) ship.pos.y = -screenDim.y / 2.0f;
	if (ship.pos.y < -screenDim.y / 2.0f) ship.pos.y = screenDim.y / 2.0f;

	CheckCollisions(&collisions);

	PushLine(renderer_p, V2(-380, 0), V2(380, 0), V3(0.5f, 0.5f, 0.5f));
	PushLine(renderer_p, V2(0, -380), V2(0, 380), V3(0.5f, 0.5f, 0.5f));

GAMEUPDATE_END:

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Entity* bullet_p = &bullets[i];
		if (bullet_p->enabled)  PushSprite(renderer_p, bullet_p->pos, bullet_p->size * VECTOR2_ONE, bullet_p->facingV, 1);
	}
	if (ship.enabled) PushSprite(renderer_p, ship.pos, ship.size * VECTOR2_ONE, ship.facingV, 0);

	if (asteroid.enabled) PushSprite(renderer_p, asteroid.pos, asteroid.size * VECTOR2_ONE, asteroid.facingV, 4, V3(0.54f, 0.27f, 0.07f));

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
		if (UIButton("Go Back", NewRect(VECTOR2_ZERO + V2(-120.0f, 100.0f), V2(250.0f, 50.0f))) || GameInput_ButtonDown(BUTTON_ESC))
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
