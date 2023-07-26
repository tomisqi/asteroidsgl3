#include "input.h"
#include "test.h"
#include "vector.h"

#define ROTATION_SPEED 360 // Degrees per second.
#define SPEED          300.0f

struct LineSegment
{
	Vector2 p1;
	Vector2 p2;
};

static LineSegment line1 = { V2(-100, 0), V2(-300, 0) };
static LineSegment line2 = { V2( 100, 100), V2( 100, -100) };
static Vector2 player;

static void MoveLine(LineSegment* line_p, Vector2 v, float deltaT)
{
	line_p->p1 += SPEED * deltaT * v;
	line_p->p2 += SPEED * deltaT * v;
}

static void RotateLine(LineSegment* line_p, float angleDeg)
{
	Vector2 v = Normalize(line_p->p1 - line_p->p2);
	float len = Magnitude(line_p->p1 - line_p->p2);
	Vector2 centerP = line_p->p2 + len / 2 * v;
	v = RotateDeg(v, angleDeg);
	line_p->p1 = centerP + len / 2 * v;
	line_p->p2 = centerP - len / 2 * v;
}

static bool LinesIntersect(LineSegment line1, LineSegment line2, Vector2& p)
{
	float x1 = line1.p1.x;
	float y1 = line1.p1.y;
	float x2 = line1.p2.x;
	float y2 = line1.p2.y;

	float x3 = line2.p1.x;
	float y3 = line2.p1.y;
	float x4 = line2.p2.x;
	float y4 = line2.p2.y;

	float tNum = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4));
	float tDen = ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
	float uNum = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2));
	float uDen = ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));

	float t = -1.0f;
	if (tDen) t = tNum / tDen;
	float u = -1.0f;
	if (uDen) u = uNum / uDen;

	p = V2(x1 + t*(x2-x1), y1+t*(y2-y1));

	return (t >= 0 && t <= 1) && (u >= 0 && u <= 1);
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
	if (GameInput_Button(BUTTON_UP_ARROW))
	{
		Vector2 facingV = Normalize(line1.p1 - line1.p2);
		MoveLine(&line1, facingV, deltaT);
	}
	if (GameInput_Button(BUTTON_DOWN_ARROW))
	{
		Vector2 facingV = Normalize(line1.p1 - line1.p2);
		MoveLine(&line1, -facingV, deltaT);
	}
	if (GameInput_Button(BUTTON_LSHIFT) && GameInput_ButtonDown(BUTTON_RIGHT_ARROW))
	{
		float angle = AngleDeg(VECTOR2_RIGHT, line1.p1 - line1.p2); 
		if (line1.p1.y < line1.p2.y) angle = 360-angle;
		angle--; if (angle < 0) angle = 359;
		float angle45 = 45 * ((int)angle/ 45); // Get the next 45 deg angle.
		MakeHorizontal(&line1);
		RotateLine(&line1, angle45);

	}
	if (GameInput_Button(BUTTON_RIGHT_ARROW) && !GameInput_Button(BUTTON_LSHIFT))
	{
		RotateLine(&line1, -ROTATION_SPEED * deltaT);
	}
	if (GameInput_Button(BUTTON_LSHIFT) && GameInput_ButtonDown(BUTTON_LEFT_ARROW))
	{
		float angle = AngleDeg(VECTOR2_RIGHT, line1.p1 - line1.p2);
		if (line1.p1.y < line1.p2.y) angle = 360 - angle;
		angle++;
		float angle45 = 45 * ((int)angle / 45) + 45; // Get the next 45 deg angle.
		MakeHorizontal(&line1);
		RotateLine(&line1, angle45);

	}
	if (GameInput_Button(BUTTON_LEFT_ARROW) && !GameInput_Button(BUTTON_LSHIFT))
	{
		RotateLine(&line1, ROTATION_SPEED * deltaT);
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

	PushLine(renderer_p, line1.p1, line1.p2, V3(1, 1, 1));
	PushLine(renderer_p, line2.p1, line2.p2, V3(1, 1, 1));
	PushText(renderer_p, "p1", V2(line1.p1.x, -line1.p1.y), V3(0, 1, 1));
	PushText(renderer_p, "p1", V2(line2.p1.x, -line2.p1.y), V3(0, 1, 1));

	Vector2 intersectionPoint = V2(U16_MAX, U16_MAX);
	if (LinesIntersect(line1, line2, intersectionPoint))
	{
		PushCircle(renderer_p, intersectionPoint, 5.0f, V3(1, 0, 1));
	}

	PushCircle(renderer_p, VECTOR2_ZERO, 2.0f, V3(1, 1, 1)); // origin

	//printf("%.02f ", AngleDeg(line1.p1 - line1.p2, line2.p1 - line2.p2));


	bool quit = false;
	if (GameInput_Button(BUTTON_ESC))
	{
		quit = true;
	}
	return quit;
}