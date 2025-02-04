#include <stdio.h>

#include "input.hpp"
#include "globals.hpp"

void glfwCharCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action) {
        // pressed
    } else {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window,true);
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
    mouse.x = x;
    mouse.y = HEIGHT-y;
    printf("\rmouse pos: %lf,%lf last update: %lf    ",mouse.x,mouse.y,((glfwGetTime()-lastMouseUpdate)));
    fflush(stdout);
    lastMouseUpdate = glfwGetTime();
}


