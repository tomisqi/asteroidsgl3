#include <stdlib.h>
#include "input.h"
#include "test.h"
#include "vector.h"
#include "intersect.h"
#include "ui.h"
#include "utils.h"

#define ROTATION_SPEED 360 // Degrees per second.
#define SPEED          500.0f

struct Player
{
	Vector2 pos;
	Vector2 facingV;
	Vector2 vel;
	float size;
};

extern Vector2 ScreenDim;

//static LineSegment line1 = { V2(-100, 0), V2(-300, 0) };
static LineSegment line2 = { V2( 0, 100), V2( 0, -100) };
static Player player = {V2(-100, 0), VECTOR2_UP, VECTOR2_ZERO, 50};
static Player player2 = { VECTOR2_ZERO, VECTOR2_UP, VECTOR2_ZERO, 5 };
static Vector2 collisionP = VECTOR2_ZERO;

static void MoveLine(LineSegment* line_p, Vector2 v, float deltaT)
{
	line_p->p1 += SPEED * deltaT * v;
	line_p->p2 += SPEED * deltaT * v;
}

static void RotateLine(LineSegment* line_p, float angleDeg)
{
	Vector2 v = Normalize(line_p->p1 - line_p->p2);
	float len = Magnitude(line_p->p1 - line_p->p2);
	Vector2 centerP = GetCenterP(*line_p);
	v = RotateDeg(v, angleDeg);
	line_p->p1 = centerP + len / 2 * v;
	line_p->p2 = centerP - len / 2 * v;
}

static void MakeHorizontal(LineSegment* line_p)
{
	Vector2 v = Normalize(line_p->p1 - line_p->p2);
	float len = Magnitude(line_p->p1 - line_p->p2);
	Vector2 centerP = line_p->p2 + len / 2 * v;
	line_p->p1.x = centerP.x + len / 2;
	line_p->p1.y = centerP.y;
	line_p->p2.x = centerP.x - len / 2;
	line_p->p2.y = centerP.y;
}

static void CheckCollision(Player* player1_p, Player* player2_p)
{
	float distCollision = player1_p->size/2 + player2_p->size/2;
	float distCollisionSq = distCollision * distCollision;
	float distSq = MagnitudeSq(player1_p->pos - player2_p->pos);
	if (distSq <= distCollisionSq)
	{
		Vector2 v1 = player1_p->vel;
		Vector2 v2 = player2_p->vel;
		float m1 = player1_p->size;
		float m2 = player2_p->size;
		Vector2 x1 = player1_p->pos;
		Vector2 x2 = player2_p->pos;

		player1_p->vel = v1 - (2*m2/(m1+m2)) * (Dot(v1 - v2, x1 - x2) / MagnitudeSq(x1-x2)) * (x1 - x2);
		player2_p->vel = v2 - (2*m1/(m1+m2)) * (Dot(v2 - v1, x2 - x1) / MagnitudeSq(x2-x1)) * (x2 - x1);
		//player1_p->vel = (player2_p->size / player1_p->size) * player2_p->vel;
		//player2_p->vel = (player1_p->size / player2_p->size) * v1;
		collisionP = player1_p->pos + (player1_p->size / 2) * Normalize(player2_p->pos - player1_p->pos);
	}
}

static Vector2 MouseToWorldPos(Vector2 mousepos)
{
	float x = (mousepos.x - ScreenDim.x/2);
	float y = (mousepos.y - ScreenDim.y/2);
	return V2(x, y);
}

Vector2 holePos = V2(GetRandomValue(-350, 350), GetRandomValue(-350, 350));
bool Test(Renderer* renderer_p, float deltaT)
{
	Mouse mouse = GameInput_GetMouse();
	mouse.pos = MouseToWorldPos(mouse.pos);
	
	if (GameInput_Button(BUTTON_ENTER))
	{
		player2.pos = VECTOR2_ZERO;
		player2.vel = VECTOR2_ZERO;
	}

	static Vector2 aimP = VECTOR2_ZERO;
	if (mouse.state == MOUSE_PRESSED_HOLD && (aimP == VECTOR2_ZERO))
	{
		aimP = mouse.pos;		
	}
	if (mouse.state == MOUSE_RELEASED && aimP != VECTOR2_ZERO)
	{
		player.pos = aimP;
		player.vel = 200.0f * Normalize(mouse.pos - aimP);
		player.vel = 2 * (mouse.pos - aimP);

		aimP = VECTOR2_ZERO;
	}

	if (aimP != VECTOR2_ZERO)
	{
		PushVector(renderer_p, aimP, mouse.pos - aimP);
	}


	player.pos += deltaT * player.vel;
	player2.pos += deltaT * player2.vel;

	// Players
	//PushCircle(renderer_p, player.pos, player.size/2, COLOR_GREEN, 64);
	//PushCircle(renderer_p, player2.pos, player2.size / 2, COLOR_YELLOW, 64);

	// Hole
	//PushCircle(renderer_p, holePos, player2.size / 2, COLOR_GRAY, 8);

	// CollisionP
	//PushXCross(renderer_p, collisionP, COLOR_CYAN);

	CheckCollision(&player, &player2);

	if (UIButton("1234567890987654321", NewRect(V2(0.25f, 0.5f), V2(0.5f, 0.05f)), TEXT_ALIGN_CENTER))
	{
		printf("Click!\n");
	}

	//static char bufx[32] = { 0 }; static char bufy[32] = { 0 };
	//UITextInput(NewRect(V2(-380, 350), V2(200, 25)), bufx);
	//UITextInput(NewRect(V2(-380, 310), V2(200, 25)), bufy);
	//float x = atof(bufx);
	//float y = atof(bufy);

	//UITextInput(NewRect(V2(0.1f, 0.1f), V2(0.1f, 0.1f)), buf);

	//PushText01(renderer_p, "Hello World!", V2(x, y), COLOR_WHITE);
	//PushText(renderer_p, "Hello World2!", mouse.pos, COLOR_WHITE);
	//PushText(renderer_p, buf, V2(-380, 380), COLOR_WHITE);

	//static char buf[64] = { 0 };
	//UITextInput(NewRect(V2(-380,350), V2(200, 25)), buf);
	//UITextInput(NewRect(V2(0, 0.1f), V2(0.1f, 0.1f)), buf);
	//static char buf2[64] = { 0 };
	//UITextInput(NewRect(V2(-380, 300), V2(200, 25)), buf2);

	SetWireframeOrtographicProj(renderer_p, NewRectCenterPos(VECTOR2_ZERO, ScreenDim));

	bool quit = false;
	if (GameInput_Button(BUTTON_ESC)) quit = true;

	return quit;
}