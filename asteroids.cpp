#include <assert.h>
#include "vector.h"
#include "input.h"
#include "renderer.h"
#include "string.h"
#include "rect.h"
#include "ui.h"
#include "timing.h"
#include "intersect.h"
#include "color.h"
#include "utils.h"
#include "animation.h"

#define SHIP_ROTATION_SPEED    360 * 1.5f // Degrees per second.
#define SHIP_ACCELERATION      1000.0f
#define SHIP_BOOST             (4 * SHIP_ACCELERATION)
#define SHIP_MAX_SPEED         1000.0f
#define BULLET_SPEED           3000.0f
#define MAX_BULLETS            20
#define BULLET_LIFETIME        2.0f
#define MAX_ENTITIES           1<<8
#define MAX_SOLIDS             15
#define MAX_ASTEROIDS          100
#define INVISIBILITY_DURATION  1
#define TAKINGDAMAGE_DURATION  1
#define SHIP_DEATH_DURATION    3
#define ASTEROID_ROT_SPEED_MIN 10
#define ASTEROID_ROT_SPEED_MAX 20
#define ASTEROID_SIZE_MIN      80
#define ASTEROID_SIZE_MAX      280
#define ASTEROID_BIG           (ASTEROID_SIZE_MIN + (ASTEROID_SIZE_MAX - ASTEROID_SIZE_MIN) / 2)
#define ASTEROID_SPEED_MIN     50
#define ASTEROID_SPEED_MAX     60
#define PARTICLE_DEBRIS_LIFETIME      3.0f
#define PARTICLE_EXHAUST_LIFETIME     0.3f
#define MAX_PARTICLES_DEBRIS   100
#define MAX_PARTICLES_EXHAUST  200
#define MAX_CHARGEDBULLETS     8
#define CHARGEDBULLET_SPEED    2000.0f
#define EXHAUST_FREQUENCY      10.0f
#define COLOR_EXHAUST          Col(0.6f, 0.8f, 1.0f, 1.0f);
#define COLOR_EXHAUST_BOOST    Col(0.957f, 1.0f, 0.475f, 1.0f);
#define MAX_EXPLOSIONS_SMALL   16
#define MAX_TURRETS            2
#define SPEED_DESTROY          2000.0f

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
	ENTITY_ENEMYBULLET     = 1 << 4,
	ENTITY_CHARGEDBULLET   = 1 << 5,
	ENTITY_TURRET    = 1 << 6,
};

enum MenuScreenE: U8
{
	MENU_NONE,
	MENU_MAIN,
	MENU_SETTINGS,
};

struct Turret
{
	double tNextMove;
	int nextAngle;
	int prevAngle;
};

struct Entity
{
	EntityTypeE type;
	bool enabled;
	double tEnabled;
	double tInvisibility;
	double tTakingDamage;
	float size;
	Vector2 pos;
	Vector2 facingV;
	Vector2 vel;
	float rotSpeed;
	float colliderRadius;
	float health;

	union
	{
		struct
		{ // Turret
			double tNextMove;
			int nextAngle;
			int prevAngle;
		};
		struct
		{ // Spaceship
			double tRespawn;
		};
	} e;

	TextureHandleT textureHandle;
	Rect uv;
};

struct Particle
{
	bool enabled;
	double tEnabled;

	Vector2 pos;
	Vector2 vel;
	Color color;
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

struct Solid
{
	int solidLinesCount;
	LineSegment solidLines[MAX_SOLIDS];
};

struct Level
{
	int level;
	int asteroidsCount;
};

struct ParticleSystem
{
	int index;
	Particle* particles_p;
};

struct AnimationObject
{
	bool enabled;
	Vector2 pos;
	TextureHandleT textureHandle;
	Animation animation;
};

extern Vector2 ScreenDim;
static SceneE scene;
static Camera camera;
static Solid solid;
static Entity ship;
static Entity asteroids[MAX_ASTEROIDS];
static CollisionEntities entityCollisions;
static int bulletIdx;
static Entity bullets[MAX_BULLETS];
static int enemyBulletIdx;
static Entity enemyBullets[MAX_BULLETS];
static int chargedBulletIdx;
static Entity chargedBullets[MAX_CHARGEDBULLETS];
static Entity* chargedBulletHolding_p = nullptr;
static bool paused;
static MenuScreenE mainMenuScreen;
static double time;
static int score;
static int asteroidsRemaining = 0;
static Level level;
static ParticleSystem psExhaust;
static ParticleSystem psDebris;
static AnimationObject explosionCharged;
static int explosionsSmallIdx;
static AnimationObject explosionsSmall[MAX_EXPLOSIONS_SMALL];
static AnimationObject explosionShip;
static Entity turrets[MAX_TURRETS];

static bool PausedMenu();

void GameInit()
{
	scene = SCENE_MAIN_MENU;
	mainMenuScreen = MENU_MAIN;
	time = 0;

	memset(&psDebris, 0, sizeof(psDebris));
	psDebris.index = 0;
	psDebris.particles_p = (Particle*)malloc(MAX_PARTICLES_DEBRIS * sizeof(Particle));

	memset(&psExhaust, 0, sizeof(psExhaust));
	psExhaust.index = 0;
	psExhaust.particles_p = (Particle*)malloc(MAX_PARTICLES_EXHAUST * sizeof(Particle));
}

static void BuildSolid(Solid* solid_p)
{
	
	solid_p->solidLines[0] = { V2(100, 100), V2(100, 1400) };
	solid_p->solidLines[1] = { V2(1400, 100), V2(1400, 1400) };
	solid_p->solidLines[2] = { V2(100, 1400), V2(1200, 1400) };
	solid_p->solidLines[3] = { V2(400, 100), V2(1400, 100) };

	solid_p->solidLines[4] = { V2(400, 400), V2(400, 1100) };
	solid_p->solidLines[5] = { V2(1100, 400), V2(1100, 1100) };
	solid_p->solidLines[6] = { V2(400, 1100), V2(900, 1100) };
	solid_p->solidLines[7] = { V2(400, 400), V2(1100, 400) };
	
	solid_p->solidLines[8] = { V2(-3000, -3000), V2(3000, -3000) };
	solid_p->solidLines[9] = { V2(3000, -3000), V2(3000, 3000) };
	solid_p->solidLines[10]= { V2(3000, 3000), V2(-3000, 3000) };
	solid_p->solidLines[11]= { V2(-3000, 3000), V2(-3000, -3000) };

	solid_p->solidLines[12] = { V2(-2000, -2000), V2(-1500, -2000) };
	solid_p->solidLines[13] = { V2(-1500, -2000), V2(-1750, -1500) };
	solid_p->solidLines[14] = { V2(-1750, -1500), V2(-2000, -2000) };


	solid_p->solidLinesCount = 15;
}

static Vector2 GetRandomUvPos(Vector2 uvSize)
{
	int xCnt = 1.0f / uvSize.x;
	int yCnt = 1.0f / uvSize.y;
	return V2(GetRandomValue(0, xCnt-1)*uvSize.x, GetRandomValue(0, yCnt-1) * uvSize.y);
}

static Vector2 FindRandomPositionForAsteroid(float colliderRadius)
{
	Vector2 pos = VECTOR2_ZERO;
	bool found = false;
	while (!found)
	{
		found = true;
		pos = V2(GetRandomValue(-3000, 3000), GetRandomValue(-3000, 3000));
		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			Entity* asteroid_p = &asteroids[i];
			float distCollision = colliderRadius + asteroid_p->colliderRadius;
			float distCollisionSq = distCollision * distCollision;
			float distSq = MagnitudeSq(pos - asteroid_p->pos);
			if (distSq <= distCollisionSq) { found = false; break; }
		}
	}
	return pos;
}

