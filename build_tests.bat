@echo off
REM Build tests for Windows (debug symbols included)
gcc -std=gnu11 -g tests\test_basic.c data.c user.c logic.c store.c -I. -o tests\test_basic.exe -luser32
if %errorlevel% equ 0 (
    echo Build tests succeeded.
    echo Run: tests\test_basic.exe
) else (
    echo Build tests failed.
)
pause
