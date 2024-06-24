@echo off

cls
g++ -std=c++23 -Wall -g -W -Werror ^
    -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers ^
    -Wno-unused-but-set-variable ^
    -I ./include ^
    test.cpp ^
    include/*.cpp ^
    -o bin/test.exe && cd bin && test.exe || echo failed!