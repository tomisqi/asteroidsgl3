#pragma once
#include "vector.h"
#include "renderer.h"

void GameInit();
bool GameUpdateAndRender(float deltaT, Vector2 screenDim, Renderer* renderer_p);