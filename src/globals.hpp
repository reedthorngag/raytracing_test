#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef _GLOBALS
#define _GLOBALS


const int WIDTH = 800;
const int HEIGHT = 600;

const double HALF_WIDTH = WIDTH/2.0;
const double HALF_HEIGHT = HEIGHT/2.0;

extern GLFWwindow* window;
extern GLuint program;

const float moveSpeed = 100000;

struct Mouse {
    double x = 0;
    double y = 0;
};

extern Mouse mouse;

extern double lastMouseUpdate;

extern bool mouseLocked;

extern unsigned int sendDebugFrame;

extern glm::vec3 cameraDir;
extern glm::vec3 cameraPos;

#endif



