#include "commands.h"

void cmd_info(int argc, char *argv[]) {
    (void)argc; (void)argv;
    print("Kernel details:\n");
    print("  Version: 1.0.1\n");
    print("  Author: CayeOficial (cayeoficial.com)\n");
    print("  Last Update Date: 2023-10-01\n");
    print("  Last Update Description: Command History Implementation\n");
}