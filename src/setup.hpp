#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "shaders/load_shader.hpp"
#include "globals.hpp"

void glfwErrorCallback(int errorCode, const char* errorMessage) {
    printf("glfw error: %d %s\n", errorCode, errorMessage);
}

void glfwMonitorCallback(GLFWmonitor* monitor, int event)
{
    if (event == GLFW_CONNECTED)
    {
        printf("Monitor connected!\n");
    }
    else if (event == GLFW_DISCONNECTED)
    {
        printf("Monitor disconnected!\n");
    }
}

void createWindow() {
    if (!glfwInit()) {
        printf("GLFW init failed!\n");
        exit(1);
    }

    glfwSetErrorCallback(glfwErrorCallback);

    glfwSetMonitorCallback(glfwMonitorCallback);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_MAXIMIZED, true);

    window = glfwCreateWindow(mode->width, mode->height, "Voxel engine v2", monitor, NULL);
    if (!window) {
        printf("Window creation failed!\n");
        exit(1);
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        printf("GLEW init failed!\n");
        exit(1);
    }

    glfwGetFramebufferSize(window,&width,&height);
    glViewport(0,0,width,height);

    printf("width: %d, height: %d\n",width, height);

    halfWidth = (double)width/2.0;
    halfHeight = (double)height/2.0;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void setupOpenGl() {
    GLuint fragShader = loadShader("../src/shaders/frag_shader.glsl",GL_FRAGMENT_SHADER);
    GLuint vertexShader = loadShader("../src/shaders/vertex_shader.glsl",GL_VERTEX_SHADER);

    program = glCreateProgram();

    glAttachShader(program,fragShader);
    glAttachShader(program,vertexShader);
    glLinkProgram(program);

    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        char* infoLog = new char[maxLength];
        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);
        printf("Linking failed! Info log: %s\n",infoLog);

        delete[] infoLog;
        glDeleteShader(fragShader);
        glDeleteShader(vertexShader);
        glDeleteProgram(program);

        exit(1);
    }

    glValidateProgram(program);

    GLint isValid = 0;
    glGetProgramiv(program, GL_VALIDATE_STATUS, (int *)&isValid);
    if (isValid == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        char* infoLog = new char[maxLength];
        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);
        printf("Linking failed! Info log: %s\n",infoLog);

        delete[] infoLog;
        glDeleteShader(fragShader);
        glDeleteShader(vertexShader);
        glDeleteProgram(program);

        exit(1);
    }

    glDeleteShader(fragShader);
    glDeleteShader(vertexShader);

    glEnable(GL_DEPTH_TEST);
}

