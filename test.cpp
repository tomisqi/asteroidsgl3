#include <stdlib.h>
#include "input.h"
#include "test.h"
#include "vector.h"
#include "intersect.h"
#include "ui.h"

#define ROTATION_SPEED 360 // Degrees per second.
#define SPEED          500.0f

struct Player
{
	Vector2 pos;
	Vector2 facingV;
	float radius;
};

extern Vector2 ScreenDim;

//static LineSegment line1 = { V2(-100, 0), V2(-300, 0) };
static LineSegment line2 = { V2( 0, 100), V2( 0, -100) };
static Player player = {V2(-100, 0), VECTOR2_UP, 10};

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

bool Test(Renderer* renderer_p, float deltaT)
{
	// Line1
	Vector2 p;
	if (GameInput_Button(BUTTON_UP_ARROW) && !LineCircleIntersect(line2, player.pos, player.radius, p))
	{
		player.pos += SPEED * deltaT * player.facingV;
	}
	if (GameInput_Button(BUTTON_DOWN_ARROW))
	{
		player.pos += -SPEED * deltaT * player.facingV;
	}
#if 0
	if (GameInput_Button(BUTTON_LSHIFT) && GameInput_ButtonDown(BUTTON_RIGHT_ARROW))
	{
		float angle = AngleDeg(VECTOR2_RIGHT, line1.p1 - line1.p2); 
		if (line1.p1.y < line1.p2.y) angle = 360-angle;
		angle--; if (angle < 0) angle = 359;
		float angle45 = 45 * ((int)angle/ 45); // Get the next 45 deg angle.
		MakeHorizontal(&line1);
		RotateLine(&line1, angle45);

	}
#endif
	if (GameInput_Button(BUTTON_RIGHT_ARROW) && !GameInput_Button(BUTTON_LSHIFT))
	{
		player.facingV = RotateDeg(player.facingV, -ROTATION_SPEED * deltaT);
	}
#if 0
	if (GameInput_Button(BUTTON_LSHIFT) && GameInput_ButtonDown(BUTTON_LEFT_ARROW))
	{
		float angle = AngleDeg(VECTOR2_RIGHT, line1.p1 - line1.p2);
		if (line1.p1.y < line1.p2.y) angle = 360 - angle;
		angle++;
		float angle45 = 45 * ((int)angle / 45) + 45; // Get the next 45 deg angle.
		MakeHorizontal(&line1);
		RotateLine(&line1, angle45);

	}
#endif
	if (GameInput_Button(BUTTON_LEFT_ARROW) && !GameInput_Button(BUTTON_LSHIFT))
	{
		player.facingV = RotateDeg(player.facingV, ROTATION_SPEED * deltaT);
	}

	// Line2
	if (GameInput_Button(BUTTON_W))
	{
		Vector2 facingV = Normalize(line2.p1 - line2.p2);
		MoveLine(&line2, facingV, deltaT);
	}
	if (GameInput_Button(BUTTON_S))
	{
		Vector2 facingV = Normalize(line2.p1 - line2.p2);
		MoveLine(&line2, -facingV, deltaT);
	}
	if (GameInput_Button(BUTTON_D))
	{
		RotateLine(&line2, -ROTATION_SPEED * deltaT);
	}
	if (GameInput_Button(BUTTON_A))
	{
		RotateLine(&line2, ROTATION_SPEED * deltaT);
	}

	// Player
	PushCircle(renderer_p, player.pos, player.radius, COLOR_GREEN, 64);
	PushLine(renderer_p, player.pos, player.pos + player.radius*player.facingV, COLOR_CYAN);

	PushLine(renderer_p, line2.p1, line2.p2, COLOR_WHITE);
	PushText(renderer_p, "p1", V2(line2.p1.x, -line2.p1.y), COLOR_CYAN);

	PushCircle(renderer_p, VECTOR2_ZERO, 2.0f, COLOR_WHITE); // origin

	PushVector(renderer_p, GetCenterP(line2), 50.0f*GetNormal(line2, player.pos), COLOR_MAGENTA);

	PushVector(renderer_p, VECTOR2_ZERO, 50.0f * VECTOR2_UP, COLOR_MAGENTA);

	//printf("%.02f\n", AngleDegRel(VECTOR2_UP, line2.p1 - line2.p2));

	//
	float angle = 0;
	//angle += (6 * deltaT); if (angle >= 360) angle = 0;
	static char buf[64] = { 0 };

	UITextInput(NewRect(V2(-380,350), V2(200, 25)), buf);
	angle = atoi(buf);

	static char buf2[64] = { 0 };
	UITextInput(NewRect(V2(-380, 300), V2(200, 25)), buf2);
#if 0
	PushVector(renderer_p, V2(-200, 200), 100.0f * VECTOR2_RIGHT, COLOR_MAGENTA);
	PushVector(renderer_p, V2(-200, 200), 100.0f * RotateDeg(VECTOR2_RIGHT, angle));

	PushVector(renderer_p, V2(200, 200), 100.0f * VECTOR2_LEFT, COLOR_MAGENTA);
	PushVector(renderer_p, V2(200, 200), 100.0f * RotateDeg(VECTOR2_LEFT, angle));

	PushVector(renderer_p, V2(-200, -200), 100.0f * VECTOR2_UP, COLOR_MAGENTA);
	PushVector(renderer_p, V2(-200, -200), 100.0f * RotateDeg(VECTOR2_UP, angle));

	PushVector(renderer_p, V2(200, -200), 100.0f * VECTOR2_DOWN, COLOR_MAGENTA);
	PushVector(renderer_p, V2(200, -200), 100.0f * RotateDeg(VECTOR2_DOWN, angle));
#endif

	SetWireframeOrtographicProj(renderer_p, NewRectCenterPos(VECTOR2_ZERO, ScreenDim));

	bool quit = false;
	if (GameInput_Button(BUTTON_ESC))
	{
		quit = true;
	}
	return quit;
}