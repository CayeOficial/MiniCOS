#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>

void print(const char* str);
void clear_screen(void);
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
int confirm(const char *message);

void cmd_help(int argc, char *argv[]);
void cmd_clear(int argc, char *argv[]);
void cmd_info(int argc, char *argv[]);
void cmd_echo(int argc, char *argv[]);
void cmd_reboot(int argc, char *argv[]);
void cmd_poweroff(int argc, char *argv[]);

typedef struct {
    const char *name;
    void (*func)(int argc, char *argv[]);
} command_t;

#endif