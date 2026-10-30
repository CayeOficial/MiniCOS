#include <stdint.h>
#include "commands/commands.h"
#include "tools/command_history.h"
#include "strings.h"

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

    char input_copy[CMD_LEN];
    int i = 0;
    while (input[i] != '\0' && i < CMD_LEN - 1) {
        input_copy[i] = input[i];
        i++;
    }
    input_copy[i] = '\0';

    char *argv[MAX_ARGS];
    int argc = parse_args(input_copy, argv);

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

int get_char(void) {
    static int is_extended = 0;

    while (1) {
        if (inb(0x64) & 0x01) {
            uint8_t scancode = inb(0x60);

            if (scancode == 0xE0) {
                is_extended = 1;
                continue;
            }

            if (!(scancode & 0x80)) {
                if (is_extended) {
                    is_extended = 0;
                    if (scancode == 0x48) return KEY_UP;
                    if (scancode == 0x50) return KEY_DOWN;
                } else {
                    char c = kbd_map[scancode];
                    if (c != 0) return (unsigned char)c;
                }
            } else {
                is_extended = 0;
            }
        }
    }
}

int contains_flag(const char *flag, int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (equals_ignore_case(argv[i], flag)) {
            return 1;
        }
    }

    return 0;
}

int confirm(const char *message, int argc, char *argv[]) {
    if (contains_flag("-y", argc, argv)) {
        return 1;
    } else if (contains_flag("-n", argc, argv)) {
        return 0;
    }

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

    char buffer[CMD_LEN];
    int buf_index = 0;

    while (1) {
        print("kernel> ");
        buf_index = 0;

        while (1) {
            int c = get_char();

            if (c == '\n') {
                buffer[buf_index] = '\0';
                history_add(buffer);
                execute_command(buffer);
                break;
            }
            else if (c == '\b') {
                if (buf_index > 0) {
                    buf_index--;
                    put_char('\b');
                }
            }
            else if (c == KEY_UP) {
                const char* prev_cmd = history_get_prev();
                if (prev_cmd) {
                    while (buf_index > 0) {
                        put_char('\b');
                        buf_index--;
                    }
                    for (int i = 0; prev_cmd[i] != '\0'; i++) {
                        buffer[buf_index++] = prev_cmd[i];
                        put_char(prev_cmd[i]);
                    }
                }
            }
            else if (c == KEY_DOWN) {
                const char* next_cmd = history_get_next();
                if (next_cmd) {
                    while (buf_index > 0) {
                        put_char('\b');
                        buf_index--;
                    }
                    for (int i = 0; next_cmd[i] != '\0'; i++) {
                        buffer[buf_index++] = next_cmd[i];
                        put_char(next_cmd[i]);
                    }
                }
            }
            else if (buf_index < CMD_LEN - 1 && c >= 32 && c <= 126) {
                buffer[buf_index++] = c;
                put_char(c);
            }
        }
    }
}