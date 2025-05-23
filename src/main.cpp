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

extern void enableReportGlErrors();

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

    checkGlError(fullResProgram,"glReadPixels");

    printf("\rdebug frame: %s: %f %f %f %f (mouse pos: %lf, %lf)\n",debugFrameTypeString[sendDebugFrame],buffer[0],buffer[1],buffer[2],buffer[3],mouse.x,mouse.y);
}

u32 getMortonPos(glm::vec3 pos) {
    glm::ivec3 p = glm::ivec3(glm::floor(pos));

    int n = p.x;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;
    u32 x = n;

    n = p.y;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;
    u32 y = n;

    n = p.z;
    n = (n | (n << 16)) & 0x030000FF;
    n = (n | (n <<  8)) & 0x0300F00F;
    n = (n | (n <<  4)) & 0x030C30C3;

    return (n << 4) | (y << 2) | x;
}

void render() {
    if (secondaryRaysFBO == 0 || normalTex == 0) {
        printf("FUCK\n");
        exit(1);
    }

    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(lowResProgram);

    glBindFramebuffer(GL_FRAMEBUFFER, secondaryRaysFBO);

    glUniform3f(glGetUniformLocation(lowResProgram, "origin"), cameraPos.x,cameraPos.y,cameraPos.z);
    glUniform3f(glGetUniformLocation(lowResProgram, "cameraDir"), cameraDir.x,cameraDir.y,cameraDir.z);
    glUniform2f(glGetUniformLocation(lowResProgram, "mousePos"), mouse.x, mouse.y);
    glUniform1ui(glGetUniformLocation(lowResProgram, "originMortonPos"), getMortonPos(cameraPos));
    glUniform1i(glGetUniformLocation(lowResProgram, "renderPosData"), sendDebugFrame);
    if (dimensionsChanged) {
        glUniform2ui(glGetUniformLocation(lowResProgram, "resolution"), width, height);
        glUniform2f(glGetUniformLocation(lowResProgram, "projPlaneSize"), glm::tan(glm::radians(45.0)), glm::tan(glm::radians(45.0)) * ((float)height)/width);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    DEBUG(3) checkGlError(lowResProgram,"glDrawArrays1");

    
    glUseProgram(lightScatteringProgram);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (dimensionsChanged) {
        glUniform2ui(glGetUniformLocation(lightScatteringProgram, "resolution"), width, height);
        dimensionsChanged = false;
    }

    glUniform3f(glGetUniformLocation(lightScatteringProgram, "origin"), cameraPos.x,cameraPos.y,cameraPos.z);


    glBindTexture(GL_TEXTURE_2D, colorBufferTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, posTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glActiveTexture(GL_TEXTURE0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    DEBUG(3) checkGlError(lightScatteringProgram,"glDrawArrays2");

    // glUseProgram(midResProgram);

    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);//midResPassFBO);

    // glUniform3f(glGetUniformLocation(midResProgram, "origin"), cameraPos.x,cameraPos.y,cameraPos.z);
    // glUniform3f(glGetUniformLocation(midResProgram, "cameraDir"), cameraDir.x,cameraDir.y,cameraDir.z);
    // glUniform2f(glGetUniformLocation(midResProgram, "mousePos"), mouse.x, mouse.y);

    // glBindTexture(GL_TEXTURE_2D, lowResPassTex);
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, lowResPassTex2);
    // glActiveTexture(GL_TEXTURE0);
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // return;
    // glUseProgram(fullResProgram);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // if (sendDebugFrame) {
    //     glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pixelsDataFBO);
    // }

    // glUniform3f(glGetUniformLocation(fullResProgram, "origin"), cameraPos.x,cameraPos.y,cameraPos.z);
    // glUniform3f(glGetUniformLocation(fullResProgram, "cameraDir"), cameraDir.x,cameraDir.y,cameraDir.z);
    // glUniform2f(glGetUniformLocation(fullResProgram, "mousePos"), mouse.x, mouse.y);
    // glUniform1i(glGetUniformLocation(fullResProgram, "renderPosData"), sendDebugFrame);

    // glBindTexture(GL_TEXTURE_2D, midResPassTex);
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    // DEBUG(3) checkGlError(lowResProgram,"glDrawArrays3");

    // if (sendDebugFrame) {  
    //     dumpPixelData();
    //     glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //     sendDebugFrame = 0;
    // }
}

int main() {
    printf("Hello world!\n");

    createWindow();

    if (!setupOpenGl())
        exit(1);

    DEBUG(1) enableReportGlErrors();

    glBindVertexArray(VAO);
    initTetraHexaTree();
    initVoxelDataAllocator();
    checkGlError(0, "createSsbo");

    printf("generating chunk...\n");
    double start = glfwGetTime();

    chunk = new Chunk();

    printf("generated chunk! time: %lf  \n",glfwGetTime()-start);

    cameraDir = glm::normalize(cameraDir);

    glfwSetKeyCallback(window,glfwCharCallback);
    glfwSetCursorPosCallback(window,glfwMousePosCallback);
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
    glfwSetScrollCallback(window, glfwScrollCallback);

    glfwSwapInterval(1);

    const int averageSize = 40;
    double times[averageSize]{};
    int i = 0;

    while (!glfwWindowShouldClose(window)) {

        //glfwWaitEvents();
        start = glfwGetTime();

        updateSsboData();

        render();

        glfwSwapBuffers(window);

        glfwPollEvents();
        doInputUpdates(glfwGetTime() - start);

        times[i++] = glfwGetTime()-start;
        i %= averageSize;
        double out = 0;
        for (int n = 0; n < averageSize && times[n]; n++) out += times[n];
        printf("\rrender time: %dms (%d fps) rotationXY: %lf, %lf camPos: %lf, %lf, %lf    ",(int)(out/(double)averageSize*1000),(int)(1000.0/(double)((out/(double)averageSize) * 1000)),rotationY,rotationX,cameraPos.x,cameraPos.y,cameraPos.z);

        //glFinish();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

