@echo off

cls
g++ -std=c++23 -Wall -g -W -Werror ^
    -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers ^
    -Wno-unused-but-set-variable ^
    src/*.cpp ^
    -o bin/output.exe || exit 1

