bits 32
section .text
    align 4
    dd 0x1BADB002               ; Multiboot Magic Number
    dd 0x00                     ; Flags
    dd - (0x1BADB002 + 0x00)    ; Checksum (magic + flags + checksum = 0)

global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    call kernel_main

.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
resb 16384                      ; 16KB for stack
stack_top: