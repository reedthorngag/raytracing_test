#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stdio.h>
#include <bit>

#include "globals.hpp"
#include "setup.hpp"
#include "raytracer.hpp"

#define encodeColor(r, g, b) ((uint32_t)(r & ~(uint8_t)0) << 16 | (uint16_t)(g & ~(uint8_t)0) << 8 | (uint8_t)b)

#define decodeR(c) (c >> 16 & ~(uint8_t)0)
#define decodeG(c) (c >> 8 & ~(uint8_t)0)
#define decodeB(c) (c & ~(uint8_t)0)

Global global;

GLuint VAO;
GLuint VBO;

#pragma pack(1)
struct Point {
    glm::vec2 pos;
    glm::vec3 color;
};

Point* points;

Node nodes[9]{
    Node{0, 0b10
            0b00
            0b00
            0b00,1},
    Node{1,0,encodeColor(0,255,0)}
};

const int treeDepth = 16;

struct Cast {
    int steps;
    uint32_t color;
};

Cast cast(Pos pos, Ray ray);

void stepIn(Pos* pos, Ray* ray, int* currdepth, int curreOffset) {

}

void stepOut(Pos* pos, Ray* ray, int* currdepth, int curreOffset) {

}

Cast cast(Pos* pos, Ray* ray) {

    uint32_t stack[16]{};
    int sp = 15;

    int steps = 0;
    int offset = 0;
    int depth = treeDepth;
    while (!nodes[offset].flags) {
        uint8_t localpos[3] = {(pos->x >> depth) & 1, (pos->y >> depth) & 1, (pos->z >> depth) & 1};
        int index = ((1 << localpos[0]) << (localpos[1]<<1) << (localpos[2]<<2));

        if (nodes[offset].bitmap & index) {
            stepIn(pos,ray,&depth,offset);
            offset = nodes[offset].offset + std::popcount((uint32_t)(nodes[offset].bitmap & (index-1)));
            stack[sp--] = offset;
        } else {
            nextIntersect(&pos,ray,depth<<1);

        }

    }

    return Cast{steps,nodes[offset].offset};
}

void render() {

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            points[x*100+y].pos = glm::vec2(x/100.0-0.5,y/100.0-.5);
            Ray ray = Ray(0,1,0);
            Pos pos = Pos(x,0,y);
            Cast hit = cast(pos,ray);
            points[x*100+y].color = glm::vec3(decodeR(hit.color)/255,decodeG(hit.color),decodeB(hit.color));
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

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> range(0,1);


    uint64_t bitmap;
    for (int i = 0; i < 64; i++)
        if (range(rng))
            bitmap |= (1 << i);

    Node* children = new Node[64];
    children[2*16+2*4+2].bitmap = bitmap;

    // for (int x = 0; x < 4; x++) {
    //     for (int y = 0; y < 4; y++) {
    //         for (int z = 0; z < 4; z++) {
    //             int offset = x*16+y*4+z;
    //             children[offset].parent = root;
    //             children[offset].pos = root->pos + Pos(x,y,z);
    //             root->children->push_back(&children[offset]);
    //             root->bitmap |= 1 << offset;
    //         }
    //     }
    // }


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

