#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stdio.h>

#include "globals.hpp"
#include "setup.hpp"
#include "chunk.hpp"
#include "input.hpp"
#include "voxel_data/tetrahexa_tree.hpp"
#include "voxel_data/voxel_allocator.hpp"


extern "C" {
    __declspec(dllexport) unsigned int NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

float vertices[] = {
    -1, -1, 0.0,
    -1,  1, 0.0,
     1, -1, 0.0,
     1,  1, 0.0,
};

GLuint VAO;
GLuint VBO;

GLuint pixelsDataTex;
GLuint mouseHitPosTex;

GLuint pixelsDataFBO;
GLuint mouseHitPosFBO;

Chunk* chunk;

const char* debugFrameTypeString[] {
    "Null",
    "ray hit pos", // 1
    "ray dir", // 2
    "ray ratios Y/X, Y/Z X/Y", // 3
    "ray ratios X/Z, Z/X Z/Y", // 4
    "ray deltas", // 5
    "ray origin", // 6
    "camDir", // 7
    "proj_pln_inter", // 8
    "x vec", // 9
    "camOrigin", // 0
};

void dumpPixelData() {
    float buffer[4];

    glReadPixels(mouse.x,mouse.y,1,1,GL_RGBA,GL_FLOAT,&buffer);

    checkGlError("glReadPixels");

    printf("\rdebug frame: %s: %f %f %f %f (mouse pos: %lf, %lf)\n",debugFrameTypeString[sendDebugFrame],buffer[0],buffer[1],buffer[2],buffer[3],mouse.x,mouse.y);
}

void render() {

    if (sendDebugFrame) {
        glBindFramebuffer(GL_FRAMEBUFFER, pixelsDataFBO);
    }

    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform3f(glGetUniformLocation(program, "origin"), cameraPos.x,cameraPos.y,cameraPos.z);
    glUniform3f(glGetUniformLocation(program, "cameraDir"), cameraDir.x,cameraDir.y,cameraDir.z);
    glUniform2f(glGetUniformLocation(program, "mousePos"), mouse.x, mouse.y);
    glUniform1i(glGetUniformLocation(program, "renderPosData"), sendDebugFrame);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    checkGlError("glDrawArrays");

    if (sendDebugFrame) {
        dumpPixelData();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        sendDebugFrame = 0;
    }
}

int main() {
    printf("Hello world!\n");

    createWindow();
    setupOpenGl();

    checkGlError("setupOpenGl");

    glUseProgram(program);

    checkGlError("glUseProgram");

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    checkGlError("glEnableVertexAttribArray");


    glGenTextures(1,&pixelsDataTex);
    glBindTexture(GL_TEXTURE_2D, pixelsDataTex);
  
    checkGlError("glBindTexture");

    // Allocate the storage.
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);

    glGenFramebuffers(1, &pixelsDataFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pixelsDataFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,pixelsDataTex,0);
    
    printf("Framebuffer status: %d\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initTetraHexaTree();
    checkGlError("createSsbo");

    printf("generating chunk...\n");
    double start = glfwGetTime();

    chunk = new Chunk();

    printf("generated chunk! time: %lf  \n",glfwGetTime()-start);

    cameraDir = glm::normalize(cameraDir);

    glfwSetKeyCallback(window,glfwCharCallback);
    glfwSetCursorPosCallback(window,glfwMousePosCallback);
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);

    glfwSwapInterval(0);

    const int averageSize = 40;
    double times[averageSize]{};
    int i = 0;

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        doInputUpdates(glfwGetTime() - start);

        //glfwWaitEvents();
        start = glfwGetTime();

        updateSsboData();
        checkGlError("updateSsboData");

        render();

        times[i++] = glfwGetTime()-start;
        i %= averageSize;
        double out = 0;
        for (int n = 0; n < averageSize && times[n]; n++) out += times[n];
        printf("\rrender time: %dms (%d fps) rotationXY: %lf, %lf camPos: %lf, %lf, %lf    ",(int)(out/(double)averageSize*1000),(int)(1000/(out/(double)averageSize * 1000)),rotationX,rotationY,cameraPos.x,cameraPos.y,cameraPos.z);

        glfwSwapBuffers(window);
        //glFinish();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

