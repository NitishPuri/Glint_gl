@echo off
echo Building the project...
cmake --build build --config Release --target ALL_BUILD
if %errorlevel% neq 0 exit /b %errorlevel%
echo Running the project...
build\Release\GLWindow.exe %1
pause