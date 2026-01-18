#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <conio.h>
#endif

int get_terminal_width(void) {
#ifdef _WIN32
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    char *cols = getenv("COLUMNS");
    return cols ? atoi(cols) : 80;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (w.ws_col > 0) {
        return w.ws_col;
    }
    char *cols = getenv("COLUMNS");
    return cols ? atoi(cols) : 80;
#endif
}

int get_terminal_height(void) {
#ifdef _WIN32
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
    char *lines = getenv("LINES");
    return lines ? atoi(lines) : 24;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (w.ws_row > 0) {
        return w.ws_row;
    }
    char *lines = getenv("LINES");
    return lines ? atoi(lines) : 24;
#endif
}

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

void read_pwd(char *buf, size_t sz) {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode, new_mode;
    GetConsoleMode(hStdin, &mode);
    new_mode = mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    SetConsoleMode(hStdin, new_mode);

    size_t i = 0;
    while (i < sz - 1) {
        int ch = _getch();
        if (ch == '\r' || ch == '\n') {
            break;
        }
        if (ch == 8 || ch == 127) {
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (ch >= 32 && ch <= 126) {
            buf[i++] = (char)ch;
            putchar('*');
            fflush(stdout);
        }
    }
    buf[i] = 0;
    SetConsoleMode(hStdin, mode);
    putchar('\n');
#else
    struct termios oldt, newt;
    int ch;
    size_t i = 0;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (i < sz - 1 && (ch = getchar()) != EOF && ch != '\n') {
        if ((ch == 127 || ch == 8) && i > 0) {
            --i;
            printf("\b \b");
            fflush(stdout);
        } else {
            buf[i++] = (char)ch;
            putchar('*');
            fflush(stdout);
        }
    }

    buf[i] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    putchar('\n');
#endif
}
