#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#endif

void init_terminal(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) {
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

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
        return csbi.srWindow.Bottom - csbi.srWindow.Top;
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

int platform_kbhit(void) {
#ifdef _WIN32
    return _kbhit();
#else
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
#endif
}

void platform_sleep(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

void msleep(int ms) {
    platform_sleep(ms);
}
