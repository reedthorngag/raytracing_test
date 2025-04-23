#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern bool keys[512];

extern double rotationY;
extern double rotationX;

void doInputUpdates(double timeSinceLast);

void glfwCharCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

void glfwMousePosCallback(GLFWwindow* window, double x, double y);

