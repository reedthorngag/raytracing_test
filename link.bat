@echo off

cls

ld -b pe-i386 libglfw3.a main.o -o bin/test.exe

