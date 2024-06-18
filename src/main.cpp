#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stdio.h>

#include "globals.hpp"
#include "setup.hpp"
#include "raytracer.hpp"

Global global;

GLuint VAO;
GLuint VBO;

#pragma pack(1)
struct Point {
    glm::vec2 pos;
    glm::vec3 color;
};

Point* points;

void render() {

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            points[x*100+y].pos = glm::vec2(x/150.0-0.5,y/150.0);
            points[x*100+y].color = glm::vec3(1.0,0,0);
        }
    }

    glBindVertexArray(VAO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Point)*100*100, points);

    glDrawArrays(GL_POINTS, 0, 100 * 100);
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

    points = new Point[100*100]{};

    

    createWindow();
    setupOpenGl();

    glUseProgram(global.program);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1,&VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Point)*100*100, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0,2,GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(sizeof(GL_FLOAT)*2));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


    printf("generating chunk...\n");
    double start = glfwGetTime();

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

