#include <glm/glm.hpp>
#include "globals.hpp"

int width;
int height;

double halfWidth;
double halfHeight;

bool dimensionsChanged = true;

Mouse mouse;

double lastMouseUpdate = 0;

bool mouseLocked = false;

unsigned int sendDebugFrame = 0;

glm::vec3 cameraDir(1,0,1);
glm::vec3 cameraPos(35,50,35);

GLFWwindow* window;

GLuint program1;
GLuint program2;
GLuint program3;

GLuint arraySsbo;
GLuint nodeSsbo;

//shit code warning, its "temporary" though (yea right)
Block hotbar[] {
    Block{
        NONE,
        RGB_TO_U64(255,0,0)
    },
    Block{
        NONE,
        RGB_TO_U64(0,255,0)
    },
    Block{
        REFLECTIVE,
        RGB_TO_U64(0,0,0)
    },
};

int hotbarLength = 3;
int currentSelected;
