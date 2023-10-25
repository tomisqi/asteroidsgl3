// TODOs:
// [x] Coordinate system
// [x] Text rendering
// [x] Mouse support
// [x] UI
// [ ] Audio
// [x] Collisions
// [x] Animations
// [x] Particle system
// [x] Camera

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include "vector.h"
#include "color.h"
#include "input.h"
#include "utils.h"
#include "shaders.h"
#include "opengl.h"
#include "texture.h"
#include "asteroids.h"
#include "renderer.h"
#include "ui.h"
#include "test.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Vector2 ScreenDim = V2(900, 900);

static void GlfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// Make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

static void BindButtons()
{
	GameInput_BindButton(BUTTON_X, GLFW_KEY_X);
	GameInput_BindButton(BUTTON_Q, GLFW_KEY_Q);
	GameInput_BindButton(BUTTON_C, GLFW_KEY_C);
	GameInput_BindButton(BUTTON_S, GLFW_KEY_S);
	GameInput_BindButton(BUTTON_A, GLFW_KEY_A);
	GameInput_BindButton(BUTTON_D, GLFW_KEY_D);
	GameInput_BindButton(BUTTON_W, GLFW_KEY_W);
	GameInput_BindButton(BUTTON_UP_ARROW, GLFW_KEY_UP);
	GameInput_BindButton(BUTTON_LEFT_ARROW, GLFW_KEY_LEFT);
	GameInput_BindButton(BUTTON_RIGHT_ARROW, GLFW_KEY_RIGHT);
	GameInput_BindButton(BUTTON_DOWN_ARROW, GLFW_KEY_DOWN);
	GameInput_BindButton(BUTTON_LSHIFT, GLFW_KEY_LEFT_SHIFT);
	GameInput_BindButton(BUTTON_ENTER, GLFW_KEY_ENTER);
	GameInput_BindButton(BUTTON_ESC, GLFW_KEY_ESCAPE);
	GameInput_BindButton(BUTTON_BACKSPACE, GLFW_KEY_BACKSPACE);
	GameInput_BindButton(BUTTON_HOME, GLFW_KEY_HOME);
	GameInput_BindButton(BUTTON_DEL, GLFW_KEY_DELETE);
	GameInput_BindButton(BUTTON_END, GLFW_KEY_END);
	GameInput_BindButton(BUTTON_LCTRL, GLFW_KEY_LEFT_CONTROL);
}

static float GetDeltaT()
{
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
	int refreshRate = videoMode->refreshRate;

	return 1.0f / refreshRate;
}

static double r2()
{
	return (double)rand() / (double)RAND_MAX;
}

void CharCallback(GLFWwindow* window, unsigned int codepoint)
{
	UICharCallback(codepoint);
}

int main(void)
{
	glfwSetErrorCallback(GlfwErrorCallback);
	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(ScreenDim.x, ScreenDim.y, "AsteroidsGL3", NULL, NULL);
	if (window == NULL) return -1;

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(1); // Enable vsync

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	srand(time(NULL)); // Initialize random seed

	Texture textures[TEXTURES_COUNT] = { 0 };
	textures[TEXTURE_SPACECRAFT] = LoadTexture("../assets/textures/Spacecraft.png");
	textures[TEXTURE_REDSHOT] = LoadTexture("../assets/textures/RedShot.png");
	textures[TEXTURE_ELI] = LoadTexture("../assets/textures/ELI.png");
	textures[TEXTURE_ASTEROID] = LoadTexture("../assets/textures/Asteroids.png");
	textures[TEXTURE_CHARGEDBULLET] = LoadTexture("../assets/textures/BlueShot.png");
	textures[TEXTURE_SHIPEXHAUST] = LoadTexture("../assets/textures/Exhaust.png");
	textures[TEXTURE_EXPLOSIONBIG] = LoadTexture("../assets/textures/ExplosionBig.png");
	textures[TEXTURE_EXPLOSION5] = LoadTexture("../assets/textures/Explosion5.png");
	textures[TEXTURE_EXPLOSIONSMALL] = LoadTexture("../assets/textures/ExplosionTiny.png");
	textures[TEXTURE_TURRET] = LoadTexture("../assets/textures/turret.png");

	OpenGL openGl;
	OpenGLInit(&openGl);

	Renderer renderer;
	RendererInit(&renderer);

	GameInput_Init();
	BindButtons();
	ButtonState buttonStates[MAX_BUTTONS];
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
	glfwSetCharCallback(window, CharCallback);

	UIInit(&renderer);
	GameInit();
	while (!glfwWindowShouldClose(window))
	{
	  for (int i = 0; i < MAX_BUTTONS; i++)
	  {
	    buttonStates[i] = (ButtonState) glfwGetKey(window, GameInput_GetBinding(i));
	  }

	  double mouseXpos, mouseYpos;
	  glfwGetCursorPos(window, &mouseXpos, &mouseYpos);
	  GameInput_NewFrame(buttonStates, glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS, V2(mouseXpos, mouseYpos), ScreenDim, GetDeltaT());

	  UINewFrame(GetDeltaT(), ScreenDim);

	  bool quit = GameUpdateAndRender(GetDeltaT(), &renderer);
	  //bool quit = Test(&renderer, GetDeltaT());

	  OpenGLEndFrame(&openGl, &renderer, textures, ScreenDim);
	  RendererEndFrame(&renderer);
	  glfwSwapBuffers(window);
	  glfwPollEvents();

	  if (quit) break;
	}

	//stbi_image_free(data); //@nocommit

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
