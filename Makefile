CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I.
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 --no-warn-rwx-segments -T linker.ld

CMD_SRCS = $(wildcard commands/*.c)
CMD_OBJS = $(CMD_SRCS:.c=.o)

TOOLS_SRCS = $(wildcard tools/*.c)
TOOLS_OBJS = $(TOOLS_SRCS:.c=.o)

OBJS = boot.o kernel.o strings.o $(CMD_OBJS) $(TOOLS_OBJS)

TARGET = kernel.bin

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

boot.o: boot.asm
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean