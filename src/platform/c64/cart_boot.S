/**
 * @file cart_boot.S
 * @brief KERNAL init after `IOINIT` (RAMTAS, RESTOR, PCINT, `cli`), then `JMP _start`.
 *
 * CBM80 skips the normal `start1` tail; without this, IRQ/jiffy and KERNAL I/O are wrong. Cart link only.
 */

        .section        .cart_boot,"ax",@progbits
        .extern         _start
        .globl          quattro_cart_preflight
        .type           quattro_cart_preflight, @function
quattro_cart_preflight:
        jsr     0xFF84        /* IOINIT */
        jsr     0xFF87        /* RAMTAS */
        jsr     0xFF8A        /* RESTOR */
        jsr     0xFF81        /* PCINT (cint + PAL/NTSC + CIA1 timer latch) */
        cli
        jmp     _start
