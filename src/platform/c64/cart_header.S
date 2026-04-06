/**
 * @file cart_header.S
 * @brief CBM80 signature + cold/warm vectors → `quattro_cart_preflight` at CPU $8000.
 */

        .section        .cart_header,"a",@progbits
        .globl          __quattro_cart_header
        .type           __quattro_cart_header, @object
__quattro_cart_header:
        .word           quattro_cart_preflight   /* cold start */
        .word           quattro_cart_preflight   /* warm start */
        .byte           0xC3, 0xC2, 0xCD, 0x38, 0x30   /* "CBM80" */
