#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "shaders/load_shader.hpp"
#include "globals.hpp"

extern Global global;

void glfwErrorCallback(int errorCode, const char* errorMessage) {
    printf("glfw error: %d %s\n", errorCode, errorMessage);
}

void createWindow() {
    if (!glfwInit()) {
        printf("GLFW init failed!\n");
        exit(1);
    }

    glfwSetErrorCallback(glfwErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    global.window = glfwCreateWindow(WIDTH, HEIGHT, "Voxel engine v2", NULL, NULL);
    if (!global.window) {
        printf("Window creation failed!\n");
        exit(1);
    }

    glfwMakeContextCurrent(global.window);

    if (glewInit() != GLEW_OK) {
        printf("GLEW init failed!\n");
        exit(1);
    }

    int width,height;
    glfwGetFramebufferSize(global.window,&width,&height);
    glViewport(0,0,width,height);

    glfwSetInputMode(global.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void loadShaders() {
    GLuint fragShader = loadShader("../src/shaders/frag_shader.glsl",GL_FRAGMENT_SHADER);
    GLuint vertexShader = loadShader("../src/shaders/vertex_shader.glsl",GL_VERTEX_SHADER);

    global.program = glCreateProgram();

    glAttachShader(global.program,fragShader);
    glAttachShader(global.program,vertexShader);
    glLinkProgram(global.program);

    GLint isLinked = 0;
    glGetProgramiv(global.program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(global.program, GL_INFO_LOG_LENGTH, &maxLength);

        char* infoLog = new char[maxLength];
        glGetProgramInfoLog(global.program, maxLength, &maxLength, infoLog);
        printf("Linking failed! Info log: %s\n",infoLog);

        delete[] infoLog;
        glDeleteShader(fragShader);
        glDeleteShader(vertexShader);
        glDeleteProgram(global.program);

        exit(1);
    }

    glValidateProgram(global.program);

    GLint isValid = 0;
    glGetProgramiv(global.program, GL_VALIDATE_STATUS, (int *)&isValid);
    if (isValid == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(global.program, GL_INFO_LOG_LENGTH, &maxLength);

        char* infoLog = new char[maxLength];
        glGetProgramInfoLog(global.program, maxLength, &maxLength, infoLog);
        printf("Linking failed! Info log: %s\n",infoLog);

        delete[] infoLog;
        glDeleteShader(fragShader);
        glDeleteShader(vertexShader);
        glDeleteProgram(global.program);

        exit(1);
    }

    glDeleteShader(fragShader);
    glDeleteShader(vertexShader);

    glEnable(GL_DEPTH_TEST);
}

