
#include "setup.hpp"

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

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);//glfwGetPrimaryMonitor();
    GLFWmonitor* monitor = monitors[0];
    DEBUG(1) printf("Number of connected monitors: %d\n",count);

    int x, y;
    glfwGetMonitorPos(monitor, &x, &y);
    DEBUG(1) printf("monitor pos: %d, %d\n",x,y);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_DOUBLEBUFFER, true);
    glfwWindowHint(GLFW_MAXIMIZED, true);
    glfwWindowHint(GLFW_DECORATED, false);
    glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
    //glfwWindowHint(GLFW_FLOATING, true);

    window = glfwCreateWindow(mode->width, mode->height, "Voxel engine v2", monitor, NULL);
    if (!window) {
        printf("Window creation failed!\n");
        exit(1);
    }

    //glfwShowWindow(window);
    //glfwSetWindowSize(window, mode->width,mode->height-50);
    ///glfwSetWindowPos(window,x,y);

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        printf("GLEW init failed!\n");
        exit(1);
    }

    DEBUG(1) printf("OpenGL version: %s\n",glGetString(GL_VERSION));

    glfwGetFramebufferSize(window,&width,&height);
    glViewport(0,0,width,height);

    DEBUG(1) printf("Frame buffer size: %d, %d\n",width, height);

    halfWidth = (double)width/2.0;
    halfHeight = (double)height/2.0;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

GLuint loadShader(GLuint program, const char* shaderSource, GLuint shaderType)  {
    GLuint shader = loadShader(shaderSource, shaderType);
    if (!shader) return 0;

    glAttachShader(program,shader);

    glDeleteShader(shader); // mark for deletion, only actually deleted after glDetachShader

    return shader;
}

bool linkProgram(GLuint program) {
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
        glDeleteProgram(program);

        return false;
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
        glDeleteProgram(program);

        return false;;
    }

    return true;
}

void reloadShaders() {
    GLuint oldProgram = program;

    if (setupOpenGl()) {
        glDeleteProgram(program);
        printf("Shaders successfully reloaded!    \n");
    } else
        program = oldProgram;
}

bool setupOpenGl() {

    program = glCreateProgram();
    
    GLuint shader1 = loadShader(program, "../src/shaders/frag_shader.glsl", GL_FRAGMENT_SHADER);
    if (!shader1) return false;
    
    GLuint shader2 = loadShader(program, "../src/shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
    if (!shader2) return false;

    if (!linkProgram(program))
        return false;

    glDetachShader(program, shader1);
    glDetachShader(program, shader2);

    glEnable(GL_DEPTH_TEST);

    glUseProgram(program);

    return true;
}

