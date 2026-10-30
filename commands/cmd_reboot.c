#include "commands.h"

void cmd_reboot(int argc, char *argv[]) {
    if (!confirm("Are you sure you want to reboot the kernel?", argc, argv)) {
        return;
    }
    print("Rebooting kernel...\n");

    uint8_t temp = 0x02;
    while (temp & 0x02) {
        temp = inb(0x64);
    }

    outb(0x64, 0xFE);
}