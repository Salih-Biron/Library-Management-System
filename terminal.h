#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#define ENABLE_ANIMATION 0
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#define ENABLE_ANIMATION 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

int get_terminal_width(void);
int get_terminal_height(void);
void clear_screen(void);
void read_pwd(char *buf, size_t sz);
void init_terminal(void);
int platform_kbhit(void);
void platform_sleep(int ms);
void msleep(int ms);

#ifdef __cplusplus
}
#endif

#endif
