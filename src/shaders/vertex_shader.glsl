#version 460 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec3 in_color;

out vec3 color;

void main() {

    // pos is 10 bits for x and y and 12 for z
    color = in_color;
    gl_Position = vec4(pos,0,1);
}