static Level AdvanceLevel(Level prevLevel)
{
	Level level = { 0 };
	level.level = prevLevel.level + 1;
	level.asteroidsCount = prevLevel.asteroidsCount * 2;

	for (int i = 0; i < level.asteroidsCount; i++)
	{
		Entity* asteroid_p = &asteroids[i];
		asteroid_p->enabled = true;
		asteroid_p->tEnabled = time;
		asteroid_p->tInvisibility = time + INVISIBILITY_DURATION;
		asteroid_p->size = GetRandomValue(ASTEROID_SIZE_MIN, ASTEROID_SIZE_MAX);
		asteroid_p->colliderRadius = 0.7f * (asteroid_p->size / 2);
		asteroid_p->uv = NewRect(GetRandomUvPos(V2(0.25f, 0.25f)), V2(0.25f, 0.25f));
		asteroid_p->pos = FindRandomPositionForAsteroid(asteroid_p->colliderRadius);
		asteroid_p->rotSpeed = GetRandomSign() * GetRandomValue(ASTEROID_ROT_SPEED_MIN, ASTEROID_ROT_SPEED_MAX);
		asteroid_p->vel = GetRandomValue(ASTEROID_SPEED_MIN, ASTEROID_SPEED_MAX) * RotateDeg(VECTOR2_UP, GetRandomValue(0, 360));
	}

	return level;
}

static void GameStart()
{
	memset(&camera, 0, sizeof(camera));
	camera.rect = NewRectCenterPos(VECTOR2_ZERO, 4.0f*ScreenDim);

	memset(&solid, 0, sizeof(solid));
	BuildSolid(&solid);

	level.level = 1;
	level.asteroidsCount = 5;
	asteroidsRemaining = level.asteroidsCount;

	memset(&ship, 0, sizeof(ship));
	ship.type = ENTITY_PLAYERSPACESHIP;
	ship.size = 85.0f;
	ship.colliderRadius = 0.8f * (ship.size / 2);
	ship.facingV = VECTOR2_UP;
	ship.enabled = true;
	ship.tEnabled = time;
	ship.tInvisibility = time + INVISIBILITY_DURATION;
	ship.textureHandle = TEXTURE_SPACECRAFT;
	ship.uv = RECT_ONE;
	ship.health = 100.0f;

	memset(&bullets, 0, sizeof(bullets));
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		bullets[i].type = ENTITY_BULLET;
		bullets[i].size = 50.0f;
		bullets[i].colliderRadius = bullets[i].size / 2;
		bullets[i].facingV = VECTOR2_UP;
		bullets[i].textureHandle = TEXTURE_REDSHOT;
	}

	memset(&enemyBullets, 0, sizeof(enemyBullets));
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		enemyBullets[i].type = ENTITY_ENEMYBULLET;
		enemyBullets[i].size = 50.0f;
		enemyBullets[i].colliderRadius = enemyBullets[i].size / 2;
		enemyBullets[i].facingV = VECTOR2_UP;
		enemyBullets[i].textureHandle = TEXTURE_REDSHOT;
	}

	memset(&asteroids, 0, sizeof(asteroids));
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		asteroids[i].type = ENTITY_ASTEROID;
		asteroids[i].size = 100.0f;
		asteroids[i].colliderRadius = 0.7f * (asteroids[i].size / 2);
		asteroids[i].facingV = VECTOR2_UP;
		asteroids[i].enabled = false;
		asteroids[i].textureHandle = TEXTURE_ASTEROID;
		asteroids[i].pos = VECTOR2_ZERO;
		asteroids[i].uv = RECT_ONE;
		asteroids[i].rotSpeed = 0;
	}
	for (int i = 0; i < level.asteroidsCount; i++)
	{
		asteroids[i].enabled = true;
		asteroids[i].tEnabled = time;
		asteroids[i].tInvisibility = time + INVISIBILITY_DURATION;
		asteroids[i].size = GetRandomValue(ASTEROID_SIZE_MIN, ASTEROID_SIZE_MAX);
		asteroids[i].colliderRadius = 0.7 * (asteroids[i].size / 2);
		asteroids[i].uv = NewRect(GetRandomUvPos(V2(0.25f, 0.25f)), V2(0.25f, 0.25f));
		asteroids[i].rotSpeed = GetRandomSign() * GetRandomValue(ASTEROID_ROT_SPEED_MIN, ASTEROID_ROT_SPEED_MAX);
		asteroids[i].pos = FindRandomPositionForAsteroid(asteroids[i].colliderRadius);
		asteroids[i].vel = GetRandomValue(ASTEROID_SPEED_MIN, ASTEROID_SPEED_MAX) * RotateDeg(VECTOR2_UP, GetRandomValue(0, 360));
	}

	memset(turrets, 0, sizeof(turrets));
	for (int i = 0; i < MAX_TURRETS; i++)
	{
		turrets[i].type = ENTITY_TURRET;
		turrets[i].size = 150.0f;
		turrets[i].colliderRadius = turrets[i].size / 2;
		turrets[i].facingV = VECTOR2_UP;
		turrets[i].uv = RECT_ONE;
		turrets[i].pos = VECTOR2_ZERO;
		turrets[i].health = 100.0f;
		turrets[i].textureHandle = TEXTURE_TURRET;
		turrets[i].rotSpeed = 180.0f;
		turrets[i].e.tNextMove = F32_MAX;
		turrets[i].e.nextAngle = 45;
		turrets[i].e.prevAngle = 0;
	}
	turrets[0].enabled = true;
	turrets[0].tEnabled = time;
	turrets[0].tInvisibility = time + INVISIBILITY_DURATION;
	turrets[0].pos = V2(800.0f, -500.0f);
	turrets[1].enabled = true;
	turrets[1].tEnabled = time;
	turrets[1].tInvisibility = time + INVISIBILITY_DURATION;
	turrets[1].pos = V2(-600.0f, 800.0f);


	memset(chargedBullets, 0, sizeof(chargedBullets));
	for (int i = 0; i < MAX_CHARGEDBULLETS; i++)
	{
		chargedBullets[i].type = ENTITY_CHARGEDBULLET;
		chargedBullets[i].size = 60.0f;
		chargedBullets[i].colliderRadius = chargedBullets[i].size / 2;
		chargedBullets[i].facingV = VECTOR2_UP;
		chargedBullets[i].textureHandle = TEXTURE_CHARGEDBULLET;
	}
	chargedBulletHolding_p = nullptr;

	psDebris.index = 0;
	assert(psDebris.particles_p != nullptr);
	for (int i = 0; i < MAX_PARTICLES_DEBRIS; i++)
	{
		psDebris.particles_p[i].enabled = false;
		psDebris.particles_p[i].color = COLOR_WHITE;
	}

	psExhaust.index = 0;
	assert(psExhaust.particles_p != nullptr);
	for (int i = 0; i < MAX_PARTICLES_EXHAUST; i++)
	{
		psExhaust.particles_p[i].enabled = false;
		psExhaust.particles_p[i].color = COLOR_EXHAUST;
	}

	memset(&explosionShip, 0, sizeof(explosionShip));
	explosionShip.enabled = false;
	explosionShip.textureHandle = TEXTURE_EXPLOSIONBIG;
	explosionShip.animation = AnimationBuild(5, 2, 10, 24.0f, false);

	memset(&explosionCharged, 0, sizeof(explosionCharged));
	explosionCharged.enabled = false;
	explosionCharged.textureHandle = TEXTURE_EXPLOSION5;
	explosionCharged.animation = AnimationBuild(2, 2, 4, 24.0f, false);

	memset(&explosionsSmall, 0, sizeof(explosionsSmall));
	for (int i = 0; i < MAX_EXPLOSIONS_SMALL; i++)
	{
		explosionsSmall[i].enabled = false;
		explosionsSmall[i].textureHandle = TEXTURE_EXPLOSIONSMALL;
		explosionsSmall[i].animation = AnimationBuild(3, 3, 8, 24.0f, false);
	}

	memset(&entityCollisions, 0, sizeof(entityCollisions));

	score = 0;

	paused = false;
}

