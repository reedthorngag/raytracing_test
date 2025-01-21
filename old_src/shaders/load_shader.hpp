#include <GL/glew.h>
#include <fstream>
#include <stdlib.h>

GLuint loadShader(const char* fileName, int type) {

    GLuint shader = glCreateShader(type);

    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        printf("Failed to open %s\n",fileName);
        exit(1);
    }

    int size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buf = new char[size];
    if (!file.read(buf,size)) {
        printf("Failed to read from %s\n",fileName);
        exit(1);
    }

    file.close();

    glShaderSource(shader,1,&buf,&size);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        char* errorLog = new char[maxLength];
        glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog);

        printf("%s: Error: %s",fileName, errorLog);

        delete[] errorLog;
        glDeleteShader(shader);

        exit(1);
    }

    delete[] buf;
    return shader;
}



