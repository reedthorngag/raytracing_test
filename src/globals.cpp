#include <glm/glm.hpp>
#include "globals.hpp"

Mouse mouse;

double lastMouseUpdate = 0;

bool getPixelData = false;

glm::vec3 cameraDir(0,0,1);
glm::vec3 cameraPos(50,50,0);
