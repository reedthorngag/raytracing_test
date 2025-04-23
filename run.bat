@echo off

del bin\output.exe
call build.bat || GOTO :EOF
echo running...
cd bin
output.exe || echo crashed!
cd ..
