@echo off
REM Build extended tests for Windows (debug symbols included)
gcc -std=gnu11 -g tests\test_extended.c data.c user.c logic.c store.c -I. -o tests\test_extended.exe -luser32
if %errorlevel% equ 0 (
    echo Build extended tests succeeded.
    echo Run: tests\test_extended.exe
) else (
    echo Build extended tests failed.
)
pause
