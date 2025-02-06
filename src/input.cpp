#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "input.hpp"
#include "globals.hpp"

bool keys[512] = { 0 };

double rotationX = 0;
double rotationY = 0;

void rotateCamera(double xDelta, double yDelta) {
    if (rotationX+yDelta <= -90 || rotationX+yDelta >= 90) yDelta = 0;
    rotationX += yDelta; // because its around, not along, that axis
    rotationY += xDelta;
    cameraDir = glm::rotateX(glm::vec3(0,0,1),(float)glm::radians(rotationX));
    cameraDir = glm::rotateY(cameraDir,(float)glm::radians(rotationY));
}

void doInputUpdates(double timeSinceLast) {
    int xDelta = 0;
    int yDelta = 0;

    int xMove = 0;
    int yMove = 0;
    int zMove = 0;

    if (keys[GLFW_KEY_LEFT]) xDelta -= 10;
    if (keys[GLFW_KEY_RIGHT]) xDelta += 10;
    if (keys[GLFW_KEY_DOWN]) yDelta += 10;
    if (keys[GLFW_KEY_UP]) yDelta -= 10;
    if (keys[GLFW_KEY_W]) zMove += 2;
    if (keys[GLFW_KEY_A]) xMove -= 2;
    if (keys[GLFW_KEY_S]) zMove -= 2;
    if (keys[GLFW_KEY_D]) xMove += 2;
    if (keys[GLFW_KEY_LEFT_SHIFT]) yMove -= 2;
    if (keys[GLFW_KEY_SPACE]) yMove += 2;

    if (xDelta || yDelta) rotateCamera(xDelta*timeSinceLast*0.5, yDelta*timeSinceLast*0.5);

    double ms = timeSinceLast / 100000;

    glm::vec3 moveSpeed = glm::vec3(speed) * glm::vec3(ms);

    if (zMove) cameraPos += cameraDir * glm::vec3(zMove) * moveSpeed;

    if (xMove) cameraPos += glm::cross(glm::vec3(0,1,0),cameraDir) * glm::vec3(xMove) * moveSpeed;

    if (yMove) cameraPos += glm::cross(glm::vec3(-1,0,0),cameraDir) * glm::vec3(yMove) * moveSpeed;

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
            case GLFW_MOUSE_BUTTON_1:
                getPixelData = true;
                break;
            
            case GLFW_MOUSE_BUTTON_2:
                mouseLocked = !mouseLocked;
                printf("\rMouse pos: %lf, %lf\n",mouse.x,mouse.y);
                break;
            
            default:
                // released
                printf("\rButton pressed: %d mods: %d   \n",button, mods);
                break;
        }
    }
}

void glfwMousePosCallback(GLFWwindow* window, double x, double y) {
    if (mouseLocked) {
        double xDelta = HALF_WIDTH - x;
        double yDelta = HALF_HEIGHT - y;
        rotateCamera(xDelta,yDelta);
        glfwSetCursorPos(window,HALF_WIDTH,HALF_HEIGHT);
    }
    mouse.x = x;
    mouse.y = HEIGHT-y;
    //printf("\rmouse pos: %lf,%lf last update: %lf    ",mouse.x,mouse.y,((glfwGetTime()-lastMouseUpdate)));
    fflush(stdout);
    lastMouseUpdate = glfwGetTime();
}


