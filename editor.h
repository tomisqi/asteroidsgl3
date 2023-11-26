#pragma once
#include "renderer.h"

void EditorInit();

void Editor(float deltaT, Renderer* renderer_p);

void EditorScrollCallback(double yoffset);
