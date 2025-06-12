@echo off

cls
g++ -Wall -g -W -Werror -o2 ^
    -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers ^
    -Wno-unused-but-set-variable ^
    test.cpp ^
    -o bin/test.exe && bin\test.exe || echo failed!