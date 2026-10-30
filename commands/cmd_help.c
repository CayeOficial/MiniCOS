#include "commands.h"

void cmd_help(int argc, char *argv[]) {
    (void)argc; (void)argv;
    print("Available commands:\n");
    print("  help   - Show available commands\n");
    print("  clear  - Clear the VGA screen\n");
    print("  info   - Show kernel details\n");
    print("  echo   - Show the passed arguments\n");
}