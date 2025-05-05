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

    checkGlError(program3,"glReadPixels");

    printf("\rdebug frame: %s: %f %f %f %f (mouse pos: %lf, %lf)\n",debugFrameTypeString[sendDebugFrame],buffer[0],buffer[1],buffer[2],buffer[3],mouse.x,mouse.y);
    printf("\rnode data: %u %llu %u\n",((Node*)nodeBlocks[0].ptr)->branch.flags,((Node*)nodeBlocks[0].ptr)->branch.bitmap,((Node*)nodeBlocks[0].ptr)->branch.children);
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

    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program1);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);//lowResPassFBO);

    glUniform3f(glGetUniformLocation(program1, "origin"), cameraPos.x,cameraPos.y,cameraPos.z);
    glUniform3f(glGetUniformLocation(program1, "cameraDir"), cameraDir.x,cameraDir.y,cameraDir.z);
    glUniform2f(glGetUniformLocation(program1, "mousePos"), mouse.x, mouse.y);
    glUniform1ui(glGetUniformLocation(program1, "originMortonPos"), getMortonPos(cameraPos));

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    return;
    glUseProgram(program2);

    glBindFramebuffer(GL_FRAMEBUFFER, midResPassFBO);

    glUniform3f(glGetUniformLocation(program2, "origin"), cameraPos.x,cameraPos.y,cameraPos.z);
    glUniform3f(glGetUniformLocation(program2, "cameraDir"), cameraDir.x,cameraDir.y,cameraDir.z);
    glUniform2f(glGetUniformLocation(program2, "mousePos"), mouse.x, mouse.y);

    glBindTexture(GL_TEXTURE_2D, lowResPassTex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    glUseProgram(program3);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (sendDebugFrame) {
        glBindFramebuffer(GL_FRAMEBUFFER, pixelsDataFBO);
    }

    glUniform3f(glGetUniformLocation(program3, "origin"), cameraPos.x,cameraPos.y,cameraPos.z);
    glUniform3f(glGetUniformLocation(program3, "cameraDir"), cameraDir.x,cameraDir.y,cameraDir.z);
    glUniform2f(glGetUniformLocation(program3, "mousePos"), mouse.x, mouse.y);
    glUniform1i(glGetUniformLocation(program3, "renderPosData"), sendDebugFrame);

    glBindTexture(GL_TEXTURE_2D, midResPassTex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (sendDebugFrame) {  
        dumpPixelData();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        sendDebugFrame = 0;
    }
}

int main() {
    printf("Hello world!\n");

    createWindow();

    if (!setupOpenGl())
        exit(1);


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

    glfwSwapInterval(1);

    const int averageSize = 40;
    double times[averageSize]{};
    int i = 0;

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        doInputUpdates(glfwGetTime() - start);

        //glfwWaitEvents();
        start = glfwGetTime();

        updateSsboData();

        render();

        times[i++] = glfwGetTime()-start;
        i %= averageSize;
        double out = 0;
        for (int n = 0; n < averageSize && times[n]; n++) out += times[n];
        printf("\rrender time: %dms (%d fps) rotationXY: %lf, %lf camPos: %lf, %lf, %lf    ",(int)(out/(double)averageSize*1000),(int)(1000/(out/(double)averageSize * 1000)),rotationY,rotationX,cameraPos.x,cameraPos.y,cameraPos.z);

        glfwSwapBuffers(window);
        //glFinish();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

