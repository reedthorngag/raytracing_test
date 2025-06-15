@echo off

cls
g++ -Wall -g -W -Werror ^
    -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers ^
    -Wno-unused-but-set-variable ^
    -I C:/lib/glew/include ^
    -I C:/lib/glfw/include ^
    -I ./include ^
    -I C:/include ^
    -L C:\lib\glew\lib\Release\x64 ^
    -L C:\lib\glfw\lib-mingw-w64 ^
    test.cpp ^
    -lopengl32 -lglew32 -lglfw3 -lgdi32 ^
    -o bin/test.exe && bin\test.exe || echo failed!