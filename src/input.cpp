#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "input.hpp"
#include "globals.hpp"
#include "voxel_data/tetrahexa_tree.hpp"
#include "ray_caster.hpp"
#include "setup.hpp"

bool keys[512] = { 0 };

double rotationY = 0;
double rotationX = 0;

void rotateCamera(double xDelta, double yDelta) {
    if (rotationY+yDelta <= -90 || rotationY+yDelta >= 90) yDelta = 0;
    rotationY += yDelta;
    rotationX += xDelta;
    if (rotationX > 180) rotationX -= 360;
    if (rotationX < -180) rotationX += 360;
    cameraDir = glm::rotateX(glm::vec3(0,0,1),(float)glm::radians(rotationY));
    cameraDir = glm::normalize(cameraDir);
    cameraDir = glm::rotateY(cameraDir,(float)glm::radians(rotationX));
    cameraDir = glm::normalize(cameraDir);
}

void doInputUpdates(double timeSinceLast) {
    int xDelta = 0;
    int yDelta = 0;

    int xMove = 0;
    int yMove = 0;
    int zMove = 0;

    if (keys[GLFW_KEY_LEFT]) xDelta -= 25;
    if (keys[GLFW_KEY_RIGHT]) xDelta += 25;
    if (keys[GLFW_KEY_DOWN]) yDelta += 25;
    if (keys[GLFW_KEY_UP]) yDelta -= 25;
    if (keys[GLFW_KEY_W]) zMove += 2;
    if (keys[GLFW_KEY_A]) xMove -= 2;
    if (keys[GLFW_KEY_S]) zMove -= 2;
    if (keys[GLFW_KEY_D]) xMove += 2;
    if (keys[GLFW_KEY_LEFT_CONTROL]) yMove -= 2;
    if (keys[GLFW_KEY_SPACE]) yMove += 2;

    if (xDelta || yDelta) rotateCamera(xDelta*timeSinceLast*0.5, yDelta*timeSinceLast*0.5);

    double ms = timeSinceLast / 100000.0;

    glm::vec3 speed = glm::vec3(moveSpeed) * glm::vec3(ms);
    if (keys[GLFW_KEY_LEFT_SHIFT]) speed *= 4;

    glm::vec3 cameraLeft = glm::cross(cameraDir,glm::vec3(0,1,0));

    if (zMove) cameraPos += cameraDir * glm::vec3(zMove) * speed;

    if (xMove) cameraPos += cameraLeft * glm::vec3(-xMove) * speed;

    if (yMove) cameraPos += glm::cross(cameraDir,cameraLeft) * glm::vec3(-yMove) * speed;

}

void glfwCharCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action) {
        keys[key] = true;
    } else {
        keys[key] = false;
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window,true);
                break;

            case GLFW_KEY_TAB:
                cameraPos = glm::vec3(50,50,0);
                break;

            case GLFW_KEY_1:
            currentSelected = glm::min(0,hotbarLength-1);
                break;
            case GLFW_KEY_2:
            currentSelected = glm::min(1,hotbarLength-1);
                break;
            case GLFW_KEY_3:
                currentSelected = glm::min(2,hotbarLength-1);
                break;
            case GLFW_KEY_4:
                currentSelected = glm::min(3,hotbarLength-1);
                break;
            case GLFW_KEY_5:
                currentSelected = glm::min(4,hotbarLength-1);
                break;
            case GLFW_KEY_6:
                currentSelected = glm::min(5,hotbarLength-1);
                break;
            case GLFW_KEY_7:
                currentSelected = glm::min(6,hotbarLength-1);
                break;
            case GLFW_KEY_8:
                currentSelected = glm::min(7,hotbarLength-1);
                break;
            case GLFW_KEY_9:
                currentSelected = glm::min(8,hotbarLength-1);
                break;
            case GLFW_KEY_0:
                currentSelected = glm::min(9,hotbarLength-1);
                break;

            case GLFW_KEY_L:
                glfwSetCursorPos(window,halfWidth,halfHeight);
                mouseLocked = !mouseLocked;
                break;

            case GLFW_KEY_R:
                printf("\r");
                reloadShaders();
                break;

            case GLFW_KEY_N:
                printf("\n");
                break;
            
            default:
                // released
                printf("\rKey pressed: %d scancode: %d mods: %d   ",key, scancode, mods);
                fflush(stdout);
                break;
        }
    }
}

void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action) {
        // pressed
    } else {
        // released
        switch (button) {
            case GLFW_MOUSE_BUTTON_1: {
                printf("\rMouse pos: %lf, %lf\n",mouse.x,mouse.y);
                RayResult result = RAY_CASTER::castRayFromCam(30);
                glm::ivec3 pos = result.pos;
                if (result.steps) {
                    deleteBlock(Pos{pos.x,pos.y,pos.z}, 6);
                    printf("\rDeleted block at %d,%d,%d!\n",pos.x,pos.y,pos.z);
                } else {
                    printf("\rNo block to delete in range!\n");
                }
                break;
            }
            
            case GLFW_MOUSE_BUTTON_2: {
                printf("\rMouse pos: %lf, %lf\n",mouse.x,mouse.y);
                glm::ivec3 pos = RAY_CASTER::castRayFromCam(30).lastPos;
                putBlock(Pos{pos.x,pos.y,pos.z},hotbar[currentSelected],6);
                printf("\rPut block at %d,%d,%d!\n",pos.x,pos.y,pos.z);
                break;
            }
            
            default:
                // released
                printf("\rButton pressed: %d mods: %d   \n",button, mods);
                break;
        }
    }
}

void glfwMousePosCallback(GLFWwindow* window, double x, double y) {
    if (mouseLocked) {
        double xDelta = halfWidth - x;
        double yDelta = halfHeight - y;
        rotateCamera(xDelta * -0.1, yDelta * -0.1); // invert x
        glfwSetCursorPos(window,halfWidth,halfHeight);
        mouse.x = halfWidth;
        mouse.y = halfHeight;
    } else {
        mouse.x = x;
        mouse.y = height-y;
    }
}

void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (yoffset > 0 && currentSelected > 0) {
        currentSelected--;
    } else if (yoffset < 0 && currentSelected < hotbarLength - 1) {
        currentSelected++;
    }
    printf("\rCurrently selected block: %d  \n",currentSelected);
}


