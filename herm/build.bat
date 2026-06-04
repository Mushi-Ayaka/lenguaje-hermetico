@echo off
setlocal
set VS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set HERM_DIR=%~dp0
set OUT_DIR=%HERM_DIR%..\build
set SRC="%HERM_DIR%main.cpp" "%HERM_DIR%herm_lexer.cpp" "%HERM_DIR%herm_parser.cpp" "%HERM_DIR%herm_resolver.cpp" "%HERM_DIR%herm_rih.cpp" "%HERM_DIR%herm_eval.cpp" "%HERM_DIR%herm_render.cpp"

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

call %VS% 2>nul
if errorlevel 1 (
    echo No se encontro Visual Studio 2022 BuildTools
    exit /b 1
)

cl /EHsc /std:c++17 /O2 /Fo"%OUT_DIR%/" /Fe"%OUT_DIR%/herm.exe" /I "%HERM_DIR%.." %SRC% /link

if %ERRORLEVEL% equ 0 (
    echo.
    echo Build OK: %OUT_DIR%herm.exe
) else (
    echo.
    echo Build FAILED
)
