#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef SETUP
#define SETUP

extern float vertices[];

extern GLuint VAO;
extern GLuint VBO;

extern GLuint pixelsDataTex;
extern GLuint pixelsDataFBO;

extern GLuint lowResPassTex;
extern GLuint lowResPassTex2;
extern GLuint lowResPassFBO;

extern GLuint midResPassTex;
extern GLuint midResPassFBO;

void glfwErrorCallback(int errorCode, const char* errorMessage);

void glfwMonitorCallback(GLFWmonitor* monitor, int event);

void createWindow();

bool setupOpenGl();
GLuint loadShader(GLuint program, const char* shaderSource, GLuint shaderType);
bool linkProgram(GLuint program);
void reloadShaders();

#endif
