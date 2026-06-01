@echo off
REM Build script for Computer Graphics Project - Person 3
REM Requires MinGW with g++ in PATH

echo ================================
echo Computer Graphics Project Build
echo ================================
echo.

echo Compiling main.cpp...
g++ main.cpp -o GraphicsProject.exe -lgdi32 -luser32 -lcomdlg32 -mwindows -O2

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ================================
    echo Build successful!
    echo Executable: GraphicsProject.exe
    echo ================================
    echo.
    echo Run GraphicsProject.exe to start the application.
) else (
    echo.
    echo ================================
    echo Build failed!
    echo Check compiler errors above.
    echo ================================
)

pause