static void AddToCollisions(CollisionEntities* collisions_p, Entity* entity_p)
{
	collisions_p->entities_p[collisions_p->count++] = entity_p;
}

static int SpawnChildrenAsteroids(Vector2 pos)
{
	Entity* childrenAsteroids[4] = { 0 };
	int found = 0;
	int j = MAX_ASTEROIDS - 1;
	while (found < 4 && j >= 0)
	{
		if (!asteroids[j].enabled)
		{
			childrenAsteroids[found++] = &asteroids[j];
		}
		j--;
	}
	assert(found == 4);
	for (int i = 0; i < 4; i++)
	{
		Entity* child_p = childrenAsteroids[i];
		child_p->size = GetRandomValue(ASTEROID_SIZE_MIN, ASTEROID_SIZE_MIN + 10);
		child_p->colliderRadius = 0.7f * (child_p->size / 2);
		child_p->uv = NewRect(GetRandomUvPos(V2(0.25f, 0.25f)), V2(0.25f, 0.25f));
		child_p->pos = pos + (ASTEROID_SIZE_MIN/2) * RotateDeg(VECTOR2_ONE, 90*i);
		child_p->rotSpeed = GetRandomSign() * GetRandomValue(ASTEROID_ROT_SPEED_MIN, ASTEROID_ROT_SPEED_MAX);
		child_p->vel = 4 * GetRandomValue(ASTEROID_SPEED_MIN, ASTEROID_SPEED_MAX) * RotateDeg(VECTOR2_UP, GetRandomValue(0, 360));
		child_p->enabled = true;
		child_p->tEnabled = time;
	}
	return found;
}

static void SpawnDebrisParticles(Vector2 pos, int count)
{
	Particle* particles = psDebris.particles_p;
	for (int i = 0; i < count; i++)
	{
		Particle* particle_p = &particles[psDebris.index++ % MAX_PARTICLES_DEBRIS];
		particle_p->enabled = true;
		particle_p->pos = pos + 20.0f * RotateDeg(VECTOR2_UP, GetRandomValue(0, 360));
		particle_p->tEnabled = time;
		particle_p->vel = 30.0f * RotateDeg(VECTOR2_UP, GetRandomValue(0, 360));
	}
}

static void SpawnExhaustParticles(Vector2 pos, Vector2 facingV, Color colorParticle)
{
	Particle* particles = psExhaust.particles_p;
	for (int i = 0; i < 1; i++)
	{
		Particle* particle_p = &particles[psExhaust.index++ % MAX_PARTICLES_EXHAUST];
		particle_p->enabled = true;
		particle_p->pos = pos;
		particle_p->tEnabled = time;
		particle_p->vel = RotateDeg(80.0f * facingV, GetRandomValue(-135, 135));
		particle_p->color = colorParticle;
	}
}

