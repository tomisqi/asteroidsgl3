#include "vector.h"
#include "input.h"
#include "renderer.h"
#include "string.h"
#include "rect.h"
#include "ui.h"
#include "timing.h"
#include "intersect.h"
#include "color.h"

#define SHIP_ROTATION_SPEED 360 * 1.5f // Degrees per second.
#define SHIP_ACCELERATION   1000.0f
#define SHIP_MAX_SPEED      1000.0f
#define BULLET_SPEED        800.0f
#define MAX_BULLETS         10
#define BULLET_LIFETIME     5.0f
#define MAX_ENTITIES        1<<8
#define MAX_SOLIDS          8

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

struct Camera
{
	Rect rect;
};

struct Level
{
	int solidsCount;
	LineSegment solids[MAX_SOLIDS];
};

extern Vector2 ScreenDim;
static SceneE scene;
static Camera camera;
static Level level;
static Entity ship;
static Entity asteroid;
static CollisionEntities entityCollisions;
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

static void BuildLevel(Level* level_p)
{
	level_p->solids[0] = { V2(100, 100), V2(100, 1400) };
	level_p->solids[1] = { V2(1400, 100), V2(1400, 1400) };
	level_p->solids[2] = { V2(100, 1400), V2(1400, 1400) };
	level_p->solids[3] = { V2(400, 100), V2(1400, 100) };

	level_p->solids[4] = { V2(400, 400), V2(400, 1100) };
	level_p->solids[5] = { V2(1100, 400), V2(1100, 1100) };
	level_p->solids[6] = { V2(400, 1100), V2(1100, 1100) };
	level_p->solids[7] = { V2(400, 400), V2(1100, 400) };

	level_p->solidsCount = 8;
}

static void GameStart()
{
	memset(&camera, 0, sizeof(camera));
	camera.rect = NewRectCenterPos(VECTOR2_ZERO, 2*ScreenDim);

	memset(&level, 0, sizeof(level));
	BuildLevel(&level);

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

	memset(&entityCollisions, 0, sizeof(entityCollisions));

	paused = false;
}

static void AddToCollisions(CollisionEntities* collisions_p, Entity* entity_p)
{
	collisions_p->entities_p[collisions_p->count++] = entity_p;
}


static void EntityEntityCollisions(CollisionEntities* collisions_p)
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

static void AddForce(Entity* entity_p, Vector2 force, float deltaT)
{
	const float mass = 1.0f;
	entity_p->vel = (deltaT/mass) * force;
}

static void EntityLevelCollisions(CollisionEntities* entities_p, float deltaT, Level* level_p)
{
	for (int i = 0; i < entities_p->count; i++)
	{
		Entity* entity_p = entities_p->entities_p[i];

		for (int s = 0; s < level_p->solidsCount; s++)
		{
			LineSegment solidLine = level_p->solids[s];
			Vector2 p;
			if (LineCircleIntersect(solidLine, entity_p->pos, entity_p->size/2, p))
			{
				switch (entity_p->type)
				{
				case ENTITY_BULLET:
				{
					Entity* bullet_p = entity_p;
					Vector2 normal = GetNormal(solidLine, bullet_p->pos);
					float angle = AngleDegRel(-bullet_p->vel, normal);
					bullet_p->vel = RotateDeg(-bullet_p->vel, 2 * angle);
					bullet_p->facingV = Normalize(bullet_p->vel);
					break;
				}
				case ENTITY_PLAYERSPACESHIP:
				{
					Entity* ship_p = entity_p;
					Vector2 normal = GetNormal(solidLine, ship_p->pos);
					float angle = AngleDegRel(-ship_p->vel, normal);
					ship_p->vel = RotateDeg(-ship_p->vel, 2 * angle);
					break;
				}
				default:
					break;
				}
			}
		}
	}
}

