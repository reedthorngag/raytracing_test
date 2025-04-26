#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#ifndef _GLOBALS
#define _GLOBALS

#define DEBUG_LEVEL 3

#define DEBUG(x) if (x <= DEBUG_LEVEL) 

extern int width;
extern int height;

extern double halfWidth;
extern double halfHeight;

extern GLFWwindow* window;

extern GLuint program1;
extern GLuint program2;
extern GLuint program3;

inline void checkGlErrorFunc(GLuint program, const char* id) {
    int error = glGetError();
    if (error != GL_NO_ERROR) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        char* infoLog = new char[maxLength];
        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);
        printf("GL Error: %s: %d: %s\n",id, error, infoLog);
        exit(1);
    }
}

// so it is easy to disable/effectively remove later
#define checkGlError(program, id) checkGlErrorFunc(program, id)



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



