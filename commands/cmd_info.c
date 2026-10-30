#include "commands.h"

void cmd_info(int argc, char *argv[]) {
    (void)argc; (void)argv;
    print("Kernel details:\n");
    print("  Version: 1.0.0\n");
    print("  Author: CayeOficial (cayeoficial.com)\n");
}