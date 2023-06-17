// TODOs:
// [ ] Coordinate system
// [ ] Text rendering
// [ ] Mouse support
// [ ] Audio

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_truetype.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <time.h>
#include "vector.h"
#include "color.h"
#include "input.h"
#include "utils.h"
#include "text.h"
#include "shaders.h"
#include "opengl.h"
#include "texture.h"
#include "asteroids.h"

#define WINDOW_SIZE	 800
Vector2 screenDim = V2(WINDOW_SIZE, WINDOW_SIZE);

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
	GameInput_BindButton(BUTTON_UP_ARROW, GLFW_KEY_UP);
	GameInput_BindButton(BUTTON_LEFT_ARROW, GLFW_KEY_LEFT);
	GameInput_BindButton(BUTTON_RIGHT_ARROW, GLFW_KEY_RIGHT);
	GameInput_BindButton(BUTTON_DOWN_ARROW, GLFW_KEY_DOWN);
	GameInput_BindButton(BUTTON_LSHIFT, GLFW_KEY_LEFT_SHIFT);
	GameInput_BindButton(BUTTON_ENTER, GLFW_KEY_ENTER);
	GameInput_BindButton(BUTTON_ESC, GLFW_KEY_ESCAPE);
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

int main(void)
{
	glfwSetErrorCallback(GlfwErrorCallback);
	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "AsteroidsGL3", NULL, NULL);
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

	GLuint shaderProgram = LoadAndCompileShaders("../shaders/shader.vs", "../shaders/shader.fs");

	Texture textures[5] = { 0 };
	textures[0] = LoadTexture("../assets/textures/triangle-41361.png");	
	textures[1] = LoadTexture("../assets/textures/path4147.png");
	textures[2] = LoadTexture("../assets/textures/ELI.png");
	textures[3] = LoadTexture("../assets/textures/InternalTileDev.png");
	textures[4] = LoadTexture("../assets/textures/path4146.png");

	OpenGL openGl;
	OpenGLInit(&openGl);

	Renderer renderer;
	RendererInit(&renderer);

	GameInput_Init();
	BindButtons();
	ButtonState buttonStates[MAX_BUTTONS];

	while (!glfwWindowShouldClose(window))
	{
	  for (int i = 0; i < MAX_BUTTONS; i++)
	  {
	    buttonStates[i] = (ButtonState) glfwGetKey(window, GameInput_GetBinding(i));
	  }
	  GameInput_NewFrame(buttonStates);

	  GameUpdateAndRender(GetDeltaT(), screenDim, &renderer);

	  OpenGLEndFrame(&openGl, &renderer.renderCommands, textures, screenDim, shaderProgram);
	  RendererEndFrame(&renderer);
	  glfwSwapBuffers(window);
	  glfwPollEvents();

	  if (GameInput_ButtonDown(BUTTON_ESC)) break; // @nocommit
	}

	//stbi_image_free(data); //@nocommit

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
