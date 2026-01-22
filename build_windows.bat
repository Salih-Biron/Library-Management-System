@echo off
REM Windows build script (ASCII-only output to avoid codepage issues)

rem 使用 C11 标准并定义 Windows 控制台相关宏以确保兼容性
gcc main.c data.c logic.c store.c user.c terminal.c -o main.exe -I. -std=gnu11 -D_ENABLE_EXTENDED_ALIGNED_STORAGE -D_WIN32_WINNT=0x0A00 -DENABLE_VIRTUAL_TERMINAL_PROCESSING=0x0004 -luser32

if %errorlevel% equ 0 (
    echo Build succeeded.
    echo Run: main.exe
) else (
    echo Build failed.
)
pause
