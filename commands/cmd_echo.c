#include "commands.h"

void cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        print(argv[i]);
        if (i < argc - 1) print(" ");
    }
    print("\n");
}