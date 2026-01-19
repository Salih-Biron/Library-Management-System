@echo off
REM Windows build script (ASCII-only output to avoid codepage issues)

gcc main.c data.c logic.c store.c user.c terminal.c -o main.exe -I. -luser32

if %errorlevel% equ 0 (
    echo Build succeeded.
    echo Run: main.exe
) else (
    echo Build failed.
)
pause
