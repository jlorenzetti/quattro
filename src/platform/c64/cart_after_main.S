/**
 * @file cart_after_main.S
 * @brief Cart `__after_main`: stock crt0 would `JMP $B205` (BASIC); here that address is cart ROM.
 */

        .section        .after_main,"ax",@progbits
        .global         __after_main
        .type           __after_main, @function
__after_main:
L:      jmp     L