static void EllasticCollision(Entity* entityA_p, Entity* entityB_p)
{
	// See https://en.wikipedia.org/wiki/Elastic_collision
	Vector2 v1 = entityA_p->vel;
	Vector2 v2 = entityB_p->vel;
	float m1 = entityA_p->size;
	float m2 = entityB_p->size;
	Vector2 x1 = entityA_p->pos;
	Vector2 x2 = entityB_p->pos;
	entityA_p->vel = v1 - (2 * m2 / (m1 + m2)) * (Dot(v1 - v2, x1 - x2) / MagnitudeSq(x1 - x2)) * (x1 - x2);
	entityB_p->vel = v2 - (2 * m1 / (m1 + m2)) * (Dot(v2 - v1, x2 - x1) / MagnitudeSq(x2 - x1)) * (x2 - x1);
}

static void EntityEntityCollisions(CollisionEntities* collisions_p)
{
	for (int i = 0; i < collisions_p->count; i++)
	{
		Entity* entityA_p = collisions_p->entities_p[i];
		for (int j = i+1; j < collisions_p->count; j++)
		{
			Entity* entityB_p = collisions_p->entities_p[j];
			float distCollision = entityA_p->colliderRadius + entityB_p->colliderRadius;
			float distCollisionSq = distCollision * distCollision;
			float distSq = MagnitudeSq(entityA_p->pos - entityB_p->pos);
			if (distSq <= distCollisionSq)
			{
				U32 collision = (U32)entityA_p->type | (U32)entityB_p->type;
				switch (collision)
				{
				case ENTITY_BULLET | ENTITY_ASTEROID:
				{
					Entity* bullet_p = entityA_p;
					Entity* asteroid_p = entityB_p;
					if (entityA_p->type == ENTITY_ASTEROID)
					{
						bullet_p = entityB_p;
						asteroid_p = entityA_p;
					}

					if (asteroid_p->size >= ASTEROID_BIG)
					{
						asteroidsRemaining += SpawnChildrenAsteroids(asteroid_p->pos);

					}

					AnimationObject* explosionSmall_p = &explosionsSmall[(explosionsSmallIdx++) % MAX_EXPLOSIONS_SMALL];
					explosionSmall_p->enabled = true;
					explosionSmall_p->pos = bullet_p->pos + bullet_p->colliderRadius * Normalize(asteroid_p->pos - bullet_p->pos);
					explosionSmall_p->animation.tStart = time;

					float sizePerc = (asteroid_p->size - ASTEROID_SIZE_MIN) / (ASTEROID_SIZE_MAX - ASTEROID_SIZE_MIN);
					int particleCount = (int)(20 * sizePerc + 10);
					SpawnDebrisParticles(asteroid_p->pos, particleCount);

					bullet_p->enabled = false;
					asteroid_p->enabled = false;
					asteroidsRemaining--;
					score++;
				}
				break;
				case ENTITY_CHARGEDBULLET | ENTITY_ASTEROID:
				{
					Entity* chargedBullet_p = entityA_p;
					Entity* asteroid_p = entityB_p;
					if (entityA_p->type == ENTITY_ASTEROID)
					{
						chargedBullet_p = entityB_p;
						asteroid_p = entityA_p;
					}

					if (asteroid_p->size >= ASTEROID_BIG)
					{
						asteroidsRemaining += SpawnChildrenAsteroids(asteroid_p->pos);
					}

					float sizePerc = (asteroid_p->size - ASTEROID_SIZE_MIN) / (ASTEROID_SIZE_MAX - ASTEROID_SIZE_MIN);
					int particleCount = (int)(20 * sizePerc + 10);
					SpawnDebrisParticles(asteroid_p->pos, particleCount);

					asteroid_p->enabled = false;
					asteroidsRemaining--;
					score++;
				}
				break;
				case ENTITY_PLAYERSPACESHIP | ENTITY_ASTEROID:
				{
					Entity* asteroid_p = entityA_p;
					Entity* ship_p = entityB_p;					
					if (entityA_p->type == ENTITY_PLAYERSPACESHIP)
					{
						asteroid_p = entityB_p;
						ship_p = entityA_p;
					}

					ship_p->tTakingDamage = time + TAKINGDAMAGE_DURATION;
					ship_p->health -= 20.0f;
					ship_p->health = Clampf(ship_p->health, 0, 100.0f);

					EllasticCollision(ship_p, asteroid_p);

					if (Magnitude(ship_p->vel) >= SPEED_DESTROY)
					{
						ship_p->health = 0;
						ship_p->vel = VECTOR2_ZERO; // Zero so that the camera doesn't continue following.
					}
				}
				break;
				case ENTITY_PLAYERSPACESHIP | ENTITY_ENEMYBULLET:
				{
					Entity* bullet_p = entityA_p;
					Entity* ship_p = entityB_p;
					if (entityA_p->type == ENTITY_PLAYERSPACESHIP)
					{
						bullet_p = entityB_p;
						ship_p = entityA_p;
					}

					SpawnDebrisParticles(bullet_p->pos, 5);

					bullet_p->enabled = false;
					AnimationObject* explosionSmall_p = &explosionsSmall[(explosionsSmallIdx++) % MAX_EXPLOSIONS_SMALL];
					explosionSmall_p->enabled = true;
					explosionSmall_p->pos = bullet_p->pos + bullet_p->colliderRadius * Normalize(ship_p->pos - bullet_p->pos);
					explosionSmall_p->animation.tStart = time;

					ship_p->tTakingDamage = time + TAKINGDAMAGE_DURATION;
					ship_p->health -= 20.0f;
					ship_p->health = Clampf(ship_p->health, 0, 100.0f);
				}
				break;
				case ENTITY_ASTEROID | ENTITY_ASTEROID:
				{
					Entity* asteroidA_p = entityA_p;
					Entity* asteroidB_p = entityB_p;

					Vector2 collisionP = asteroidA_p->pos + (asteroidA_p->size / 2) * Normalize(asteroidB_p->pos - asteroidA_p->pos);
					SpawnDebrisParticles(collisionP, 5);
					EllasticCollision(asteroidA_p, asteroidB_p);
				}
				break;
				case ENTITY_TURRET | ENTITY_BULLET:
				{
					Entity* bullet_p = entityA_p;
					Entity* turret_p = entityB_p;
					if (entityA_p->type == ENTITY_TURRET)
					{
						bullet_p = entityB_p;
						turret_p = entityA_p;
					}

					turret_p->health -= 25.0f;
					turret_p->health = Clampf(turret_p->health, 0, 100.0f);

					if (turret_p->health == 0)
					{
						explosionShip.enabled = true;
						explosionShip.pos = turret_p->pos;
						explosionShip.animation.tStart = time;

						turret_p->enabled = false;
					}

					bullet_p->enabled = false;
					AnimationObject* explosionSmall_p = &explosionsSmall[(explosionsSmallIdx++) % MAX_EXPLOSIONS_SMALL];
					explosionSmall_p->enabled = true;
					explosionSmall_p->pos = bullet_p->pos + bullet_p->colliderRadius * Normalize(turret_p->pos - bullet_p->pos);
					explosionSmall_p->animation.tStart = time;

					score++;

				}
				break;
				case ENTITY_TURRET | ENTITY_CHARGEDBULLET:
				{
					Entity* chargedBullet_p = entityA_p;
					Entity* turret_p = entityB_p;
					if (entityA_p->type == ENTITY_TURRET)
					{
						chargedBullet_p = entityB_p;
						turret_p = entityA_p;
					}

					turret_p->health = 0;
					turret_p->enabled = false;
					explosionShip.enabled = true;
					explosionShip.pos = turret_p->pos;
					explosionShip.animation.tStart = time;

					chargedBullet_p->enabled = false;
					explosionCharged.enabled = true;
					explosionCharged.pos = chargedBullet_p->pos;
					explosionCharged.animation.tStart = time;

					score++;
				}
				break;
				case ENTITY_TURRET | ENTITY_PLAYERSPACESHIP:
				{
					Entity* turret_p = entityA_p;
					Entity* ship_p = entityB_p;
					if (entityA_p->type == ENTITY_PLAYERSPACESHIP)
					{
						turret_p = entityB_p;
						ship_p = entityA_p;
					}

					ship_p->tInvisibility = time + INVISIBILITY_DURATION;
					ship_p->tTakingDamage = time + TAKINGDAMAGE_DURATION;
					ship_p->health -= 75.0f;
					ship_p->health = Clampf(ship_p->health, 0, 100.0f);
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

static void EntitySolidCollisions(CollisionEntities* entities_p, float deltaT, Solid* solid_p)
{
	for (int i = 0; i < entities_p->count; i++)
	{
		Entity* entity_p = entities_p->entities_p[i];

		for (int s = 0; s < solid_p->solidLinesCount; s++)
		{
			LineSegment solidLine = solid_p->solidLines[s];
			Vector2 p;
			if (LineCircleIntersect(solidLine, entity_p->pos, entity_p->colliderRadius, p))
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
				}
				break;
				case ENTITY_CHARGEDBULLET:
				{
					Entity* chargedBullet_p = entity_p;
					chargedBullet_p->enabled = false;

					explosionCharged.enabled = true;
					explosionCharged.pos = p;
					explosionCharged.animation.tStart = time;
				}
				break;
				case ENTITY_ENEMYBULLET:
				{
					Entity* bullet_p = entity_p;
					bullet_p->enabled = false;

					AnimationObject* explosionSmall_p = &explosionsSmall[(explosionsSmallIdx++) % MAX_EXPLOSIONS_SMALL];
					explosionSmall_p->enabled = true;
					explosionSmall_p->pos = p;
					explosionSmall_p->animation.tStart = time;
				}
				break;
				case ENTITY_PLAYERSPACESHIP:
				{
					Entity* ship_p = entity_p;
					Vector2 normal = GetNormal(solidLine, ship_p->pos);
					float angle = AngleDegRel(-ship_p->vel, normal);
					ship_p->vel = RotateDeg(-ship_p->vel, 2 * angle);

					if (Magnitude(ship_p->vel) >= SPEED_DESTROY)
					{
						ship_p->health = 0;
						ship_p->vel = VECTOR2_ZERO; // Zero so that the camera doesn't continue following.
					}
				}
				break;
				case ENTITY_ASTEROID:
				{
					Entity* asteroid_p = entity_p;
					Vector2 normal = GetNormal(solidLine, asteroid_p->pos);
					float angle = AngleDegRel(-asteroid_p->vel, normal);
					asteroid_p->vel = RotateDeg(-asteroid_p->vel, 2 * angle);
					break;
				}
				default:
					break;
				}
			}
		}
	}
}

static Vector2 MouseToWorldPos(Vector2 mousepos)
{
	float x = (mousepos.x / ScreenDim.x) * camera.rect.size.x + camera.rect.pos.x;
	float y = (mousepos.y / ScreenDim.y) * camera.rect.size.y + camera.rect.pos.y;
	return V2(x, y);
}

#define PERIOD_BLINK_SPRITE 0.1f
static bool Blink(double tStartBlink)
{
	bool show = true;
	float elapsed = time - tStartBlink;
	float cyclesElapsed = elapsed / PERIOD_BLINK_SPRITE;
	float posWithinCycle = cyclesElapsed - (int)cyclesElapsed;
	if (posWithinCycle >= 0.5f) show = false;
	return show;
}

static void Game(float deltaT, Renderer* renderer_p)
{
	float shipAcceleration = 0.0f;
	Vector2 shipAccelerationV = VECTOR2_ZERO;

	if (paused) goto GAMEUPDATE_END;

	time += deltaT;

	Mouse mouse = GameInput_GetMouse();
	mouse.pos = MouseToWorldPos(mouse.pos);
	
	ship.facingV = Normalize(mouse.pos - ship.pos);
	
	if (ship.enabled)
	{
		if (GameInput_Button(BUTTON_W))
		{
			shipAcceleration = SHIP_ACCELERATION;
			if (GameInput_Button(BUTTON_LSHIFT)) shipAcceleration = SHIP_BOOST;
			shipAccelerationV = ship.facingV;
		}
		if (GameInput_Button(BUTTON_S) && Magnitude(ship.vel) > 0.1f)
		{
			shipAcceleration = -2 * SHIP_BOOST;
			shipAccelerationV = Normalize(ship.vel);
		}
		if (GameInput_Button(BUTTON_A))
		{
			shipAcceleration = SHIP_ACCELERATION;
			Vector2 leftFacingV = RotateDeg(ship.facingV, 90);
			shipAccelerationV = leftFacingV;
		}
		if (GameInput_Button(BUTTON_D))
		{
			shipAcceleration = SHIP_ACCELERATION;
			Vector2 rightFacingV = RotateDeg(ship.facingV, -90);
			shipAccelerationV = rightFacingV;
		}

		if (mouse.leftButton == MOUSE_PRESSED || mouse.leftButton == MOUSE_DOUBLECLICK)
		{
			Entity* bullet_p = &bullets[(bulletIdx++) % MAX_BULLETS];
			bullet_p->pos = ship.pos;
			bullet_p->facingV = ship.facingV;
			bullet_p->vel = ship.vel + BULLET_SPEED * Normalize(ship.facingV);
			bullet_p->enabled = true;
			bullet_p->tEnabled = time;
		}

		if ((mouse.leftButton == MOUSE_PRESSED_HOLD) && (GetMouseHoldTime(mouse) > 0.5f) && !chargedBulletHolding_p)
		{
			Entity* bullet_p = &chargedBullets[(chargedBulletIdx++) % MAX_CHARGEDBULLETS];
			bullet_p->pos = ship.pos + 20.0f * ship.facingV;
			bullet_p->facingV = ship.facingV;
			bullet_p->vel = VECTOR2_ZERO;
			bullet_p->enabled = true;
			bullet_p->tEnabled = time;
			chargedBulletHolding_p = bullet_p;
		}
	}

	ship.vel += shipAcceleration * deltaT * Normalize(shipAccelerationV);
	if (shipAcceleration == 0.0f)
	{
		ship.vel = 0.97f * ship.vel;
	}

	if (chargedBulletHolding_p)
	{
		chargedBulletHolding_p->facingV = ship.facingV;
		chargedBulletHolding_p->pos = ship.pos + 20.0f * ship.facingV;
		chargedBulletHolding_p->tEnabled = time;
		if (mouse.leftButton == MOUSE_RELEASED)
		{
			chargedBulletHolding_p->vel = ship.vel + BULLET_SPEED * Normalize(ship.facingV);
			for (int i = 0; i < 2; i++)
			{
				Entity* bullet1_p = &bullets[(bulletIdx++) % MAX_BULLETS];
				bullet1_p->pos = ship.pos;
				bullet1_p->facingV = RotateDeg(ship.facingV, 3 * (i + 1));
				bullet1_p->vel = ship.vel + BULLET_SPEED * Normalize(bullet1_p->facingV);
				bullet1_p->enabled = true;
				bullet1_p->tEnabled = time;

				Entity* bullet2_p = &bullets[(bulletIdx++) % MAX_BULLETS];
				bullet2_p->pos = ship.pos;
				bullet2_p->facingV = RotateDeg(ship.facingV, -3 * (i + 1));
				bullet2_p->vel = ship.vel + BULLET_SPEED * Normalize(bullet2_p->facingV);
				bullet2_p->enabled = true;
				bullet2_p->tEnabled = time;
			}
			chargedBulletHolding_p = nullptr;
		}
	}

	entityCollisions.count = 0;
	if (asteroidsRemaining == 0)
	{
		level = AdvanceLevel(level);
		asteroidsRemaining = level.asteroidsCount;
	}

	ship.pos += deltaT * ship.vel;
	if (ship.enabled && (time > ship.tInvisibility)) AddToCollisions(&entityCollisions, &ship);
	//PushVector(renderer_p, ship.pos + 50.0f * Normalize(asteroid.pos - ship.pos), 20.0f*Normalize(asteroid.pos - ship.pos));

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Entity* bullet_p = &bullets[i];
		if (bullet_p->enabled)
		{
			bullet_p->pos += deltaT * bullet_p->vel;
			AddToCollisions(&entityCollisions, bullet_p);
			if ((time - bullet_p->tEnabled) > BULLET_LIFETIME) bullet_p->enabled = false;
		}
	}

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		Entity* asteroid_p = &asteroids[i];
		if (asteroid_p->enabled && (time > asteroid_p->tInvisibility))
		{
			asteroid_p->facingV = RotateDeg(asteroid_p->facingV, asteroid_p->rotSpeed * deltaT);
			asteroid_p->pos += deltaT * asteroid_p->vel;
			AddToCollisions(&entityCollisions, asteroid_p);
		}
	}

	for (int i = 0; i < MAX_TURRETS; i++)
	{
		Entity* turret_p = &turrets[i];
		if (turret_p->enabled && (time > turret_p->tInvisibility))
		{
			int nextAngle = turret_p->e.nextAngle;
			int prevAngle = turret_p->e.prevAngle;
			float angle = AngleDeg360(VECTOR2_UP, turret_p->facingV);
			
			if (((nextAngle > prevAngle) && (angle >= nextAngle)) || ((nextAngle < prevAngle) && (angle < prevAngle && angle >= nextAngle)))
			{
				turret_p->facingV = RotateDeg(VECTOR2_UP, nextAngle);
				turret_p->rotSpeed = 0.0f;
				turret_p->e.tNextMove = time + 1.0f;
				turret_p->e.prevAngle = turret_p->e.nextAngle;
				turret_p->e.nextAngle = (turret_p->e.nextAngle + 45) % 360;

				Vector2 facingV = turret_p->facingV;
				for (size_t i = 0; i < 4; i++)
				{
					Entity* bullet_p = &enemyBullets[(enemyBulletIdx++) % MAX_BULLETS];
					bullet_p->pos = turret_p->pos;
					bullet_p->vel = BULLET_SPEED * facingV;
					bullet_p->facingV = facingV;
					bullet_p->enabled = true;
					bullet_p->tEnabled = time;

					facingV = RotateDeg(facingV, 90);
				}				
			}

			if (time >= turret_p->e.tNextMove)
			{
				turret_p->rotSpeed = 180.0f;
			}

			turret_p->facingV = RotateDeg(turret_p->facingV, turret_p->rotSpeed * deltaT);

			AddToCollisions(&entityCollisions, turret_p);
		}
	}

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Entity* bullet_p = &enemyBullets[i];
		if (bullet_p->enabled)
		{
			bullet_p->pos += deltaT * bullet_p->vel;
			AddToCollisions(&entityCollisions, bullet_p);
			if ((time - bullet_p->tEnabled) > BULLET_LIFETIME) bullet_p->enabled = false;
		}
	}

	for (int i = 0; i < MAX_PARTICLES_DEBRIS; i++)
	{
		Particle* particle_p = &psDebris.particles_p[i];
		if (particle_p->enabled)
		{
			particle_p->pos += deltaT * particle_p->vel;
			if ((time - particle_p->tEnabled) > PARTICLE_DEBRIS_LIFETIME) particle_p->enabled = false;
		}
	}

	for (int i = 0; i < MAX_PARTICLES_EXHAUST; i++)
	{
		Particle* particle_p = &psExhaust.particles_p[i];
		if (particle_p->enabled)
		{
			particle_p->pos += deltaT * particle_p->vel;
			if ((time - particle_p->tEnabled) > PARTICLE_EXHAUST_LIFETIME) particle_p->enabled = false;
		}
	}

	for (int i = 0; i < MAX_CHARGEDBULLETS; i++)
	{
		Entity* chargedBullet_p = &chargedBullets[i];
		if (chargedBullet_p->enabled)
		{
			chargedBullet_p->pos += deltaT * chargedBullet_p->vel;
			AddToCollisions(&entityCollisions, chargedBullet_p);
			if ((time - chargedBullet_p->tEnabled) > BULLET_LIFETIME) chargedBullet_p->enabled = false;
		}
	}

	if (explosionCharged.enabled)
	{
		explosionCharged.enabled = !AnimationUpdate(&explosionCharged.animation, time);
	}

	if (explosionShip.enabled)
	{
		explosionShip.enabled = !AnimationUpdate(&explosionShip.animation, time);
	}

	for (int i = 0; i < MAX_EXPLOSIONS_SMALL; i++)
	{
		AnimationObject* explosionSmall_p = &explosionsSmall[i];
		if (explosionSmall_p->enabled)
		{
			explosionSmall_p->enabled = !AnimationUpdate(&explosionSmall_p->animation, time);
		}
	}

	if (!ship.enabled && time >= ship.e.tRespawn)
	{
		ship.enabled = true;
		ship.facingV = VECTOR2_UP;
		ship.pos = VECTOR2_ZERO;
		ship.tEnabled = time;
		ship.tInvisibility = time + INVISIBILITY_DURATION;
		ship.health = 100.0f;
	}

	if (ship.health == 0 && ship.enabled)
	{
		explosionShip.enabled = true;
		explosionShip.pos = ship.pos;
		explosionShip.animation.tStart = time;

		ship.enabled = false;
		ship.e.tRespawn = time + SHIP_DEATH_DURATION;
	}

	EntityEntityCollisions(&entityCollisions);
	EntitySolidCollisions(&entityCollisions, deltaT, &solid);

	camera.rect = NewRectCenterPos(ship.pos, camera.rect.size);
	SetSpritesOrtographicProj(renderer_p, camera.rect);
	SetWireframeOrtographicProj(renderer_p, camera.rect);

	PushXCross(renderer_p, mouse.pos, COLOR_YELLOW);

GAMEUPDATE_END:

	for (int i = 0; i < MAX_SOLIDS; i++)
	{
		LineSegment solidLine = solid.solidLines[i];
		PushLine(renderer_p, solidLine.p1, solidLine.p2, COLOR_GREEN);
	}

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Entity* bullet_p = &bullets[i];
		if (bullet_p->enabled) PushSprite(renderer_p, bullet_p->pos, bullet_p->size * VECTOR2_ONE, bullet_p->facingV, bullet_p->textureHandle);
		bullet_p = &enemyBullets[i];
		if (bullet_p->enabled) PushSprite(renderer_p, bullet_p->pos, bullet_p->size * VECTOR2_ONE, bullet_p->facingV, bullet_p->textureHandle, COLOR_RED);
	}

	for (int i = 0; i < MAX_CHARGEDBULLETS; i++)
	{
		Entity* chargedBullet_p = &chargedBullets[i];
		if (chargedBullet_p->enabled)
		{
			PushSprite(renderer_p, chargedBullet_p->pos, chargedBullet_p->size * V2(1.0f, 1.7f), chargedBullet_p->facingV, chargedBullet_p->textureHandle);
			//PushCircle(renderer_p, chargedBullet_p->pos, chargedBullet_p->colliderRadius, COLOR_GREEN);
		}
	}

	for (int i = 0; i < MAX_PARTICLES_DEBRIS; i++)
	{
		Particle* particle_p = &psDebris.particles_p[i];
		if (particle_p->enabled)
		{
			float lifePerc = (time - particle_p->tEnabled) / PARTICLE_DEBRIS_LIFETIME;
			Color color = particle_p->color; color.a = 1.0f - lifePerc;
			PushCircle(renderer_p, particle_p->pos, 1.0f, color, 3);
		}
	}

	for (int i = 0; i < MAX_PARTICLES_EXHAUST; i++)
	{
		Particle* particle_p = &psExhaust.particles_p[i];
		if (particle_p->enabled)
		{
			float lifePerc = (time - particle_p->tEnabled) / PARTICLE_EXHAUST_LIFETIME;
			Color color = particle_p->color; color.a = 1.0f - lifePerc;
			PushCircle(renderer_p, particle_p->pos, 1.0f, color, 3);
		}
	}

	if (ship.enabled)
	{
		if (shipAcceleration > 0.0f)
		{
			Vector2 posExhaust = ship.pos - 0.77f * ship.size * ship.facingV;
			float exhaustYScale = 0.1f * sin(2 * PI * EXHAUST_FREQUENCY * time) + 0.9f;
			Rect uvExhaust = NewRect(V2(0.5306, 0.35), V2(0.1361, 0.65));
			if (shipAcceleration >= SHIP_BOOST) uvExhaust = NewRect(V2(0, 0.35), V2(0.1361, 0.65));
			PushSprite(renderer_p, posExhaust, V2(ship.size / 2, exhaustYScale * ship.size), -ship.facingV, TEXTURE_SHIPEXHAUST, COLOR_WHITE, uvExhaust);

			Color colorParticle = COLOR_EXHAUST;
			if (shipAcceleration >= SHIP_BOOST) colorParticle = COLOR_EXHAUST_BOOST;
			SpawnExhaustParticles(posExhaust, -ship.facingV, colorParticle);
		}

		if (shipAcceleration < 0.0f)
		{
			Vector2 posExhaust1 = ship.pos + (ship.size / 4) * RotateDeg(ship.facingV,  90) + (ship.size / 8) * ship.facingV;
			Vector2 posExhaust2 = ship.pos + (ship.size / 4) * RotateDeg(ship.facingV, -90) + (ship.size / 8) * ship.facingV;
			float exhaustYScale = 0.1f * sin(2 * PI * EXHAUST_FREQUENCY * time) + 0.9f;
			Rect uvExhaust = NewRect(V2(0.3197, 0.35), V2(0.0816, 0.40));
			PushSprite(renderer_p, posExhaust1, V2(ship.size / 4, exhaustYScale * (ship.size/2)), RotateDeg(ship.facingV,  10), TEXTURE_SHIPEXHAUST, COLOR_WHITE, uvExhaust);
			PushSprite(renderer_p, posExhaust2, V2(ship.size / 4, exhaustYScale * (ship.size/2)), RotateDeg(ship.facingV, -10), TEXTURE_SHIPEXHAUST, COLOR_WHITE, uvExhaust);
		}

		Color color = COLOR_WHITE;
		if (time < ship.tInvisibility) color.a = Blink(ship.tEnabled) ? 1.0f : 0.0f;
		if (time < ship.tTakingDamage) color.g = Blink(ship.tEnabled) ? 1.0f : 0.0f;
		PushSprite(renderer_p, ship.pos, ship.size* VECTOR2_ONE, ship.facingV, ship.textureHandle, color);
		//PushCircle(renderer_p, ship.pos, ship.colliderRadius, COLOR_GREEN);
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		Entity* asteroid_p = &asteroids[i];
		if (asteroid_p->enabled)
		{
			Color color = COLOR_WHITE;
			if (time < asteroid_p->tInvisibility) color.a = Blink(ship.tEnabled) ? 1.0f : 0.0f;
			PushSprite(renderer_p, asteroid_p->pos, asteroid_p->size * VECTOR2_ONE, asteroid_p->facingV, asteroid_p->textureHandle, color, asteroid_p->uv);
			//PushCircle(renderer_p, asteroid_p->pos, asteroid_p->colliderRadius, COLOR_GREEN);
		}
	}

	for (int i = 0; i < MAX_TURRETS; i++)
	{
		Entity* turret_p = &turrets[i];
		if (turret_p->enabled)
		{
			Color color = COLOR_WHITE;
			if (time < turret_p->tInvisibility) color.a = Blink(turret_p->tEnabled) ? 1.0f : 0.0f;
			PushSprite(renderer_p, turret_p->pos, turret_p->size * VECTOR2_ONE, turret_p->facingV, turret_p->textureHandle, color, turret_p->uv);
			//PushCircle(renderer_p, turret_p->pos, turret_p->colliderRadius, COLOR_GREEN);
		}
	}

	if (explosionCharged.enabled)
	{
		PushSprite(renderer_p, explosionCharged.pos, 200.0f * VECTOR2_ONE, VECTOR2_UP, explosionCharged.textureHandle, Col(0.537f, 0.902f, 1.0f), AnimationGetCurrentUv(&explosionCharged.animation));
	}

	if (explosionShip.enabled)
	{
		PushSprite(renderer_p, explosionShip.pos, 200.0f * VECTOR2_ONE, VECTOR2_UP, explosionShip.textureHandle, COLOR_WHITE, AnimationGetCurrentUv(&explosionShip.animation));
	}

	for (int i = 0; i < MAX_EXPLOSIONS_SMALL; i++)
	{
		AnimationObject* explosionSmall_p = &explosionsSmall[i];
		if (explosionSmall_p->enabled)
		{
			PushSprite(renderer_p, explosionSmall_p->pos, 50.0f * VECTOR2_ONE, VECTOR2_UP, explosionSmall_p->textureHandle, COLOR_WHITE, AnimationGetCurrentUv(&explosionSmall_p->animation));
		}
	}

	char buf[32] = { 0 }; sprintf(buf, "Score: %d", score);
	UILabel(buf, V2(0.01f, 0.03f), TEXT_ALIGN_LEFT);
	sprintf(buf, "Remaining: %d", asteroidsRemaining);
	UILabel(buf, V2(0.01f, 0.01f), TEXT_ALIGN_LEFT);

	sprintf(buf, "Health: %d", (int)ship.health);
	UILabel(buf, V2(0.79f, 0.02f), TEXT_ALIGN_RIGHT);
	float healthBarWidth = 0.1880f * (ship.health / 100.0f);
	UIRect(NewRect(V2(0.8f, 0.010f), V2(0.19f, 0.02f)), COLOR_WHITE);
	UIRect(NewRect(V2(0.801f, 0.011f), V2(healthBarWidth, 0.018f)), Col(0.19f, 0.49f, 0.25f, 1.0f));

	sprintf(buf, "%.2f", Magnitude(ship.vel));
	UILabel(buf, V2(0.95f, 0.5f), TEXT_ALIGN_RIGHT);

	if (paused) paused = PausedMenu();

	if (GameInput_ButtonDown(BUTTON_ESC))
	{
		paused = !paused;
	}
}

