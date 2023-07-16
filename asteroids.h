#pragma once
#include "vector.h"
#include "renderer.h"

void GameInit();
bool GameUpdateAndRender(float deltaT, Renderer* renderer_p);