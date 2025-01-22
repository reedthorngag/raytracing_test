#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stdio.h>

#include "globals.hpp"
#include "setup.hpp"
#include "chunk.hpp"

float vertices[] = {
    -1, -1, 0.0,
    -1,  1, 0.0,
     1, -1, 0.0,
     1,  1, 0.0,
};  

Global global;

GLuint VAO;
GLuint VBO;

Chunk* chunk;

void render() {

    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// inline void glfwCharCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//     global.input.glfwCharCallback(window,key,scancode,action,mods);
// }

// inline void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
//     global.input.glfwMouseButtonCallback(window,button,action,mods);
// }

// inline void glfwMousePosCallback(GLFWwindow* window, double x, double y) {
//     global.input.glfwMousePosCallback(window,x,y);
// }

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


    printf("generating chunk...\n");
    double start = glfwGetTime();

    chunk = new Chunk();

    printf("generated chunk! time: %lf  \n",glfwGetTime()-start);


    // glfwSetKeyCallback(global.window,glfwCharCallback);
    // glfwSetCursorPosCallback(global.window,glfwMousePosCallback);
    // glfwSetMouseButtonCallback(global.window, glfwMouseButtonCallback);

    glfwSwapInterval(1);

    double times[40]{};
    int i = 0;

    while (!glfwWindowShouldClose(global.window)) {
        glfwPollEvents();

        start = glfwGetTime();
        render();
        times[i++] = glfwGetTime()-start;
        i %= 40;
        double out = 0;
        for (int n = 0; n < 40 && times[n]; n++) out += times[n];
        printf("\rrender time: %dms    ",(int)(out/40.0*100000));

        glfwSwapBuffers(global.window);
    }

    glfwDestroyWindow(global.window);
    glfwTerminate();
    return 0;
}

