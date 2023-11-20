#include <string.h>
#include "vector.h"
#include "renderer.h"
#include "texture.h"
#include "input.h"
#include "ui.h"
#include "rect.h"

#define COLOR_GRID Col(0.24f, 0.24f, 0.24f)
#define GRID_SIZE 100
#define CAM_ZOOM_STEP 100.0f
#define CAM_INITIAL_SIZE (2.0f * ScreenDim)
#define MAX_ENTITIES 2

struct Entity
{
	Vector2 pos;
	Vector2 facingV;
	float size;
	TextureHandleT textureHandle;
	Rect uv;
};

struct Camera
{
	Rect rect;
};

static Camera camera;
static Entity ship;
static Entity asteroid;
static Entity* entitiesArr_p[MAX_ENTITIES] = { &ship, &asteroid };
static int selected = -1;

static Vector2 MouseToWorldPos(Vector2 mousepos)
{
	float x = (mousepos.x / ScreenDim.x) * camera.rect.size.x + camera.rect.pos.x;
	float y = (mousepos.y / ScreenDim.y) * camera.rect.size.y + camera.rect.pos.y;
	return V2(x, y);
}

static inline int AlignToGrid(float coord, int gridSize)
{
	return gridSize * ((int)coord / gridSize);
}

static float GetZoomLevel(Camera* camera_p, float zoomStep)
{
	if (CAM_INITIAL_SIZE.x == camera_p->rect.size.x) return 1.0f;
	return 2.0f * ((CAM_INITIAL_SIZE.x - camera_p->rect.size.x) / zoomStep);
}

static void DrawGrid(Renderer* renderer_p, Camera* camera_p, int gridSize)
{
	float yStart = AlignToGrid(camera_p->rect.pos.y, gridSize) - gridSize;
	float yEnd   = AlignToGrid(camera_p->rect.pos.y + camera_p->rect.size.y, gridSize) + gridSize;
	float xStart = AlignToGrid(camera_p->rect.pos.x, gridSize) - gridSize;
	float xEnd   = AlignToGrid(camera_p->rect.pos.x + camera_p->rect.size.x, gridSize) + gridSize;

	float y = yStart;
	while (y < yEnd)
	{
		PushLine(renderer_p, V2(xStart, y), V2(xEnd, y), COLOR_GRID, y == 0 ? 2.0f : 0.001f);
		y += gridSize;
	}
	float x = xStart;
	while (x < xEnd)
	{
		PushLine(renderer_p, V2(x, yStart), V2(x, yEnd), COLOR_GRID, x == 0 ? 2.0f : 0.001f);
		x += gridSize;
	}
}

static void CameraPan(Mouse* mouse_p)
{
	static Vector2 mouseStartPos;
	static Vector2 camStartPos;
	if (mouse_p->rightButton == MOUSE_PRESSED)
	{
		mouseStartPos = mouse_p->pos;
		camStartPos = camera.rect.pos;
	}
	if (mouse_p->rightButton == MOUSE_PRESSED_HOLD)
	{
		camera.rect.pos = camStartPos - (mouse_p->pos - mouseStartPos);
	}
}

static int SelectSingle(Vector2 mousePos)
{
	int index = -1;
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		Entity* entity_p = entitiesArr_p[i];
		float distSq = MagnitudeSq(mousePos - entity_p->pos);
		float halfSize = entity_p->size / 2;
		if (distSq <= (halfSize * halfSize))
		{
			index = i;
			break;
		}
	}

	return index;
}

void EditorScrollCallback(double yoffset)
{
	Vector2 camCenter = GetRectCenter(camera.rect);
	Vector2 camSize = camera.rect.size - yoffset * 2 * CAM_ZOOM_STEP * VECTOR2_ONE;
	camera.rect = NewRectCenterPos(camCenter, camSize);
}

void EditorInit()
{
	memset(&camera, 0, sizeof(camera));
	camera.rect = NewRectCenterPos(VECTOR2_ZERO, CAM_INITIAL_SIZE);

	memset(&ship, 0, sizeof(ship));
	ship.pos = VECTOR2_ZERO;
	ship.facingV = VECTOR2_UP;
	ship.size = 85.0f;
	ship.textureHandle = TEXTURE_SPACECRAFT;
	ship.uv = RECT_ONE;

	memset(&asteroid, 0, sizeof(asteroid));
	asteroid.pos = 500.0f * VECTOR2_ONE;
	asteroid.facingV = VECTOR2_UP;
	asteroid.size = 150.0f;
	asteroid.textureHandle = TEXTURE_ASTEROID;
	asteroid.uv = NewRect(VECTOR2_ZERO, V2(0.25f, 0.25f));

	selected = -1;
}

void Editor(Renderer* renderer_p)
{
	Mouse mouse = GameInput_GetMouse();
	CameraPan(&mouse);

	if (mouse.leftButton == MOUSE_PRESSED)
	{
		selected = SelectSingle(MouseToWorldPos(mouse.pos));
	}

	DrawGrid(renderer_p, &camera, GRID_SIZE);

	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		Entity* entity_p = entitiesArr_p[i];
		PushSprite(renderer_p, entity_p->pos, entity_p->size * VECTOR2_ONE, entity_p->facingV, entity_p->textureHandle, COLOR_WHITE, entity_p->uv);
	}

	if (selected >= 0)
	{
		Entity* entity_p = entitiesArr_p[selected];
		PushRect(renderer_p, NewRectCenterPos(entity_p->pos, entity_p->size * VECTOR2_ONE), COLOR_GREEN, VECTOR2_UP);
	}

	if (UIButton("Reset", NewRect(V2(0.94f, 0.97f), V2(0.05f, 0.02f))))
	{
		camera.rect = NewRectCenterPos(VECTOR2_ZERO, CAM_INITIAL_SIZE);
	}
	if (UIButton("+", NewRect(V2(0.94f, 0.945f), V2(0.02f, 0.02f))))
	{
		Vector2 camCenter = GetRectCenter(camera.rect);
		Vector2 camSize = camera.rect.size - 5 * CAM_ZOOM_STEP * VECTOR2_ONE;
		camera.rect = NewRectCenterPos(camCenter, camSize);
	}
	if (UIButton("-", NewRect(V2(0.97f, 0.945f), V2(0.02f, 0.02f))))
	{
		Vector2 camCenter = GetRectCenter(camera.rect);
		Vector2 camSize = camera.rect.size + 5 * CAM_ZOOM_STEP * VECTOR2_ONE;
		camera.rect = NewRectCenterPos(camCenter, camSize);
	}

	SetSpritesOrtographicProj(renderer_p, camera.rect);
	SetWireframeOrtographicProj(renderer_p, camera.rect);
}