#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef _GLOBALS
#define _GLOBALS


const int WIDTH = 800;
const int HEIGHT = 600;

const double halfWidth = WIDTH/2.0;
const double halfWidth = HEIGHT/2.0;

struct Global {
    GLFWwindow* window;
    GLuint program;
};

const glm::vec3 startPos(0);

const float moveSpeed = 0.05;
const float speed_scale = 5;

#endif