static bool PausedMenu()
{
	bool paused = true;
	UILayout("PausedMenu");
	{
		if (UIButton("Continue", NewRect(VECTOR2_ZERO + V2(-120.0f, 100.0f), V2(250.0f, 50.0f))))
		{
			printf("Continue\n");
			paused = false;
		}
		if (UIButton("Restart", NewRect(VECTOR2_ZERO + V2(-120.0f, 0.0f), V2(250.0f, 50.0f))))
		{
			printf("Restart\n");
			paused = false;
			GameStart();
		}
		if (UIButton("Main Menu", NewRect(VECTOR2_ZERO + V2(-120.0f, -100.0f), V2(250.0f, 50.0f))))
		{
			printf("Main Menu\n");
			scene = SCENE_MAIN_MENU;
		}
	}
	return paused;
}


static void Game(float deltaT, Renderer* renderer_p)
{
	if (paused) goto GAMEUPDATE_END;

	time += deltaT;

	Vector2 prevFacingV = ship.facingV;
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
		if (GameInput_Button(BUTTON_LSHIFT)) acceleration *= 4.0f;
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

	if (!GameInput_Button(BUTTON_UP_ARROW))
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

	entityCollisions.count = 0;

	ship.pos += deltaT * ship.vel;
	AddToCollisions(&entityCollisions, &ship);
	PushVector(renderer_p, ship.pos -50.0f * Normalize(ship.pos), -20.0f*Normalize(ship.pos));
	char buf[32] = { 0 };
	sprintf(buf, "%.02f\n", Magnitude(ship.vel));
	PushText(renderer_p, buf, V2(-380, 380), COLOR_WHITE);

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Entity* bullet_p = &bullets[i];
		if (bullet_p->enabled)
		{
			bullet_p->pos += deltaT * bullet_p->vel;
			//PushCircle(renderer_p, bullet_p->pos, bullet_p->size/2, V3(0, 1, 0));

			AddToCollisions(&entityCollisions, bullet_p);

			if ((time - bullet_p->tEnabled) > BULLET_LIFETIME) bullet_p->enabled = false;
		}
	}

	if (asteroid.enabled)
	{
		asteroid.pos = V2(500, 100);
		PushCircle(renderer_p, asteroid.pos, asteroid.size / 2, COLOR_GREEN);
		AddToCollisions(&entityCollisions, &asteroid);
	}

	EntityEntityCollisions(&entityCollisions);
	EntityLevelCollisions(&entityCollisions, deltaT, &level);

	//PushLine(renderer_p, V2(-1380, 0), V2(1380, 0), V3(0.5f, 0.5f, 0.5f));
	//PushLine(renderer_p, V2(0, -1380), V2(0, 1380), V3(0.5f, 0.5f, 0.5f));

	for (int i = 0; i < MAX_SOLIDS; i++)
	{
		LineSegment solid = level.solids[i];
		PushLine(renderer_p, solid.p1, solid.p2, COLOR_GREEN);
	}

	//camera.rect.size = V2(800, 800) + (Magnitude(ship.vel) / 2000) * V2(800, 800);
	camera.rect = NewRectCenterPos(ship.pos, camera.rect.size);
	SetSpritesOrtographicProj(renderer_p, camera.rect);
	SetWireframeOrtographicProj(renderer_p, camera.rect);

GAMEUPDATE_END:

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Entity* bullet_p = &bullets[i];
		if (bullet_p->enabled)  PushSprite(renderer_p, bullet_p->pos, bullet_p->size * VECTOR2_ONE, bullet_p->facingV, 1);
	}
	if (ship.enabled) PushSprite(renderer_p, ship.pos, ship.size * VECTOR2_ONE, ship.facingV, 0);

	if (asteroid.enabled) PushSprite(renderer_p, asteroid.pos, asteroid.size * VECTOR2_ONE, asteroid.facingV, 4, Col(0.54f, 0.27f, 0.07f));

	if (paused) paused = PausedMenu();

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

bool GameUpdateAndRender(float deltaT, Renderer* renderer_p)
{
	bool quitGame = false;
	switch (scene)
	{
	case SCENE_MAIN_MENU:
		quitGame = MainMenu();
		break;
	case SCENE_GAME:
		Game(deltaT, renderer_p);
		break;
	case NONE:
	default:
		break;
	}

	return quitGame;
}
