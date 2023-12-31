static bool LineLineIntersect(LineSegment line1, LineSegment line2, Vector2& p)
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

	bool result = (t >= 0 && t <= 1) && (u >= 0 && u <= 1);
	if (result) p = V2(x1 + t * (x2 - x1), y1 + t * (y2 - y1));

	return result;
}

static bool LineCircleIntersect(LineSegment line, Vector2 circlePos, float circleRadius, Vector2& p)
{
	p = V2(U16_MAX, U16_MAX);

	float radiusSq = circleRadius * circleRadius;
	if ((MagnitudeSq(line.p1 - circlePos) <= radiusSq)) p = line.p1;
	if ((MagnitudeSq(line.p2 - circlePos) <= radiusSq)) p = line.p2;

	Vector2 normal = RotateDeg(Normalize(line.p1 - line.p2), 90);
	LineSegment lineCircle = { circlePos - circleRadius * normal, circlePos + circleRadius * normal };
	LineLineIntersect(line, lineCircle, p);

	return p.x != U16_MAX || p.y != U16_MAX;
}