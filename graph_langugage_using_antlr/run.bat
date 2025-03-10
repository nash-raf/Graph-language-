@echo off
setlocal

rem generate cpp files
echo Generating parser and lexer...
java -jar "C:\sdks\antlr-4.13.2-complete.jar" -Dlanguage=Cpp -visitor -no-listener Base.g4 -o generated

if %errorlevel% neq 0 (
    echo Error generating parser and lexer
    exit /b %errorlevel%
)

rem compile cpp files
echo Compiling parser and lexer...
g++ -std=c++17 -IC:\antlr4-runtime-4.13\antlr4-cpp-runtime-4.13.2-source\runtime\src -Igenerated -I. ^
    -LC:\antlr4-runtime-4.13\antlr4-cpp-runtime-4.13.2-source\build\runtime ^
    main.cpp generated\*.cpp ^
    -lantlr4-runtime -o GraphProgram.exe

if %errorlevel% neq 0 (
    echo Compilation failed!
    exit /b 1
)

rem Run the program
echo Running the program...
if "%~1"=="" (
    echo No input file provided. Please provide a file to parse.
    exit /b 1
)

if not exist "%~1" (
    echo File not found: %~1
    exit /b 1
)

GraphProgram.exe "%~1"