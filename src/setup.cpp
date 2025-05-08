
#include "setup.hpp"

#include "shaders/load_shader.hpp"
#include "globals.hpp"

float vertices[] = {
    -1, -1, 0.0,
    -1,  1, 0.0,
     1, -1, 0.0,
     1,  1, 0.0,
};

GLuint VAO;
GLuint VBO;

GLuint pixelsDataTex;

GLuint pixelsDataFBO;
GLuint mouseHitPosFBO;

GLuint lowResPassTex;
GLuint lowResPassTex2;
GLuint lowResPassFBO;

GLuint midResPassTex;
GLuint midResPassFBO;

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


bool setupProgram1() {
    program1 = glCreateProgram();
    
    GLuint shader1 = loadShader(program1, "../src/shaders/low_res.frag", GL_FRAGMENT_SHADER);
    if (!shader1) return false;
    
    GLuint shader2 = loadShader(program1, "../src/shaders/shader.vert", GL_VERTEX_SHADER);
    if (!shader2) return false;

    if (!linkProgram(program1))
        return false;

    glDetachShader(program1, shader1);
    glDetachShader(program1, shader2);

    return true;
}

bool setupProgram2() {
    program2 = glCreateProgram();
    
    GLuint shader1 = loadShader(program2, "../src/shaders/mid_res.frag", GL_FRAGMENT_SHADER);
    if (!shader1) return false;
    
    GLuint shader2 = loadShader(program2, "../src/shaders/shader.vert", GL_VERTEX_SHADER);
    if (!shader2) return false;

    if (!linkProgram(program2))
        return false;

    glDetachShader(program2, shader1);
    glDetachShader(program2, shader2);

    return true;
    
}

bool setupProgram3() {

    program3 = glCreateProgram();
    
    GLuint shader1 = loadShader(program3, "../src/shaders/full_res.frag", GL_FRAGMENT_SHADER);
    if (!shader1) return false;
    
    GLuint shader2 = loadShader(program3, "../src/shaders/shader.vert", GL_VERTEX_SHADER);
    if (!shader2) return false;

    if (!linkProgram(program3))
        return false;

    glDetachShader(program3, shader1);
    glDetachShader(program3, shader2);

    return true;
}

bool createDependencies() {

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTextures(1,&lowResPassTex);
    glBindTexture(GL_TEXTURE_2D, lowResPassTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width>>2, height>>2);

    glGenTextures(1,&lowResPassTex2);
    glBindTexture(GL_TEXTURE_2D, lowResPassTex2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width>>2, height>>2);

    glGenFramebuffers(1, &lowResPassFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lowResPassFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,lowResPassTex,0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,lowResPassTex2,0);

    GLenum drawBuffers[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);
    

    glGenTextures(1,&midResPassTex);
    glBindTexture(GL_TEXTURE_2D, midResPassTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width>>1, height>>1);

    glGenFramebuffers(1, &midResPassFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, midResPassFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,midResPassTex,0);


    glGenTextures(1,&pixelsDataTex);
    glBindTexture(GL_TEXTURE_2D, pixelsDataTex);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);

    glGenFramebuffers(1, &pixelsDataFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pixelsDataFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,pixelsDataTex,0);

    return true;
}

bool setupOpenGl() {

    glDisable(GL_DEPTH_TEST);

    if (!setupProgram1()) return false;
    if (!setupProgram2()) return false;
    if (!setupProgram3()) return false;

    if (!createDependencies()) return false;

    return true;
}

void reloadShaders() {
    glUseProgram(0);
    GLuint oldProgram1 = program1;
    GLuint oldProgram2 = program2;
    GLuint oldProgram3 = program3;

    if (setupProgram1() && setupProgram2() && setupProgram3()) {
        glDeleteProgram(oldProgram1);
        glDeleteProgram(oldProgram2);
        glDeleteProgram(oldProgram3);
        printf("Shaders successfully reloaded!    \n");
    } else {
        program1 = oldProgram1;
        program2 = oldProgram2;
        program3 = oldProgram3;
    }

    glUseProgram(program3);
}

