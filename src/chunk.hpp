#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stdio.h>

const uint32_t w = 100;
const uint32_t h = 100;
const uint32_t l = 100;

class Chunk {
    
public:

    GLuint tex;

    char texData[w*h*l * 3];

    Chunk();

    ~Chunk();
};