static bool PausedMenu()
{
	bool paused = true;
	if (UIButton("Continue", NewRect(V2(0.35f, 0.6f), V2(0.3f, 0.05f))))
	{
		printf("Continue\n");
		paused = false;
	}
	if (UIButton("Restart", NewRect(V2(0.35f, 0.5f), V2(0.3f, 0.05f))))
	{
		printf("Restart\n");
		paused = false;
		GameStart();
	}
	if (UIButton("Main Menu", NewRect(V2(0.35f, 0.4f), V2(0.3f, 0.05f))))
	{
		printf("Main Menu\n");
		scene = SCENE_MAIN_MENU;
	}
	return paused;
}

static bool MainMenuMain()
{
	bool quitGame = false;
	if (UIButton("Start Game", NewRect(V2(0.35f, 0.6f), V2(0.3f, 0.05f))))
	{
		printf("Start Game\n");
		GameStart();
		scene = SCENE_GAME;
	}
	if (UIButton("Settings", NewRect(V2(0.35f, 0.5f), V2(0.3f, 0.05f))))
	{
		printf("Settings\n");
		mainMenuScreen = MENU_SETTINGS;
	}
	if (UIButton("Quit Game", NewRect(V2(0.35f, 0.4f), V2(0.3f, 0.05f))))
	{
		printf("Quit Game\n");
		quitGame = true;
	}
	return quitGame;
}

static void Settings()
{
	if (UIButton("Go Back", NewRect(V2(0.4f, 0.5f), V2(0.2f, 0.05f))) || GameInput_ButtonDown(BUTTON_ESC))
	{
		printf("Go Back\n");
		mainMenuScreen = MENU_MAIN;
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

