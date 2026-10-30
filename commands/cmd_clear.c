#include "commands.h"

void cmd_clear(int argc, char *argv[]) {
    (void)argc; (void)argv;
    clear_screen();
}