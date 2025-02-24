#include <glm/glm.hpp>
#include "globals.hpp"

Mouse mouse;

double lastMouseUpdate = 0;

bool mouseLocked = false;

unsigned int sendDebugFrame = 0;

glm::vec3 cameraDir(1,0,1);
glm::vec3 cameraPos(35,50,35);

GLFWwindow* window;
GLuint program;
