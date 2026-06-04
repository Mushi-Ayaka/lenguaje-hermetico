@echo off
setlocal
set VS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set TB_DIR=%~dp0
set SRC="%TB_DIR%main.cpp" "%TB_DIR%rih_reader.cpp" "%TB_DIR%json_reader.cpp" "%TB_DIR%renderer.cpp" "%TB_DIR%tb_writer.cpp"
set OUT_DIR=%TB_DIR%..\..\build

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

call %VS% 2>nul
if errorlevel 1 (
    echo No se encontro Visual Studio 2022 BuildTools
    exit /b 1
)

cl /EHsc /std:c++17 /O2 /Fo"%OUT_DIR%/" /Fe"%OUT_DIR%/operador_tb.exe" /I "%TB_DIR%.." %SRC% /link

if %ERRORLEVEL% equ 0 (
    echo.
    echo Build OK: %OUT_DIR%operador_tb.exe
) else (
    echo.
    echo Build FAILED
)
