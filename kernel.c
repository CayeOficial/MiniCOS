#include <stdint.h>
#include "commands/commands.h"

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLACK 0x0F
#define MAX_ARGS 16

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;

void clear_screen(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (WHITE_ON_BLACK << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

void put_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (WHITE_ON_BLACK << 8) | ' ';
        }
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (WHITE_ON_BLACK << 8) | c;
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
}

void print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        put_char(str[i]);
    }
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static command_t cmd_table[] = {
    {"help",  cmd_help},
    {"clear", cmd_clear},
    {"info",  cmd_info},
    {"echo",  cmd_echo},
    {"reboot",  cmd_reboot},
    {"poweroff",  cmd_poweroff},
};

static const int num_commands = sizeof(cmd_table) / sizeof(command_t);

int parse_args(char *input, char *argv[]) {
    int argc = 0;
    char *ptr = input;

    while (*ptr != '\0') {
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') {
            *ptr = '\0';
            ptr++;
        }
        if (*ptr == '\0') break;

        if (argc < MAX_ARGS) {
            argv[argc++] = ptr;
        }

        while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r') {
            ptr++;
        }
    }
    argv[argc] = 0;
    return argc;
}

void execute_command(char* input) {
    print("\n");

    char *argv[MAX_ARGS];
    int argc = parse_args(input, argv);

    if (argc == 0) return;

    for (int i = 0; i < num_commands; i++) {
        if (strcmp(argv[0], cmd_table[i].name) == 0) {
            cmd_table[i].func(argc, argv);
            return;
        }
    }

    print("Command not found: ");
    print(argv[0]);
    print("\n");
}

const char kbd_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',   0, ' '
};

char get_char(void) {
    while (1) {
        if (inb(0x64) & 0x01) {
            uint8_t scancode = inb(0x60);
            if (!(scancode & 0x80)) {
                char c = kbd_map[scancode];
                if (c != 0) return c;
            }
        }
    }
}

int confirm(const char *message) {
    print(message);
    print(" [y/N]: ");

    char choice = 0;

    while (1) {
        char c = get_char();

        if (c == '\n') {
            print("\n");
            if (choice == 'N')
                print("Operation cancelled by user.\n");
            return choice == 'Y';
        }
        else if (c == '\b') {
            if (choice != 0) {
                put_char('\b');
                choice = 0;
            }
        }
        else if (c == 'y' || c == 'Y' || c == 'n' || c == 'N') {
            choice = c > 'Z' ? c - 32 : c;
            put_char(c > 'Z' ? c - 32 : c);
        }
    }
}

void kernel_main(void) {
    clear_screen();
    print("COS Modular Kernel\nWrite 'help' for commands.\n\n");

    char buffer[128];
    int buf_index = 0;

    while (1) {
        print("kernel> ");
        buf_index = 0;

        while (1) {
            char c = get_char();

            if (c == '\n') {
                buffer[buf_index] = '\0';
                execute_command(buffer);
                break;
            } else if (c == '\b') {
                if (buf_index > 0) {
                    buf_index--;
                    put_char('\b');
                }
            } else if (buf_index < 127) {
                buffer[buf_index++] = c;
                put_char(c);
            }
        }
    }
}