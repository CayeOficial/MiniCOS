#include "commands.h"

void cmd_poweroff(int argc, char *argv[]) {
    if (!confirm("Are you sure you want to power off the kernel?", argc, argv)) {
        return;
    }
    print("Powering off kernel...\n");

    outw(0x604, 0x2000);   // QEMU ACPI
    outw(0xB004, 0x2000);

    outb(0x64, 0xFE);
}