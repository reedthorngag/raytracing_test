#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stdio.h>
#include <bit>

#include "globals.hpp"
#include "setup.hpp"
#include "raytracer.hpp"

#define encodeColor(r, g, b) ((uint32_t)(r & (uint8_t)~0) << 16 | (uint16_t)(g & (uint8_t)~0) << 8 | (uint8_t)b)

#define decodeR(c) ((c >> 16) & (uint8_t)~0)
#define decodeG(c) ((c >> 8) & (uint8_t)~0)
#define decodeB(c) (c & (uint8_t)~0)

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
    Node{0, 0b00000001, 1},
    Node{1,0,encodeColor(255,0,0)}
};

const int treeDepth = 8;

struct Cast {
    int steps;
    uint32_t color;
};

Cast cast(Pos* pos, Ray* ray) {

    struct Item {
        Pos pos;
        int offset;
        uint8_t index;
    };

    Item stack[16]{};
    stack[15] = Item{*pos,0,0};

    int steps = 0;
    int offset = 0;
    int depth = treeDepth-2;
    while (!nodes[offset].flags) {
        uint8_t index = 1 << ((pos->x >> depth) & 1) << (pos->y >> depth << 1) << (pos->z >> depth << 2);

        //printf("here: %d b: %#x i: %d o: %d\n",nodes[offset].bitmap & index, nodes[offset].bitmap, index, offset);
        if (nodes[offset].bitmap & index) {
            // step in
            stack[depth] = Item{*pos,offset,index};
            offset = nodes[offset].offset + std::popcount((uint32_t)(nodes[offset].bitmap & (index-1)));
            depth--;
            continue;
        } else {
            nextIntersect(pos,*ray,1<<depth);
            depth++;
            uint8_t index2 = 1 << ((pos->x >> depth) & 1) << (pos->y >> depth << 1) << (pos->z >> depth << 2);
            if (stack[depth].index != index2) {
                if (depth == 7) return Cast(steps,255);
                // step out
                continue;
            }
            depth--;
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
            Cast hit = cast(&pos,&ray);
            //printf("pos: %d,%d,%d color: %#x decoded color: %d,%d,%d\n",pos.x,pos.y,pos.z,hit.color,decodeR(hit.color),decodeG(hit.color),decodeB(hit.color));
            points[x*100+y].color = glm::vec3(decodeR(hit.color)/255,decodeG(hit.color)/255,decodeB(hit.color)/255);
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
    loadShaders();

    glUseProgram(global.program2);

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

