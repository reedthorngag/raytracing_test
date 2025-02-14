#include <glm/glm.hpp>
#include "globals.hpp"

Mouse mouse;

double lastMouseUpdate = 0;

bool mouseLocked = false;

unsigned int sendDebugFrame = 0;

glm::vec3 cameraDir(0,0,1);
glm::vec3 cameraPos(50,50,0);

GLFWwindow* window;
GLuint program;
