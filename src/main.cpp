#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stdio.h>

#include "globals.hpp"
#include "setup.hpp"
#include "chunk.hpp"
#include "input.hpp"

float vertices[] = {
    -1, -1, 0.0,
    -1,  1, 0.0,
     1, -1, 0.0,
     1,  1, 0.0,
};  

Global global;

GLuint VAO;
GLuint VBO;

GLuint pixelsDataTex;
GLuint mouseHitPosTex;

GLuint pixelsDataFBO;
GLuint mouseHitPosFBO;

Chunk* chunk;

void dumpPixelData() {
    float buffer[4];

    glReadPixels(mouse.x,mouse.y,1,1,GL_RGBA,GL_FLOAT,&buffer);

    printf("\rpixel data: %f %f %f %f (mouse pos: %lf, %lf)\n",buffer[0],buffer[1],buffer[2],buffer[3],mouse.x,mouse.y);
}

void render() {

    if (getPixelData) {
        glBindFramebuffer(GL_FRAMEBUFFER, pixelsDataFBO);
    }

    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform3f(glGetUniformLocation(global.program, "origin"), cameraPos.x,cameraPos.y,cameraPos.z);
    glUniform3f(glGetUniformLocation(global.program, "cameraDir"), cameraDir.x,cameraDir.y,cameraDir.z);
    glUniform2f(glGetUniformLocation(global.program,"mousePos"), mouse.x, mouse.y);
    glUniform1i(glGetUniformLocation(global.program,"renderPosData"), getPixelData);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (getPixelData) {
        dumpPixelData();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        getPixelData = false;
    }
}

int main() {
    printf("Hello world!\n");

    createWindow();
    setupOpenGl();

    glUseProgram(global.program);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



    glGenTextures(1,&pixelsDataTex);
    glBindTexture(GL_TEXTURE_2D, pixelsDataTex);
    // Allocate the storage.
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, WIDTH, HEIGHT);

    glGenFramebuffers(1, &pixelsDataFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pixelsDataFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,pixelsDataTex,0);
    
    printf("Framebuffer status: %d\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    printf("generating chunk...\n");
    double start = glfwGetTime();

    chunk = new Chunk();

    printf("generated chunk! time: %lf  \n",glfwGetTime()-start);


    glfwSetKeyCallback(global.window,glfwCharCallback);
    glfwSetCursorPosCallback(global.window,glfwMousePosCallback);
    glfwSetMouseButtonCallback(global.window, glfwMouseButtonCallback);

    glfwSwapInterval(0);

    double times[5]{};
    int i = 0;

    while (!glfwWindowShouldClose(global.window)) {

        start = glfwGetTime();

        glfwWaitEvents();
        render();

        times[i++] = glfwGetTime()-start;
        i %= 5;
        double out = 0;
        for (int n = 0; n < 5 && times[n]; n++) out += times[n];
        //printf("\rrender time: %dms    ",(int)(out/5.0*100000));

        glfwSwapBuffers(global.window);
        glFinish();
    }

    glfwDestroyWindow(global.window);
    glfwTerminate();
    return 0;
}

